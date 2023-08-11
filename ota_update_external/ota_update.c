#include "ota_update.h"
#include "app_debug.h"
#include "string.h"
#include "flash_if.h"
#include "main.h"
#include "utilities.h"
#include "Flash.h"
#include "FRAM.h"
#if(_ENCRYPT_FLASH_)
#include "aes.h"
#endif

#define OTA_FUNC_PLACE(x)				x		// RAMFUNCTION_SECTION_CODE
#define OTA_TIMEOUT_MS                  (60000)


typedef struct
{
    uint8_t data[FLASH_IF_SECTOR_SIZE];      // ensure align 4
    uint32_t size;
} ota_bytes_remain_t;


static uint32_t m_expected_fw_size = 0;
static bool m_found_header = false;
static uint32_t m_current_write_size = 0;
static bool m_ota_is_running = false;
static ota_bytes_remain_t m_ota_remain;
volatile uint32_t ota_timeout_ms;
extern uint32_t last_byte_number;
uint32_t m_crc;

bool ota_update_is_running(void)
{
	return m_ota_is_running;
}

uint8_t ota_get_downloaded_percent(void)
{
    if (m_expected_fw_size == 0)
    {
        return 0;
    } 

    return (m_current_write_size * 100)/ m_expected_fw_size;
}

bool ota_update_start(uint32_t expected_size)
{
    bool retval = true;
    DEBUG_INFO("Firmware size %u bytes, download to addr 0x%08X\r\n", expected_size, EXTERNAL_OTA_UPDATE_APPLICATION_START_ADDR);
    if (ota_timeout_ms == 0)
    {
        ota_timeout_ms = OTA_TIMEOUT_MS;
        m_crc = 0;
    }
    if (expected_size > OTA_UPDATE_APPLICATION_SIZE)
    {
        retval = false;
    }
    else
    {
        m_expected_fw_size = expected_size; // - OTA_UPDATE_DEFAULT_HEADER_SIZE;
        m_current_write_size = 0;
        memset(&m_ota_remain, 0, sizeof(m_ota_remain));
        if (retval)
        {
            retval = flash_if_init();
            flash_if_error_t err;
#if OTA_ERASE_ALL_FLASH_BEFORE_WRITE
 
#if(!_USE_EXTERNAL_FLASH_)
            err = flash_if_erase(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR, expected_size);
#else
            err = flash_if_erase(EXTERNAL_OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR, expected_size);
#endif
            //Delayms(100);
#if(!_USE_EXTERNAL_FLASH_)
            err = flash_if_erase(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR, expected_size);
#else
           // uint8_t *app_addr = (uint8_t*)(OTA_UPDATE_APPLICATION_START_ADDR);
            /*for(uint8_t i = 0; i < (OTA_UPDATE_APPLICATION_SIZE/EXTERNAL_FLASH_BLOCK_SIZE); i++)
            {
                uint8_t read_back = 0xFF;
                FlashWriteBytes(EXTERNAL_FLASH_BACKUP_START_ADDRESS + i *EXTERNAL_FLASH_BLOCK_SIZE, (uint8_t*)app_addr, (uint16_t)EXTERNAL_FLASH_BLOCK_SIZE);
                Delayms(100);
                FlashReadBytes(EXTERNAL_FLASH_BACKUP_START_ADDRESS + i *EXTERNAL_FLASH_BLOCK_SIZE , &read_back , 1);
                if(*(app_addr + i * EXTERNAL_FLASH_BLOCK_SIZE) != read_back)
                {
                    DEBUG_ERROR("Error Write Flash Memory: Expected 0x%02x-readback: 0x%02x\r\n",*(app_addr + i * EXTERNAL_FLASH_BLOCK_SIZE),  read_back);
                }
            }*/
#endif
            
#else
            err = FLASH_IF_OK;
#endif
            
            if (err != FLASH_IF_OK)
            {
                DEBUG_ERROR("Erase flash error\r\n");
                retval = false;
            }

            m_found_header = false;
            m_ota_is_running = true;
        }
    }
    return retval;
}

 /**
  * Download firmware region detail
  * -------------------------------
  *         16 bytes header
  * -------------------------------  
  *         Raw firmware
  * -------------------------------
  *         4 bytes CRC32
  * -------------------------------
  */
