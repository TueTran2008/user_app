#include "gsm.h"
#include "gsm_http.h"
#include "gsm_utilities.h"
#include "app_debug.h"
#include <stdio.h>
#include <string.h>
#include "gsm.h"
#include "main.h"
#include "ota_update.h"
#include "hardware.h"
#include "flash_if.h"
#include "gsm_filefs.h"

#define OTA_FILE_NAME "UFS:ota.bin"
#define BACKUP_FILE_NAME "UFS:backup.bin"

#define FILE_READ_BUFF_SIZE     512 

static uint32_t m_total_bytes_recv = 0;
static uint32_t m_file_size = 0;
static char m_http_cmd_buffer[FILE_READ_BUFF_SIZE+128];
static int32_t m_file_step = 0;
static int32_t m_file_handle = -1;
const char *m_file_name = OTA_FILE_NAME;

void gsm_file_fs_copy_ota_firmware(gsm_response_event_t event, void *response_buffer)
{
    DEBUG_INFO("Download big file step %u, result %s\r\n",
               m_file_step,
               (event == GSM_EVENT_OK) ? "[OK]" : "[FAIL]");
    if (event != GSM_EVENT_OK)
    {
        DEBUG_ERROR("List file failed\r\n");
        NVIC_SystemReset();
    }

    if (m_file_step == 0)
    {
        // Get file size
        //+QFLST: "UFS:F_M12-1.bmp",562554
        char *filename = strstr((char *)response_buffer, m_file_name);
        if (filename == NULL)
        {
            DEBUG_ERROR("File name error\r\n");
            NVIC_SystemReset();
        }
        filename += strlen(m_file_name) + 2; // Skip ",
        m_file_size = gsm_utilities_get_number_from_string(0, filename);
        DEBUG_INFO("Image %s size %u bytes\r\n", m_file_name, m_file_size);
        ota_update_start(m_file_size);

        sprintf(m_http_cmd_buffer, "AT+QFOPEN=\"%s\",2\r\n", m_file_name);
        gsm_hw_send_at_cmd(m_http_cmd_buffer,
                           "+QFOPEN:",
                           "OK\r\n",
                           2000,
                           1,
                           gsm_file_fs_copy_ota_firmware);
    }
    else if (m_file_step == 1) // Seek file
    {
        gsm_utilities_parse_file_handle((char *)response_buffer, &m_file_handle);
        DEBUG_INFO("File handle %s\r\n", (char *)response_buffer);

        if (m_file_handle != -1)
        {
            // Seek to file position
            sprintf(m_http_cmd_buffer, "AT+QFSEEK=%u,%u,0\r\n", 
                                        m_file_handle, 
                                        m_total_bytes_recv);
            gsm_hw_send_at_cmd(m_http_cmd_buffer,
                               "OK\r\n",
                               "",
                               2000,
                               1,
                               gsm_file_fs_copy_ota_firmware);
        }
        else        // An error
        {
            m_total_bytes_recv = 0;
            m_file_step = 0;
            m_file_handle = -1;
            DEBUG_ERROR("File handle failed %d\r\n", m_file_handle);
            NVIC_SystemReset();
            return;
        }
    }
    else if (m_file_step == 2) // Read file
    {
        sprintf(m_http_cmd_buffer, "AT+QFREAD=%u,%u\r\n", m_file_handle, FILE_READ_BUFF_SIZE);
        gsm_hw_send_at_cmd(m_http_cmd_buffer,
                           "CONNECT ",
                           "OK\r\n",
                           2000,
                           1,
                           gsm_file_fs_copy_ota_firmware);
        gsm_change_hw_polling_interval(2);
    }
    else if (m_file_step == 3)
    {
        uint8_t *content;
        uint32_t size;
        gsm_utilities_get_qfile_content(response_buffer, &content, &size);

        DEBUG_VERBOSE("Data size %u bytes\r\n", size);
        LL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        ota_update_write_next(content, size);

        m_total_bytes_recv += size;
        // Check if all data received
        if (m_total_bytes_recv >= m_file_size)
        {
            DEBUG_VERBOSE("Closing file\r\n");
            sprintf(m_http_cmd_buffer, "AT+QFCLOSE=%u\r\n", m_file_handle);
            gsm_hw_send_at_cmd(m_http_cmd_buffer,
                               "OK\r\n",
                               "",
                               2000,
                               1,
                               gsm_file_fs_copy_ota_firmware);
        }
        else
        {
            m_file_step = 2;
            sprintf(m_http_cmd_buffer, "AT+QFREAD=%u,%u\r\n", 
                                        m_file_handle, 
                                        FILE_READ_BUFF_SIZE);
            gsm_hw_send_at_cmd(m_http_cmd_buffer,
                               "CONNECT",
                               "OK\r\n",
                               2000,
                               1,
                               gsm_file_fs_copy_ota_firmware);
        }
    }
    else if (m_file_step == 4)
    {
        DEBUG_INFO("All data received\r\n");
        if (ota_update_commit_flash())
        {
            ota_flash_cfg_t new_cfg;
            new_cfg.flag = OTA_FLAG_NO_NEW_FIRMWARE; // OTA_FLAG_UPDATE_NEW_FW;
            new_cfg.firmware_size = m_file_size;
            new_cfg.reserve[0] = 0;
            flash_if_write_ota_info_page((uint32_t *)&new_cfg, 
                                        sizeof(ota_flash_cfg_t) / sizeof(uint32_t));
        }
        else
        {
            DEBUG_ERROR("Invalid checksum, rollback firmware\r\n");
            ota_update_rollback_firmware();
        }
        NVIC_SystemReset();
        return;
    }

    m_file_step++;
}


