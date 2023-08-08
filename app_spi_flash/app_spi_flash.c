#include "app_spi_flash.h"
#include "app_debug.h"
#include <string.h>
#include "spi.h"

#define DEBUG_FLASH 0
#define SPI_DUMMY 0x00

#define CMD_READ_DATA                               0x03
#define CMD_FAST_READ                               0x0B
#define CMD_READ_INDENTIFICATION                    0x9F
#define CMD_READ_DEVICE_AND_MANUFACTURE_ID          0x90
#define CMD_WRITE_ENABLE                            0x06
#define CMD_WRITE_DISABLE                           0x04

#define CMD_ERASE_SECTOR                            0x20    // Sector Erase - 4KB
#define CMD_ERASE_BLOCK                             0xD8    // Block Erase - 64KB
#define CMD_ERASE_ALL                               0xC7    // 0x60 Chip Erase
#define CMD_WRITE_STATUS_REGISTER                   0x01    // Write status register
#define CMD_PAGE_PROGRAM                            0x02    // Write data to page
#define CMD_READ_STATUS_REGISTER                    0x05    // Read status register
#define CMD_DEEP_POWER_DOWN                         0xB9    // Enter deep power down mode       
#define CMD_WAKEUP_FROM_DEEP_POWER_DOWN             0xAB    // Exit power down mode

// #define CLSR_CMD 0x30 /* Dua Flash ve trang thai mac dinh */

///* Dinh nghia cac lenh cho dia chi do rong 4-byte */
//#define READ_DATA_CMD4 0x13      // Read
//#define FAST_READ_DATA_CMD4 0x0C // FastRead
//#define PP_CMD4 0x12             // Page Program
//#define SE_CMD4 0x21             // Sector Erase - 4KB - 32bit addrress
//#define BE_CMD4 0xDC             // Block Erase - 64KB - 32bits address

///* Mot so lenh danh rieng cho Flash GigaDevice */
//#define EN4B_MODE_CMD 0xB7 // Vao che do 32bits dia chi
//#define EX4B_MODE_CMD 0xE9 // Thoat che do 32bits dia chi
//#define RDSR1_CMD 0x05     // Read status register 1
//#define RDSR2_CMD 0x35     // Read status register 2
//#define RDSR3_CMD 0x15     // Read status register 3

//#define WRSR1_CMD 0x01 // Write status register 1
//#define WRSR2_CMD 0x31 // Write status register 2
//#define WRSR3_CMD 0x11 // Write status register 3

/* Winbond */
//#define WB_RESET_STEP0_CMD 0x66
//#define WB_RESET_STEP1_CMD 0x99



#define FLASH_WRITE_TIMEOUT_MS 5
#define FLASH_ERASE_TIMEOUT_MS 120000

#define SECTOR_ERASE_MIN_MS 100
#define SECTOR_ERASE_TIME_MAX_MS 300

#define GET_DEVICE_ID_RETRIES 2

#define MEGABYTE(x)         (x<<20)     // x *= (1024*1024)
#define KILOBYTE(x)         (x<<10)     // x *= 1024

static const app_flash_info_t m_info_table[] =
{
    {APP_SPI_FLASH_DEVICE_INVALID, 0, APP_SPI_DEVICE_ERROR}, // bytes
    { 
        APP_SPI_FLASH_FL164K, MEGABYTE(2), APP_SPI_FRAM
    },
    {
        APP_SPI_FLASH_FL127S, MEGABYTE(16), APP_SPI_FRAM,
    },
    {
        APP_SPI_FLASH_FL256S, MEGABYTE(32), APP_SPI_FRAM,
    },
    {
        APP_SPI_FLASH_GD256, MEGABYTE(32), APP_SPI_FLASH,
    },
    {
        APP_SPI_FLASH_W25Q256JV, MEGABYTE(32), APP_SPI_FLASH,
    },
    {
        APP_SPI_FLASH_W25Q80D, MEGABYTE(2), APP_SPI_FLASH,
    },
    {
        APP_SPI_FLASH_W25Q128, MEGABYTE(16), APP_SPI_FLASH,
    },
    {
        APP_SPI_FLASH_W25Q32, MEGABYTE(4), APP_SPI_FLASH,
    },
    {
        APP_SPI_FLASH_W25Q64, MEGABYTE(8), APP_SPI_FLASH,
    },
    {
        APP_SPI_FRAM_MB85RS16, KILOBYTE(8), APP_SPI_FRAM,
    },
    {
        APP_SPI_FLASH_AT25SF128, MEGABYTE(16), APP_SPI_FLASH,
    },
    {
        APP_SPI_FRAM_FM25V02A, KILOBYTE(32), APP_SPI_FRAM,
    },
    //{  APP_SPI_FRAM_FM25V02A, 	        16*1024,  	    APP_SPI_FRAM,			},
    {
        APP_SPI_FRAM_MB85RS64V, KILOBYTE(8), APP_SPI_FRAM,
    },
    {
        APP_SPI_FRAM_FM25V01, KILOBYTE(16), APP_SPI_FRAM,
    },
    {
        APP_SPI_FRAM_FM25V10, KILOBYTE(16), APP_SPI_FRAM,
    },
    {
        APP_SPI_FRAM_MB85RS128TYPNF, KILOBYTE(16), APP_SPI_FRAM,
    },
    
    {
        APP_SPI_FLASH_IS25LP032D, MEGABYTE(4), APP_SPI_FLASH,
    },
    
    { 
        APP_SPI_FLASH_GD25Q64, MEGABYTE(8), APP_SPI_FLASH
    },
                
    {APP_SPI_FLASH_MAX, 0, APP_SPI_DEVICE_ERROR},
};