bool ota_update_write_next(uint8_t *data, uint32_t length)
{
    // TODO write data to flash
    // ASSERT(length > 16)
	m_ota_is_running = true;
    
    // Step1 : Header must has same hardware version and same firmware type
    // Step2 : Write data to flash, exclude checksum
    if (m_found_header == false 
		&& m_current_write_size < sizeof(ota_image_header_t))
    {
        // TODO : verify length of data >= 16 byte
        ota_image_header_t image_info;
        memcpy(&image_info, data, sizeof(ota_image_header_t));
        if (memcmp(image_info.name.header, 
                    OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE, 
                    strlen(OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE)))
        {
            DEBUG_INFO("Invalid header\r\n");
            return false;
        }
        if (memcmp(image_info.name.hardware_version, 
                    OTA_UPDATE_DEFAULT_HEADER_DATA_HARDWARE, 
                    strlen(OTA_UPDATE_DEFAULT_HEADER_DATA_HARDWARE)))
        {
            DEBUG_INFO("Invalid hardware\r\n");
            return false;
        }

        
        m_found_header = true;
        DEBUG_INFO("Found header\r\n");
    }
    if (!m_found_header)
    {
        DEBUG_ERROR("Not found firmware header\r\n");
        return false;
    }
    
    if (m_current_write_size == m_expected_fw_size)
    {
        DEBUG_ERROR("File size is over\r\n");
        return false;
    }
    
    while (length && m_current_write_size < m_expected_fw_size)
    {
        uint32_t bytes_need_copy = FLASH_IF_SECTOR_SIZE - m_ota_remain.size;

        if (bytes_need_copy > length)
        {
            bytes_need_copy = length;
        }
        
        memcpy(&m_ota_remain.data[m_ota_remain.size], data, bytes_need_copy);
        length -= bytes_need_copy;
        data += bytes_need_copy;
        m_ota_remain.size += bytes_need_copy;
        
        if (m_ota_remain.size == FLASH_IF_SECTOR_SIZE)
        {
            DEBUG_INFO("Total write size %u, at addr 0x%08X\r\n", m_current_write_size + m_ota_remain.size, 
                                                                #if(!_USE_EXTERNAL_FLASH_)
                                                                OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size
                                                                #else
                                                                 EXTERNAL_OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size
                                                                 #endif
                                                                 );
            flash_if_error_t err = FLASH_IF_OK;
#if OTA_ERASE_ALL_FLASH_BEFORE_WRITE == 0
            err = flash_if_erase(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size, FLASH_IF_SECTOR_SIZE);
#endif
#if(!_USE_EXTERNAL_FLASH_)
            err += flash_if_copy(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size,
            											(uint32_t*)&m_ota_remain.data[0],
														m_ota_remain.size/4);
#else
            err += flash_if_copy(EXTERNAL_OTA_UPDATE_APPLICATION_START_ADDR + m_current_write_size,
            											(uint32_t*)&m_ota_remain.data[0],
														m_ota_remain.size/4);         
#endif
            if (err != FLASH_IF_OK)
            {
                DEBUG_ERROR("Write data to flash error\r\n");
                return false;
            }
            DEBUG_INFO("DONE\r\n");
            m_current_write_size += m_ota_remain.size;
            m_ota_remain.size = 0;
        }
        else
        {
            break;
        }
    }

    if (m_current_write_size >= m_expected_fw_size)
    {
        DEBUG_INFO("All data received\r\n");
    }
    
    return true;
}

void ota_update_set_expected_size(uint32_t size)
{
    m_expected_fw_size = size;
}

bool ota_update_commit_flash(void)
{
    // TODO write boot information
	bool retval = true;
    if (m_ota_remain.size)
    {
        DEBUG_INFO("Commit flash =>> Write final %u bytes, total %u bytes\r\n", 
                    m_ota_remain.size, 
                    m_current_write_size + m_ota_remain.size);

        for (uint32_t i = m_ota_remain.size; i < FLASH_IF_SECTOR_SIZE; i++)
        {
        	m_ota_remain.data[i] = 0xFF;
        }

        if (FLASH_IF_OK != flash_if_copy(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size, 
                                        (uint32_t*)&m_ota_remain.data[0], 
                                        FLASH_IF_SECTOR_SIZE/4))
        {
        	retval = false;
        }
        m_ota_remain.size = 0;
    }
    return retval;
}

OTA_FUNC_PLACE(void ota_update_write_header(uint32_t addr, ota_information_t *header))
{
    DEBUG_INFO("Write header at addr 0x%08X\r\n", addr);
    flash_if_erase(addr, OTA_UPDATE_INFORMATION_SIZE);
    
	memcpy(&m_ota_remain.data[0], (uint8_t*)header, sizeof(ota_information_t));
    if (flash_if_copy(addr, (uint32_t*)&m_ota_remain.data[0], (sizeof(ota_information_t) + 3)/4) != FLASH_IF_OK)
    {
        DEBUG_ERROR("Write data to flash error\r\n");
    }
    else
    {
        DEBUG_INFO("Write header done\r\n");
    }
}