void gsm_file_fs_backup_current_firmware(gsm_response_event_t event, void *response_buffer)
{
    DEBUG_INFO("Backup step %u, result %s, resp %s\r\n",
               m_file_step,
               (event == GSM_EVENT_OK) ? "[OK]" : "[FAIL]",
                (char*)response_buffer);
    if (event != GSM_EVENT_OK)
    {
        DEBUG_ERROR("List file failed\r\n");
        NVIC_SystemReset();
    }

    if (m_file_step == 0)
    {
        uint32_t free = 0;
        bool ret = gsm_utilities_get_storage_file_info(response_buffer, &free, NULL);
        DEBUG_INFO("Free space %u\r\n", free);
        
        // If we dont have enough flash mem =>> Skip backup process, directly update new firmware
        if (!ret || free < APPLICATION_SIZE)
        {
            m_file_name = OTA_FILE_NAME;
            gsm_hw_send_at_cmd("AT+QFLST=\"*\"\r\n",
                               "QFLST",
                               "OK\r\n",
                               8000,
                               2,
                               gsm_file_fs_copy_ota_firmware);
            return;
        }
        
        // Valid space, open mode : create
        sprintf(m_http_cmd_buffer, "AT+QFOPEN=\"%s\",1\r\n", BACKUP_FILE_NAME);
        gsm_hw_send_at_cmd(m_http_cmd_buffer,
                           "+QFOPEN:",
                           "OK\r\n",
                           2000,
                           1,
                           gsm_file_fs_backup_current_firmware);
    }
    else if (m_file_step == 1) // Seek file
    {
        gsm_utilities_parse_file_handle((char *)response_buffer, &m_file_handle);
        DEBUG_INFO("File handle %s\r\n", (char *)response_buffer);

        if (m_file_handle != -1)
        {
            DEBUG_INFO("Seek file\r\n");
            sprintf(m_http_cmd_buffer, "AT+QFSEEK=%u,%u,0\r\n", m_file_handle, 0);
            gsm_hw_send_at_cmd(m_http_cmd_buffer,
                               "OK\r\n",
                               "",
                               2000,
                               1,
                               gsm_file_fs_backup_current_firmware);
        }
        else
        {
            DEBUG_ERROR("File handle failed %d\r\n", m_file_handle);
            m_total_bytes_recv = 0;
            m_file_step = 0;
            m_file_handle = -1;
            NVIC_SystemReset();
            return;
        }
    }
    else if (m_file_step == 2) // Write file
    {
        DEBUG_INFO("Write file\r\n");
        sprintf(m_http_cmd_buffer, "AT+QFWRITE=%u,%u,%u\r\n", 
                                    m_file_handle, 
                                    FILE_READ_BUFF_SIZE, 
                                    200000);
        gsm_hw_send_at_cmd(m_http_cmd_buffer,
                           "CONNECT",
                           "",
                           3000,
                           1,
                           gsm_file_fs_backup_current_firmware);
    }
    else if (m_file_step == 3)
    {
        uint8_t *data = (uint8_t*)APPLICATION_START_ADDR;
        uint32_t number_of_block = APPLICATION_SIZE/1024;
        
        // Backup current firmware to flash
        for (uint32_t i = 0; i < number_of_block; i++)
        {
            gsm_hw_uart_send_raw(data, 1024);
            data += 1024;
            LL_IWDG_ReloadCounter(IWDG);
            LL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            DEBUG_INFO("Sending %u block\r\n", i);
            sys_delay_ms(25);
        }

        DEBUG_INFO("Closing file\r\n");
        sprintf(m_http_cmd_buffer, "AT+QFCLOSE=%u\r\n", m_file_handle);
        gsm_hw_send_at_cmd(m_http_cmd_buffer,
                               "OK\r\n",
                               "",
                               2000,
                               1,
                               gsm_file_fs_backup_current_firmware);
    }
    else if (m_file_step == 4)
    {
        // Backup complete, copy new firmware
        DEBUG_INFO("Backup firmware done\r\n");
        m_file_name = OTA_FILE_NAME;
        m_file_step = 0;
        gsm_hw_send_at_cmd("AT+QFLST=\"*\"\r\n",
                               "QFLST",
                               "OK\r\n",
                               8000,
                               2,
                               gsm_file_fs_copy_ota_firmware);
        return;
    }

    m_file_step++;
}

void gsm_file_fs_start(gsm_file_fs_type_t type)
{
//    gsm_hw_send_at_cmd("AT+QFLST=\"*\"\r\n",
//                       "QFLST",
//                       "OK\r\n",
//                       8000,
//                       2,
//                       gsm_file_fs_backup_current_firmware);
    
    if (type == GSM_FILE_FS_ROLLBACK_FIRMWARE)
    {
        DEBUG_INFO("Rollback firmware\r\n");
        m_file_name = BACKUP_FILE_NAME;
        gsm_hw_send_at_cmd("AT+QFLDS=\"UFS\"\r\n",
                           "+QFLDS:",
                           "OK\r\n",
                           5000,
                           2,
                           gsm_file_fs_copy_ota_firmware);
    }
    else
    {
        gsm_hw_send_at_cmd("AT+QFLDS=\"UFS\"\r\n",
                           "+QFLDS:",
                           "OK\r\n",
                           5000,
                           2,
                           gsm_file_fs_backup_current_firmware);
    }
}