enum
{
    NUM_CURRENT_LOOK_UP = sizeof(m_info_table) / sizeof(app_flash_info_t)
};

static uint8_t flash_get_device_id(app_flash_drv_t *driver);

// static uint32_t m_max_ringbuffer_log_size;
// static uint8_t m_sector_data[SPI_FLASH_SECTOR_SIZE];

static app_flash_info_t *lookup_flash(app_flash_device_t id)
{
    if (id > APP_SPI_FLASH_MAX)
    {
        return NULL;
    }

    return (app_flash_info_t *)&m_info_table[id];
}


void app_spi_flash_initialize(app_flash_drv_t *driver)
{
    // Wakeup flash
    app_spi_flash_wakeup(driver);
    
#if 0
    app_spi_flash_status_register_t status = app_spi_flash_read_status(driver);
    if (status.name.QE)
    {
        DEBUG_ERROR("QSPI mode\r\n");
    }
    else
    {
        DEBUG_WARN("SPI mode\r\n");
    }
#endif
    
    // Get device info
    if (flash_get_device_id(driver))
    {
        app_flash_info_t *info = lookup_flash(driver->info.device);
        if (info)
        {
            memcpy(&driver->info, info, sizeof(app_flash_info_t));
        }
        else
        {
            driver->info.type = APP_SPI_DEVICE_ERROR;
        }
        DEBUG_RAW("Flash type: %u, name ", driver->info.type);
        switch (driver->info.device)
        {
        case APP_SPI_FLASH_FL164K: // 8MB
            DEBUG_RAW("FL164K, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_FL127S: // 16MB
            DEBUG_RAW("FL127S, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_FL256S: // 32MB
            DEBUG_RAW("FL256S, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_GD256: // 32MB
            DEBUG_RAW("GD256, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_W25Q256JV: // 32MB
            DEBUG_RAW("256JV, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_W25Q80D:
            DEBUG_RAW("W25Q80DL, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_W25Q128:
            DEBUG_RAW("W25Q128, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_W25Q32:
            DEBUG_RAW("W25Q32FV, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_W25Q64:
            DEBUG_RAW("W25Q64, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FRAM_MB85RS16:
            DEBUG_RAW("MB85RS16, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_AT25SF128:
            DEBUG_RAW("AT25SF128, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FRAM_FM25V02A:
            DEBUG_RAW("FM25V02A, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FRAM_MB85RS64V:
            DEBUG_RAW("MB85RS64V, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FRAM_FM25V01:
            DEBUG_RAW("FM25V01, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FRAM_FM25V10:
            DEBUG_RAW("FM25V10, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FRAM_MB85RS128TYPNF:
            DEBUG_RAW("MB85RS128TYPNF, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_IS25LP032D:
            DEBUG_RAW("IS25LP032D, size %u bytes", driver->info.size);
            break;
        case APP_SPI_FLASH_GD25Q64:
            DEBUG_RAW("GD25Q64, size %u bytes", driver->info.size);
            break;
        default:
            DEBUG_RAW("UNKNOWNN: %u", driver->info.device);
            driver->error = true;
            break;
        }

        if (!driver->error)
        {
            if (driver->info.type == APP_SPI_FLASH)
            {
                driver->info.sector_size = 4096;
                driver->info.page_size = 256;
            }

            // TODO : handle FRAM
            // sector 0 : use for header =>> size =  sizeof_flash - sector0_size
            driver->ringbuffer_log_size = driver->info.size - driver->info.sector_size;
        }
        else
        {
            driver->ringbuffer_log_size = 0;
        }
        DEBUG_RAW("\r\n");
    }
    else
    {
        DEBUG_WARN("Flash failed\r\n");
        driver->error = true;
        driver->info.size = 0;
        return;
    }
}

bool app_spi_flash_is_ok(app_flash_drv_t *driver)
{
    return driver->error ? false : true;
}

static void flash_write_control(app_flash_drv_t *driver, uint8_t enable)
{
    driver->callback.spi_cs(driver->spi, 0);
    if (enable)
    {
        spi_flash_transmit(driver, CMD_WRITE_ENABLE);
    }
    else
    {
        spi_flash_transmit(driver, CMD_WRITE_DISABLE);
    }
    driver->callback.spi_cs(driver->spi, 1);
}

bool app_spi_flash_wait_action_complete(app_flash_drv_t *driver, uint32_t timeout_ms)
{
    bool result = false;
    app_spi_flash_status_register_t status;
    while (1)
    {
        status = app_spi_flash_read_status(driver);
        
        if (status.name.WIP == 0)
        {
            DEBUG_VERBOSE("Wait WIP : status register 0x%02X\r\n", 
                          status.val);
            result = true;
            break;
        }
        if (timeout_ms)
        {
            timeout_ms--;
            driver->callback.delay_ms(driver, 1);
        }
        else
        {
            break;
        }
    }

    if (timeout_ms == 0)
    {
//        DEBUG_ERROR("[%s-%d] error\r\n", __FUNCTION__, __LINE__);
    }
    return result;
}

bool app_spi_flash_wait_erase_complete(app_flash_drv_t *driver, uint32_t timeout_ms)
{
    bool result = false;
    app_spi_flash_status_register_t status;
    while (1)
    {
        status = app_spi_flash_read_status(driver);
        
        if (status.name.WIP == 0)
        {
            result = true;
            DEBUG_VERBOSE("Erase flash : status register 0x%08X\r\n", status.val);
            break;
        }
        if (timeout_ms)
        {
            timeout_ms--;
            driver->callback.delay_ms(driver, 1);
        }
        else
        {
            break;
        }
    }

    if (timeout_ms == 0)
    {
//        DEBUG_ERROR("[%s-%d] error\r\n", __FUNCTION__, __LINE__);
    }
    return result;
}

bool app_spi_flash_direct_write_page(app_flash_drv_t *driver, uint32_t addr, uint8_t *buffer, uint16_t length)
{
    if (driver->error)
    {
        return false;
    }
    DEBUG_VERBOSE("Flash write page addr 0x%08X, size %u\r\n", addr, length);

    flash_write_control(driver, 1);

    uint8_t tmp[32];
    uint32_t buffer_size = 0;

//    // uint8_t cmd;
//    if (driver->info.device == APP_SPI_FLASH_FL256S || driver->info.device == APP_SPI_FLASH_GD256 || driver->info.device == APP_SPI_FLASH_W25Q256JV)
//    {
//        /* Send write cmd */
//        tmp[buffer_size++] = PP_CMD4;

//        /* Send 4 byte addr */
//        tmp[buffer_size++] = (addr >> 24) & 0xFF;
//    }
//    else
    {
        /* Send write cmd */
        tmp[buffer_size++] = CMD_PAGE_PROGRAM;
    }

    if (driver->info.type == APP_SPI_FLASH)
    {
        /* Send 3 bytes address */
        tmp[buffer_size++] = (addr >> 16) & 0xFF;
        tmp[buffer_size++] = (addr >> 8) & 0xFF;
        tmp[buffer_size++] = addr & 0xFF;
    }
    else // FRAM : only 2 byte address send
    {
        /* Send 2 bytes address */
        tmp[buffer_size++] = (addr & 0xFF00) >> 8;
        tmp[buffer_size++] = addr & 0xFF;
    }

    driver->callback.spi_cs(driver->spi, 0);
    // Send write command
    driver->callback.spi_tx_buffer(driver->spi, tmp, buffer_size);
    
    /* Send data to flash */
    driver->callback.spi_tx_buffer(driver->spi, buffer, length);

    driver->callback.spi_cs(driver->spi, 1);

    if (!app_spi_flash_wait_action_complete(driver, FLASH_WRITE_TIMEOUT_MS))
    {
        return false;
    }

#if APP_SPI_FLASH_VERIFY_AFTER_WRITE
    uint32_t i = 0;
    uint32_t old_addr = addr;
    bool found_error = false;
    
    uint32_t tmp_len = length >> 2;     // Div by 4
    tmp_len <<= 2;      // Mul byte 4
    for (i = 0; i < tmp_len; i += 4) // Debug only
    {
        uint32_t rb;
        app_spi_flash_read_bytes(driver, old_addr + i, (uint8_t *)&rb, 4);
        if (memcmp(&rb, buffer, 4))
        {
            found_error = true;
            DEBUG_ERROR("Flash write error at addr 0x%08X, readback 0x%08X, expect 0x%08X, len %u\r\n", 
                        old_addr + i, rb, *((uint32_t *)(buffer)), 
                        tmp_len);
            return false;
            //            break;
        }
        buffer += 4;
    }
    
    length = length-tmp_len;
    if (length)
    {
        uint8_t rb[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        
        app_spi_flash_read_bytes(driver, tmp_len, rb, length);
        
        if (memcmp(rb, buffer, length))
        {
            found_error = true;
            DEBUG_ERROR("Flash write error at addr 0x%08X\r\n");
            return false;
            //            break;
        }
    }
    
    if (found_error == false)
    {
        DEBUG_VERBOSE("Page write success\r\n");
    }
#endif
    return true;
    // vPortFree(tmp);
}

bool app_spi_flash_write(app_flash_drv_t *driver, uint32_t addr, uint8_t *buffer, uint32_t length)
{
    if (driver->error)
    {
        return false;
    }
    if (length == 0)
    {
        return true;
    }
    /* Split data into page size (256), only availble in Flash */
    uint32_t offset_addr = 0;
    uint32_t length_need_to_write = 0;
    uint32_t nb_bytes_written = 0;

    DEBUG_VERBOSE("Flash write %u bytes, from addr 0x%08X\r\n", length, addr);
    if (addr + length > driver->info.size)
    {
        DEBUG_ERROR("Flash write from 0x%08X to 0x%08X is over flash size %u bytes\r\n",
                    addr,
                    addr + length, driver->info.size);
        return false;
    }
    uint32_t max_write_size = driver->info.page_size;
    if (driver->info.type == APP_SPI_FRAM)
    {
        max_write_size = length + 1;
    }
    while (length)
    {
        offset_addr = addr % max_write_size;

        if (offset_addr > 0)
        {
            if (offset_addr + length > max_write_size)
            {
                length_need_to_write = max_write_size - offset_addr;
            }
            else
            {
                length_need_to_write = length;
            }
        }
        else
        {
            if (length > max_write_size)
            {
                length_need_to_write = max_write_size;
            }
            else
            {
                length_need_to_write = length;
            }
        }

        length -= length_need_to_write;

        if (!app_spi_flash_direct_write_page(driver,
                                             addr,
                                             &buffer[nb_bytes_written],
                                             length_need_to_write))
        {
            return false;
        }
        else
        {
            nb_bytes_written += length_need_to_write;
            addr += length_need_to_write;
        }
    }
    return true;
}

void app_spi_flash_read_bytes(app_flash_drv_t *driver, uint32_t addr, uint8_t *buffer, uint16_t length)
{
    if (driver->error)
    {
        return;
    }

    uint8_t cmd_buffer[6];
    uint8_t index = 0;
    uint32_t next_sector_offset = 0;
//    if (addr + length > driver->info.size)
//    {
//        next_sector_offset = (addr + length) - driver->info.size;
//        length = driver->info.size - addr;
//    }
    while (1)
    {
        /* Send read cmd*/
        cmd_buffer[index++] = CMD_READ_DATA;

        if (driver->info.type == APP_SPI_FLASH)
        {
            /* Send 3 bytes address */
            cmd_buffer[index++] = (addr >> 16) & 0xFF;
            cmd_buffer[index++] = (addr >> 8) & 0xFF;
            cmd_buffer[index++] = addr & 0xFF;
        }
        else    // FRAM 2 byte address
        {
            /* Send 2 bytes address */
            cmd_buffer[index++] = (addr & 0xFF00) >> 8;
            cmd_buffer[index++] = addr & 0xFF;
        }

        driver->callback.spi_cs(driver->spi, 0);
        driver->callback.spi_tx_buffer(driver->spi, cmd_buffer, index);
        driver->callback.spi_rx_buffer(driver->spi, buffer, length);
        driver->callback.spi_cs(driver->spi, 1);

        if (next_sector_offset == 0)
        {
            break;
        }
        else
        {
            buffer += length;
            addr = next_sector_offset;
            next_sector_offset = 0;
            continue;
        }
    }
}

bool app_spi_flash_erase_sector_4k(app_flash_drv_t *driver, uint32_t sector_count)
{
    bool retval = false;
    uint32_t addr = 0;    
    uint8_t cmd_buffer[32];
    uint32_t cmd_count = 0;
    
    if (driver->info.type == APP_SPI_FRAM)
    {
        retval = true;
        goto end;
    }
    if (driver->error)
    {
        goto end;
    }


    addr = sector_count * driver->info.sector_size; // Sector 4KB
    
    DEBUG_VERBOSE("Erase sector %u, addr 0x%08X\r\n", sector_count, addr);
    
    flash_write_control(driver, 1);


//    if (driver->info.device == APP_SPI_FLASH_FL256S || driver->info.device == APP_SPI_FLASH_GD256 || driver->info.device == APP_SPI_FLASH_W25Q256JV)
//    {
//        /* Gui lenh */
//        cmd_buffer[cmd_count++] = SE_CMD4;

//        /* Send 4 bytes address */
//        cmd_buffer[cmd_count++] = (addr >> 24) & 0xFF;
//    }
//    else
    {
        /* Gui lenh */
        cmd_buffer[cmd_count++] = CMD_ERASE_SECTOR;
    }

    /* Send 3 byte address */
    cmd_buffer[cmd_count++] = (addr >> 16) & 0xFF;
    cmd_buffer[cmd_count++] = (addr >> 8) & 0xFF;
    cmd_buffer[cmd_count++] = addr & 0xFF;

    driver->callback.spi_cs(driver->spi, 0);
    driver->callback.spi_tx_buffer(driver->spi, cmd_buffer, cmd_count);
    driver->callback.spi_cs(driver->spi, 1);

    driver->callback.delay_ms(driver, SECTOR_ERASE_MIN_MS/2);
    
    if (app_spi_flash_wait_action_complete(driver, SECTOR_ERASE_TIME_MAX_MS/2))
    {
        DEBUG_VERBOSE("Success\r\n", sector_count);
        retval = true;
    }
    else
    {
        DEBUG_ERROR("Failed\r\n", sector_count);
        retval = false;
        goto end;
    }
    
#if APP_SPI_FLASH_VERIFY_AFTER_WRITE
    if (app_spi_flash_is_sector_empty(driver, sector_count))
    {
        retval = true;
    }
    else
    {
        DEBUG_ERROR("Erase sector %u failed\r\n", sector_count);
        retval = false;
    }
#endif
    
end:
    return retval;
}

// void flash_erase_block_64K(uint16_t sector_count)
//{
//     uint32_t addr = 0;
//     uint32_t old_addr = 0;
//     uint8_t cmd;

//    addr = sector_count * 65536; //Sector 64KB
//    old_addr = addr;

//    flash_write_control(1, addr);

//    driver->callback.spi_cs(driver->spi, 0);

//    if (m_flash_version == FL256S || m_flash_version == GD256C || m_flash_version == W25Q256JV)
//    {
//        cmd = BE_CMD4;
//        spi_flash_transmit(cmd);

//        /* Send 4 bytes address */
//        cmd = (addr >> 24) & 0xFF;
//        spi_flash_transmit(cmd);
//    }
//    else
//    {
//        cmd = CMD_ERASE_BLOCK;
//        spi_flash_transmit(cmd);
//    }

//    /* Send 3 bytes address */
//    cmd = (addr >> 16) & 0xFF;
//    spi_flash_transmit(cmd);

//    cmd = (addr >> 8) & 0xFF;
//    spi_flash_transmit(cmd);

//    cmd = addr & 0xFF;
//    spi_flash_transmit(cmd);

//    driver->callback.spi_cs(driver->spi, 1);
//    app_spi_flash_wait_action_complete(old_addr);
//}

static uint8_t flash_get_device_id(app_flash_drv_t *driver)
{
//    uint8_t reg_status = 0;
    uint8_t tries;
    uint8_t cmd;
    app_spi_flash_device_id_t id;
    bool val = false;

    driver->error = false;
    driver->info.device = APP_SPI_FLASH_DEVICE_INVALID;

    for (tries = 0; tries < GET_DEVICE_ID_RETRIES; tries++)
    {
#if 0
        cmd = CMD_READ_DEVICE_AND_MANUFACTURE_ID;
#else
        cmd = CMD_READ_INDENTIFICATION;
#endif
//        if (cmd == CMD_READ_DEVICE_AND_MANUFACTURE_ID)
//        {
//            uint8_t buffer_tx[6] = {CMD_READ_DEVICE_AND_MANUFACTURE_ID, CMD_READ_INDENTIFICATION, 0x00, 0x00, 0xFF, 0x00};
//            uint8_t buffer_rx[6];

//#if 0
//                driver->callback.spi_cs(driver->spi, 0);
//			driver->callback.spi_tx_byte(driver->spi, cmd);

//			/* 3 byte address */
//			cmd = 0;
//			driver->callback.spi_tx_byte(driver->spi, 0x00);
//			driver->callback.spi_tx_byte(driver->spi, 0x00);
//			driver->callback.spi_tx_byte(driver->spi, 0x00);

//			cmd = 0xFF;
//			driver->callback.spi_tx_rx(driver->spi, &cmd, &id.name.manufacture_id[1], 1);
//			driver->callback.spi_tx_rx(driver->spi, &cmd, &id.name.device_id[0], 1);
//#else
//            driver->callback.spi_tx_rx(driver->spi, buffer_tx, buffer_rx, 6);
//            driver->callback.spi_cs(driver->spi, 1);
//            id.name.manufacture_id[1] = buffer_rx[5];
//            id.name.device_id[0] = buffer_rx[4];

//#endif
//            DEBUG_INFO("device id: 0x%02X, manufacture id: 0x%02X\r\n", 
//                       id.name.device_id[0],
//                       id.name.manufacture_id[1]);
//        }
//        else
        {
            uint8_t buffer_size = 1 + sizeof(app_spi_flash_device_id_t);
            uint8_t buffer_tx[buffer_size];
            uint8_t buffer_rx[buffer_size];
            buffer_tx[0] = cmd;
            
            driver->callback.spi_cs(driver->spi, 0);
            driver->callback.spi_tx_rx(driver->spi, buffer_tx, buffer_rx, buffer_size);
            driver->callback.spi_cs(driver->spi, 1);
            
            memcpy(&id, &buffer_rx[1], sizeof(app_spi_flash_device_id_t));


            DEBUG_INFO("Manufacture: 0x%02X, type: 0x%02X, capacity 0x%02X\r\n", 
                       id.name.manu_id,
                       id.name.mem_type,
                       id.name.capacity);
        }

//        if (cmd == CMD_READ_DEVICE_AND_MANUFACTURE_ID)
//        {
//            if (id.name.manufacture_id[1] == 0x01)
//            {
//                DEBUG_INFO("FRAM\r\n");
//                if (id.name.device_id[0] == 0x16)
//                {
//                    driver->info.device = APP_SPI_FLASH_FL164K;
//                    val = true;
//                }
//                else if (id.name.device_id[0] == 0x17)
//                {
//                    driver->info.device = APP_SPI_FLASH_FL127S;
//                    val = true;
//                }
//            }
//            else if (id.name.manufacture_id[1] == 0xEF)
//            {
//                DEBUG_INFO("Windbond\r\n");
//                if (id.name.device_id[0] == 0x13)
//                {
//                    DEBUG_INFO("W25Q80DL\r\n");
//                    driver->info.device = APP_SPI_FLASH_W25Q80D;
//                    val = true;
//                }
//                if (id.name.device_id[0] == 0x15)
//                {
//                    DEBUG_INFO("W25Q32FV\r\n");
//                    driver->info.device = APP_SPI_FLASH_W25Q32;
//                    val = true;
//                }
//                else if (id.name.device_id[0] == 0x17)
//                {
//                    DEBUG_INFO("W25Q128\r\n");
//                    driver->info.device = APP_SPI_FLASH_W25Q128;
//                    val = true;
//                }
//                if (id.name.device_id[0] == 0x18)
//                {
//                    DEBUG_INFO("256JV\r\n");
//                    driver->info.device = APP_SPI_FLASH_W25Q256JV;
//                    // Enter mode : 4 bytes address
//                    driver->callback.spi_cs(driver->spi, 0);
//                    cmd = EN4B_MODE_CMD;
//                    driver->callback.spi_tx_byte(driver->spi, cmd);
//                    driver->callback.spi_cs(driver->spi, 1);

//                    driver->callback.delay_ms(driver, 10);
//                    // Read status register 3, bit ADS  (S16) - bit 0
//                    driver->callback.spi_cs(driver->spi, 0);
//                    cmd = RDSR3_CMD;
//                    driver->callback.spi_tx_byte(driver->spi, cmd);

//                    cmd = SPI_DUMMY;
//                    driver->callback.spi_tx_rx(driver->spi, &cmd, &reg_status, 1);

//                    DEBUG_INFO("status register: %02X\r\n", reg_status);
//                    if (reg_status & 0x01)
//                    {
//                        DEBUG_INFO("Address mode : 32 bit\r\n");
//                    }
//                    driver->callback.spi_cs(driver->spi, 1);
//                    val = true;
//                }
//            }
//            else if (id.name.manufacture_id[1] == 0xC8) /* APP_SPI_FLASH_GD256 - GigaDevice 256Mb */
//            {
//                DEBUG_INFO("Giga device\r\n");
//                if (id.name.device_id[0] == 0x18)
//                {
//                    driver->info.device = APP_SPI_FLASH_GD256;
//                    DEBUG_INFO("GD256\r\n");

//                    // Enter mode : 4 bytes address
//                    driver->callback.spi_cs(driver->spi, 0);
//                    cmd = EN4B_MODE_CMD;
//                    driver->callback.spi_tx_byte(driver->spi, cmd);
//                    driver->callback.spi_cs(driver->spi, 1);

//                    driver->callback.delay_ms(driver, 10);
//                    // Read register status 2, bit ADS - 5
//                    driver->callback.spi_cs(driver->spi, 0);
//                    cmd = RDSR2_CMD;
//                    driver->callback.spi_tx_byte(driver->spi, cmd);

//                    cmd = SPI_DUMMY;
//                    driver->callback.spi_tx_rx(driver->spi, &cmd, &reg_status, 1);
//                    driver->callback.spi_cs(driver->spi, 1);

//                    DEBUG_INFO("status register: %02X\r\n", reg_status);
//                    if (reg_status & 0x20)
//                    {
//                        DEBUG_INFO("Address mode : 32 bit\r\n");
//                    }
//                    else
//                    {
//                        driver->callback.delay_ms(driver, 100);
//                        continue;
//                    }
//                    val = true;
//                }
//            }
//            else if (id.name.manufacture_id[1] == 0x89) /* APP_SPI_FLASH_AT25SF128 */
//            {
//                DEBUG_INFO("Adesto Technologies\r\n");
//                if (id.name.device_id[0] == 0x1F)
//                {
//                    driver->info.device = APP_SPI_FLASH_AT25SF128;
//                    DEBUG_INFO("AT25SF128A\r\n");

//                    // Enter mode : 4 bytes address
//                    driver->callback.spi_cs(driver->spi, 0);
//                    cmd = EN4B_MODE_CMD;
//                    driver->callback.spi_tx_byte(driver->spi, cmd);
//                    driver->callback.spi_cs(driver->spi, 1);

//                    driver->callback.delay_ms(driver, 10);
//                    // Read register status 2, bit ADS - 5
//                    driver->callback.spi_cs(driver->spi, 0);
//                    cmd = RDSR2_CMD;
//                    driver->callback.spi_tx_byte(driver->spi, cmd);

//                    cmd = SPI_DUMMY;
//                    driver->callback.spi_tx_rx(driver->spi, &cmd, &reg_status, 1);
//                    driver->callback.spi_cs(driver->spi, 1);

//                    DEBUG_INFO("status register: %02X\r\n", reg_status);
//                    if (reg_status & 0x20)
//                    {
//                        DEBUG_INFO("Address mode : 32 bit\r\n");
//                    }
//                    else
//                    {
//                        driver->callback.delay_ms(driver, 500);
//                        continue;
//                    }
//                    val = true;
//                }
//            }
//            else if (id.name.manufacture_id[0] == 0x7F && id.name.manufacture_id[1] == 0x7F) /* APP_SPI_FLASH_AT25SF128 */
//            {
//                DEBUG_INFO("Cypress\r\n");
//                // Cypress read total 9 byte,
//                // 6 byte MSB = 7F7F7F7F7F7F
//                // Byte 7 done care
//                // Byte 89 = 2100 =>> FM25V01
//                // Byte 89 = 2008 =>> FM25V02A

//                uint8_t size = 10;
//                uint8_t cypess_manufacture_data_tx[size];
//                uint8_t cypess_manufacture_data_rx[size];
//                cypess_manufacture_data_tx[0] = CMD_READ_INDENTIFICATION;

//                driver->callback.spi_cs(driver->spi, 0);
//                driver->callback.spi_tx_rx(driver->spi,
//                                           cypess_manufacture_data_tx,
//                                           cypess_manufacture_data_rx, size);
//                driver->callback.spi_cs(driver->spi, 1);
//                for (uint32_t i = 1; i < size; i++)
//                {
//                    DEBUG_RAW("%02X ", cypess_manufacture_data_rx[i]);
//                }
//                DEBUG_RAW("\r\n");

//                if (cypess_manufacture_data_rx[1] == 0x7F && cypess_manufacture_data_rx[2] == 0x7F 
//                    && cypess_manufacture_data_rx[3] == 0x7F && cypess_manufacture_data_rx[4] == 0x7F 
//                    && cypess_manufacture_data_rx[5] == 0x7F && cypess_manufacture_data_rx[6] == 0x7F 
//                    && cypess_manufacture_data_rx[8] == 0x22 && cypess_manufacture_data_rx[9] == 0x08)
//                {
//                    driver->info.device = APP_SPI_FRAM_FM25V02A;
//                    DEBUG_INFO("FM25V02A\r\n");
//                    val = true;
//                }
//                else if (cypess_manufacture_data_rx[1] == 0x7F && cypess_manufacture_data_rx[2] == 0x7F 
//                        && cypess_manufacture_data_rx[3] == 0x7F && cypess_manufacture_data_rx[4] == 0x7F 
//                        && cypess_manufacture_data_rx[5] == 0x7F && cypess_manufacture_data_rx[6] == 0x7F 
//                        && cypess_manufacture_data_rx[8] == 0x21 && cypess_manufacture_data_rx[9] == 0x00)
//                {
//                    driver->info.device = APP_SPI_FRAM_FM25V01;
//                    DEBUG_INFO("FM25V01\r\n");
//                    val = true;
//                }
//                else if (cypess_manufacture_data_rx[1] == 0x7F && cypess_manufacture_data_rx[2] == 0x7F 
//                        && cypess_manufacture_data_rx[3] == 0x7F && cypess_manufacture_data_rx[4] == 0x7F 
//                        && cypess_manufacture_data_rx[5] == 0x7F && cypess_manufacture_data_rx[6] == 0x7F 
//                        && cypess_manufacture_data_rx[8] == 0x24 && cypess_manufacture_data_rx[9] == 0x00)
//                {
//                    driver->info.device = APP_SPI_FRAM_FM25V10;
//                    DEBUG_INFO("FM25V10\r\n");
//                    val = true;
//                }
//            }
//            else
//            {
//                DEBUG_ERROR("Unknown device\r\n");
//                val = false;
//            }
//        }
//        else if (cmd == CMD_READ_INDENTIFICATION)
        {
            if (id.name.manu_id == 0xEF)        // Windbond
            {
                DEBUG_VERBOSE("Winbond\r\n");
                if (0 /*id.name.device_id[0] == 0x40 && id.name.device_id[1] == 0x17*/)
                {
                    DEBUG_VERBOSE("W25Q65\r\n");
                    driver->info.device = APP_SPI_FLASH_W25Q64;
                    val = true;
                }
            }
            else if (id.name.manu_id == 0xC8)        // Giga device
            {
                if (id.name.mem_type == 0x40 && id.name.capacity == 0x17)
                {
                    driver->info.device = APP_SPI_FLASH_GD25Q64;
                    val = true;
                }
            }
//            if (id.name.device_id[0] == 0x17 && id.name.device_id[1] == 0xC8)
//            {
//                DEBUG_VERBOSE("Winbond\r\n");
//                if (id.name.manufacture_id[0] == 0xC8 && id.name.manufacture_id[1] == 0x40)
//                {
//                    DEBUG_VERBOSE("W25Q32JV\r\n");
//                    driver->info.device = APP_SPI_FLASH_W25Q32;
//                    val = true;
//                }
//            }
            if (id.name.manu_id == 0x9D && id.name.mem_type == 0x60
                && id.name.capacity == 0x16)
            {
                driver->info.device = APP_SPI_FLASH_IS25LP032D;
                val = true;
            }
//            else if (id.name.manufacture_id[0] == 0x04 && id.name.manufacture_id[1] == 0x7F)
//            {
//                if (id.name.device_id[0] == 0x01 && id.name.device_id[1] == 0x01)
//                {
//                    DEBUG_INFO("MB85RS16\r\n");
//                    driver->info.device = APP_SPI_FRAM_MB85RS16;
//                    val = true;
//                }
//                else if (id.name.device_id[0] == 0x03 && id.name.device_id[1] == 0x02)
//                {
//                    DEBUG_INFO("MB85RS64V\r\n");
//                    driver->info.device = APP_SPI_FRAM_MB85RS64V;
//                    val = true;
//                }
//                else if (id.name.device_id[0] == 0x24 && id.name.device_id[1] == 0x03)
//                {
//                    DEBUG_INFO("MB85RS128TYPNF\r\n");
//                    driver->info.device = APP_SPI_FRAM_MB85RS128TYPNF;
//                    val = true;
//                }
//            }
//            else if (id.name.manufacture_id[0] == 0x1F && id.name.manufacture_id[1] == 0x89) /* APP_SPI_FLASH_AT25SF128 */
//            {
//                DEBUG_INFO("Adesto Technologies\r\n");
//                driver->info.device = APP_SPI_FLASH_AT25SF128;
//                DEBUG_INFO("AT25SF128A\r\n");
//                val = true;
//            }
//            else if (id.name.manufacture_id[0] == 0x7F && id.name.manufacture_id[1] == 0x7F) /* APP_SPI_FLASH_AT25SF128 */
//            {
//                DEBUG_INFO("Cypress\r\n");
//                // Cypress read total 9 byte,
//                // 6 byte MSB = 7F7F7F7F7F7F
//                // Byte 7 done care
//                // Byte 89 = 2100 =>> FM25V01
//                // Byte 89 = 2008 =>> FM25V02A

//                uint8_t size = 10;
//                uint8_t cypess_manufacture_data_tx[10];
//                uint8_t cypess_manufacture_data_rx[10];
//                cypess_manufacture_data_tx[0] = CMD_READ_INDENTIFICATION;

//                driver->callback.spi_cs(driver->spi, 0);
//                driver->callback.spi_tx_rx(driver->spi,
//                                           cypess_manufacture_data_tx,
//                                           cypess_manufacture_data_rx,
//                                           10);
//                driver->callback.spi_cs(driver->spi, 1);
//                for (uint32_t i = 1; i < size; i++)
//                {
//                    DEBUG_RAW("%02X ", cypess_manufacture_data_rx[i]);
//                }
//                DEBUG_RAW("\r\n");

//                if (cypess_manufacture_data_rx[1] == 0x7F && cypess_manufacture_data_rx[2] == 0x7F 
//                    && cypess_manufacture_data_rx[3] == 0x7F && cypess_manufacture_data_rx[4] == 0x7F 
//                    && cypess_manufacture_data_rx[5] == 0x7F && cypess_manufacture_data_rx[6] == 0x7F 
//                    && cypess_manufacture_data_rx[8] == 0x22 && cypess_manufacture_data_rx[9] == 0x08)
//                {
//                    driver->info.device = APP_SPI_FRAM_FM25V02A;
//                    DEBUG_INFO("FM25V02A\r\n");
//                    val = true;
//                }
//                else if (cypess_manufacture_data_rx[1] == 0x7F && cypess_manufacture_data_rx[2] == 0x7F 
//                        && cypess_manufacture_data_rx[3] == 0x7F && cypess_manufacture_data_rx[4] == 0x7F 
//                        && cypess_manufacture_data_rx[5] == 0x7F && cypess_manufacture_data_rx[6] == 0x7F 
//                        && cypess_manufacture_data_rx[8] == 0x21 && cypess_manufacture_data_rx[9] == 0x00)
//                {
//                    driver->info.device = APP_SPI_FRAM_FM25V01;
//                    DEBUG_INFO("FM25V01\r\n");
//                    val = true;
//                }
//                else if (cypess_manufacture_data_rx[1] == 0x7F && cypess_manufacture_data_rx[2] == 0x7F 
//                        && cypess_manufacture_data_rx[3] == 0x7F && cypess_manufacture_data_rx[4] == 0x7F 
//                        && cypess_manufacture_data_rx[5] == 0x7F && cypess_manufacture_data_rx[6] == 0x7F 
//                        && cypess_manufacture_data_rx[8] == 0x24 && cypess_manufacture_data_rx[9] == 0x00)
//                {
//                    driver->info.device = APP_SPI_FRAM_FM25V10;
//                    DEBUG_INFO("FM25V10\r\n");
//                    val = true;
//                }
//            }
        }

        if (val)
        {
            break;
        }
    }
    return val;
}

app_spi_flash_status_register_t app_spi_flash_read_status(app_flash_drv_t *driver)
{
    app_spi_flash_status_register_t status;
    
    driver->callback.spi_cs(driver->spi, 0);
    driver->callback.spi_tx_byte(driver->spi, CMD_READ_STATUS_REGISTER);
    driver->callback.spi_rx_buffer(driver->spi, &status.val, 1);
    driver->callback.spi_cs(driver->spi, 1);
    
    return status;
}

bool app_spi_flash_erase_all(app_flash_drv_t *driver, uint32_t timeout_ms, bool wait_until_cplt)
{
    uint8_t cmd;
    bool retval = false;
    DEBUG_INFO("Erase all flash\r\n");
    cmd = CMD_ERASE_ALL;
    
    flash_write_control(driver, 1);
    
    app_spi_flash_status_register_t status;
    
    status = app_spi_flash_read_status(driver);
    DEBUG_VERBOSE("Before erase : status register 0x%08X\r\n", status.val);
    
    driver->callback.spi_cs(driver->spi, 0);
    driver->callback.spi_tx_byte(driver->spi, cmd);
    driver->callback.spi_cs(driver->spi, 1);
            
    if (wait_until_cplt)
    {
        /* Read status register */
        if (timeout_ms < 1000)
        {
            timeout_ms = 1000;
        }
        driver->callback.delay_ms(driver, 1000);
        timeout_ms -= 1000;
        retval = app_spi_flash_wait_erase_complete(driver, timeout_ms);
    }
    else
    {
        retval = true;
    }

#if APP_SPI_FLASH_VERIFY_AFTER_WRITE
    bool found_error = false;
    uint32_t old_addr = 0;
    uint8_t buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    for (uint32_t i = 0; i < 4096; i++)      // Debug only
    {
        uint32_t tmp;
        app_spi_flash_read_bytes(driver, old_addr + i, (uint8_t*)&tmp, 4);
        if (memcmp(&tmp, buffer, 4))
        {
            found_error = true;
            DEBUG_ERROR("Flash erase error at addr 0x%08X, readback 0x%08X\r\n", 
                        old_addr + i, 
                        tmp);
            break;
        }
    }
    if (found_error == false)
    {
        DEBUG_VERBOSE("Erase success\r\n");
    }
#endif

    if (retval)
    {
        DEBUG_VERBOSE("Erase [DONE]\r\n");
    }
    else
    {
        DEBUG_ERROR("Erase flash failed\r\n");
        driver->error = true;
    }
    return retval;
}

void app_spi_flash_wakeup(app_flash_drv_t *driver)
{
#if APP_SPI_FLASH_LOW_POWER_MODE
    for (uint8_t i = 0; i < 1; i++)
    {
        driver->callback.spi_cs(driver->spi, 0);
        uint8_t cmd = CMD_WAKEUP_FROM_DEEP_POWER_DOWN;
        driver->callback.spi_tx_byte(driver->spi, cmd);
        // Dummy data
        cmd = 0x00;
        driver->callback.spi_tx_byte(driver->spi, cmd);
        driver->callback.spi_tx_byte(driver->spi, cmd);
        driver->callback.spi_tx_byte(driver->spi, cmd);
        driver->callback.spi_tx_byte(driver->spi, cmd);
        
        driver->callback.spi_cs(driver->spi, 1);

        driver->callback.delay_ms(driver, 1);
    }
#endif
}

void app_spi_flash_shutdown(app_flash_drv_t *driver)
{
#if APP_SPI_FLASH_LOW_POWER_MODE
    driver->callback.spi_cs(driver->spi, 0);
    uint8_t cmd = CMD_DEEP_POWER_DOWN;
    driver->callback.spi_tx_byte(driver->spi, cmd);
    driver->callback.spi_cs(driver->spi, 1);

    // delay 3us
    driver->callback.delay_ms(driver, 1);
#endif
}

bool app_spi_flash_is_sector_empty(app_flash_drv_t *driver, uint32_t sector)
{
    bool retval = true;
    if (driver->info.type == APP_SPI_FRAM)
    {
        return true;
    }
    uint32_t addr = sector * driver->info.sector_size;
    for (uint32_t i = 0; i < driver->info.sector_size;) // Debug only
    {
        uint32_t tmp;
        app_spi_flash_read_bytes(driver, addr + i, (uint8_t *)&tmp, 4);
        if (tmp != 0xFFFFFFFF)
        {
            retval = false;
            break;
        }
        i += 4;
    }
    if (retval)
    {
//        DEBUG_VERBOSE("We need erase next sector %u\r\n", sector);
    }
    return retval;
}