bool ota_update_finish(bool status)
{
    bool retval = false;
    m_found_header = false;
    if (status)
    {
        // Check if remain bytes of data
        if (m_ota_remain.size)
        {
            DEBUG_INFO("Write final %u bytes, total %u bytes\r\n",
						m_ota_remain.size,
						m_current_write_size + m_ota_remain.size);
#if(!_USE_EXTERNAL_FLASH_)         
            flash_if_erase(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size, FLASH_IF_SECTOR_SIZE);
            flash_if_copy(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size,
            				(uint32_t*)&m_ota_remain.data[0],
							(m_ota_remain.size+3)/4);
        }
#else
            flash_if_copy(EXTERNAL_OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size,
            				(uint32_t*)&m_ota_remain.data[0],
							(m_ota_remain.size+3)/4);
        }
#endif
        if ((m_current_write_size + m_ota_remain.size) != m_expected_fw_size)
        {
            DEBUG_ERROR("Firmware download not complete\r\n");
        }
        else        
        {
            m_ota_remain.size = 0;
            // Verify checksum from download area to (firmware size - 4 bytes crc32), last 4 bytes is crc32
            //uint32_t header_size = sizeof(ota_image_header_t);
            if (ota_update_verify_checksum(EXTERNAL_OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR, m_expected_fw_size))
            {
                // Write data into ota information page, bootloader will check this page and perform firmware copy
                
                DEBUG_INFO("Valid checksum, write OTA flag\r\n");
                ota_information_t new_cfg;
                new_cfg.ota_flag = OTA_UPDATE_FLAG_UPDATE_NEW_FIRMWARE; // OTA_FLAG_UPDATE_NEW_FW;
                new_cfg.size = m_expected_fw_size;      // 4 is size of CRC32
                new_cfg.crc32 = m_crc;
#if(!_USE_EXTERNAL_FLASH_)
                ota_update_write_header(OTA_INFORMATION_START_ADDR, &new_cfg);
#else
                ota_update_write_header(EXTERNAL_OTA_INFORMATION_START_ADDR, &new_cfg);
#endif
                retval = true;
            }
            else
            {
                DEBUG_ERROR("Invalid checksum\r\n");        
            }       
        }
    }
    else
    {
        DEBUG_ERROR("OTA update failed\r\n");
    }
	
    m_current_write_size = 0;
    m_ota_remain.size = 0;
	m_ota_is_running = false;
    m_expected_fw_size = 0;
    memset(&m_ota_remain, 0, sizeof(m_ota_remain));
    return retval;
}

uint32_t ota_update_crc_by_sum(const uint8_t* data_p, uint32_t length)
{
    uint32_t crc = 0;
    length = length;
    DEBUG_INFO("Internal Flash CRC Lenght: %u\r\n", length);
    while (length--)
    {
        crc += *data_p++;
    }
    
    return crc;
}

uint32_t external_ota_update_crc_by_sum(uint32_t address, uint32_t length, uint32_t *read_crc)
{
    uint32_t crc = 0;
    uint8_t buffer[256+16];
    uint8_t output_buffer[256 + 16];
    uint32_t nb_of_byte_left = length;
    DEBUG_RAW("CRC Length: %u\r\n", length);
    /*Doc 256 byte dau. Tinh check sum phai tru di 4 bytes header*/
#if(_ENCRYPT_FLASH_)
    FlashReadBytes(address, buffer, sizeof(buffer));
    AES_ECB_decrypt(buffer, (uint8_t*)PUBLIC_KEY, output_buffer, sizeof(buffer));
    for (uint16_t i = 0 + sizeof(ota_image_header_t); i < 256; i++)
    {
        crc += output_buffer[i];
    }
    /*Tang Address doc len 256+ 16 don vi*/
    address = address + sizeof(buffer);
    nb_of_byte_left = nb_of_byte_left - sizeof(buffer);
#endif
    
    
#if(!_ENCRYPT_FLASH_)
    for(uint32_t i = 0; i <= (length/ sizeof(buffer)); i++)
    {
        if(nb_of_byte_left >= sizeof(buffer))
        {
            FlashReadBytes(address, buffer, sizeof(buffer));
            for(uint16_t buffer_index = 0; buffer_index < sizeof(buffer) ; buffer_index++)
            {
                crc += buffer[buffer_index];
            }
            address = address + sizeof(buffer);
            nb_of_byte_left = nb_of_byte_left - sizeof(buffer);
        }
        else if(nb_of_byte_left > 0)
        {
            FlashReadBytes(address, buffer, nb_of_byte_left);
            for(uint16_t buffer_index = 0; buffer_index < nb_of_byte_left; buffer_index++)
            {
                crc += buffer[buffer_index];
            }
            address = address + nb_of_byte_left;
            nb_of_byte_left = 0;
        }
    }
#else
    for(uint32_t i = 1; i <= (length/ sizeof(buffer)); i++)
    {
        if(nb_of_byte_left >= sizeof(buffer))
        {
            FlashReadBytes(address, buffer, sizeof(buffer));
            AES_ECB_decrypt(buffer, (uint8_t*)PUBLIC_KEY, output_buffer, sizeof(buffer));
            /*16 la size cua 1 block padding*/
            for(uint16_t buffer_index = 0; buffer_index < sizeof(buffer) - 16; buffer_index++)
            {
                crc += output_buffer[buffer_index];
            }
            address = address + sizeof(buffer);
            nb_of_byte_left = nb_of_byte_left - sizeof(buffer);
        }
        else if(nb_of_byte_left > 0)
        {
            FlashReadBytes(address, buffer, nb_of_byte_left);
            AES_ECB_decrypt(buffer, (uint8_t*)PUBLIC_KEY, output_buffer, sizeof(buffer));
            /*So luong byte lan cuoi khi ghi vao flash*/
            for(uint16_t buffer_index = 0; buffer_index < last_byte_number - 4; buffer_index++)
            {
                crc += output_buffer[buffer_index];
            }
            memcpy(read_crc, &output_buffer[last_byte_number - 4] , 4);
            address = address + nb_of_byte_left;
            nb_of_byte_left = 0;
        }
    }
#endif
    
    //DEBUG_INFO("Address of last crc byte 0x%08x\r\n", address);
    return crc;
}

bool ota_update_verify_checksum(uint32_t begin_addr, uint32_t length)
{
#if 0
	// Last 16 bytes of firmware is the md5 checksum value
	DEBUG_INFO("Verify checksum from addr 0x%08X, len %u\r\n", begin_addr, length);
    app_md5_ctx md5_cxt;
    uint32_t checksum_addr;
    uint32_t *page_data = (uint32_t*)&m_ota_remain.data[0];
    __ALIGNED(4) uint8_t expected_md5[16];
    uint32_t number_word = (length - 16) / 4;
    app_md5_init(&md5_cxt);
    uint8_t md5_result[OTA_UPDATE_MD5_CHECKSUM_SIZE];

    // Calculate MD5
    for (uint32_t i = 0; i < number_word; i++)
    {
    	vdm_wdt_feed();
    	flash_if_read(begin_addr, page_data, 4);
        app_md5_update(&md5_cxt, (uint8_t *)begin_addr, 4);
    	begin_addr += 4;
    }
    app_md5_final(md5_result, &md5_cxt);

    // Read md5 in binary firmware
    flash_if_read(begin_addr, (uint32_t*)&expected_md5[0], 16);

    // Debug
    DEBUG_INFO("Expected md5\r\n");
    for (uint32_t i = 0; i < 16; i++)
    {
    	DEBUG_RAW("%02X ", expected_md5[i]);
    }
    DEBUG_RAW("\r\n");

    DEBUG_INFO("Calculated md5\r\n");
    for (uint32_t i = 0; i < 16; i++)
    {
    	DEBUG_RAW("%02X ", md5_result[i]);
    }
    DEBUG_RAW("\r\n");

    // Compare
    if (memcmp(md5_result, (uint8_t *)expected_md5, OTA_UPDATE_MD5_CHECKSUM_SIZE) == 0)
    {
    	DEBUG_INFO("Checksum is valid\r\n");
        return true;
    }
    else
    {
    	DEBUG_ERROR("Checksum is error\r\n");
        return false;
    }
#else
    // First 16 bytes is image header, so skip it
    // Last 4 bytes is CRC32 of firmware
    
    begin_addr += sizeof(ota_image_header_t);
    length -= sizeof(ota_image_header_t);
    length -= 4;        // sizeof crc
    
    
#if(!_USE_EXTERNAL_FLASH_)
    uint32_t crc = ota_update_crc_by_sum((uint8_t*)begin_addr, length);
    uint32_t expected_crc = *((uint32_t*)(begin_addr+length));
#else
    uint8_t expected_crc_buff[4];

    uint32_t expected_crc = 0;
    uint32_t crc = external_ota_update_crc_by_sum(begin_addr, length, &expected_crc);
    FlashReadBytes(begin_addr + length, expected_crc_buff, 4);
    memcpy(&expected_crc, expected_crc_buff, sizeof(uint32_t));
    DEBUG_INFO("Read CRC at 0x%08x\r\n", begin_addr + length);
    DEBUG_RAW("Expected 0x%08X, calculated 0x%08X\r\n", expected_crc, crc);
#endif
    //DEBUG_INFO("Estimate crc from 0x%08X, size %u bytes, last value 0x%08X\r\n", begin_addr, length, *((uint32_t*)(begin_addr+length)));
    if (expected_crc == crc)
    {
        DEBUG_INFO("CRC valid\r\n");
        m_crc = expected_crc;
        return true;
    }
    return false;
#endif
}

bool app_ota_is_all_data_received(void)
{
    if (m_expected_fw_size && 
        m_current_write_size >= m_expected_fw_size)
    {
        return true;
    }
    return false;
}
