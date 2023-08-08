#include "app_flash_log.h"
#include "app_debug.h"
#include "utilities.h"
#include "usart.h"
#include "gsm.h"
#include <string.h>
#include "app_eeprom.h"
#include "version_control.h"
#include "app_spi_flash.h"

#define APP_FLASH_LOG_ERASE_TIME 120000
#define APP_FLASH_HEADER_DONT_NEED_ERASE_FLASH 0x12347689
#define APP_FLASH_HEADER_BUFFER_VALID 0x7613
#define APP_FLASH_DATA_BUFFER_VALID 0x9912
#define LOG_DATA_OFFSET 4096
#define FLASH_LOG_LOCK()    
#define FLASH_LOG_UNLOCK()
#define SIZE_OF_LOG_DATA(x,y) (x + y)
#define BUF_MIN(x, y) ((x) < (y) ? (x) : (y))
#define BUF_MAX(x, y) ((x) > (y) ? (x) : (y))

#define FLASH_FEED_WATCHDOG()       LL_IWDG_ReloadCounter(IWDG)

static uint32_t m_current_header_addr = 0;
static const uint16_t m_sizeof_ring_buffer_pointer = sizeof(app_flash_ringbuffer_pointer_t);

/**
 * @brief       Store new message into flash
 * @param[in]   driver Flash driver
 * @param[in]   data Pointer to data
 * @param[in]   size Size of data
 */
static void flash_store_new_message(app_flash_drv_t *driver, uint8_t* data, uint16_t size);


void app_flash_log_measurement_data(app_flash_drv_t *driver, app_flash_log_data_t *wr_data)
{
    if (driver->error)
    {
//        DEBUG_ERROR("Flash init error, ignore write msg\r\n");
        return;
    }
    DEBUG_INFO("Flash write new data\r\n");
    uint32_t size = sizeof(app_flash_log_data_t);
    // Get CRC
    wr_data->crc = utilities_calculate_crc32((uint8_t*)wr_data, 
                                            sizeof(app_flash_log_data_t) - HARDWARE_CRC32_SIZE);
    flash_store_new_message(driver, (uint8_t*)wr_data, size);
}

void build_json_to_rs485(app_flash_log_data_t* data, uint32_t total_msg, uint32_t packet_num)
{
    // Build message
    uint32_t len = 0;
    char buffer[128];
    char* ptr = buffer;
    len += snprintf(ptr+len, 128, "{\"TotalPacket\":%lu,\"CurrentPacket\":%lu,", total_msg, packet_num);

    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += snprintf(ptr+len, 128, "\"Timestamp\":%lu,", data->timestamp);
    
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += snprintf((char *)(ptr + len), 128, "\"BatteryLevel\":%u,", data->internal_battery_voltage);
    len += snprintf((char *)(ptr + len), 128, "\"Temperature\":%d,", data->temp);

    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

 	len += snprintf((char *)(ptr + len), 128, "\"ID\":\"G2-%s\",", gsm_get_module_imei());
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    // Counter
    for (uint32_t i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
    {
        // Build input pulse counter
        if (data->counter[i].k == 0)
        {
            data->counter[i].k = 1;
        }
        len += snprintf((char *)(ptr + len), 128, "\"Input1_J%u\":%u,",
                        i+1,
                        data->counter[i].real_counter/data->counter[i].k + data->counter[i].indicator);
        len += snprintf((char *)(ptr + len), 128, "\"Input1_J%u_D\":%u,",
                        i+1,
                        data->counter[i].reverse_counter/data->counter[i].k /* + data->counter[i].indicator */);
        usart_lpusart_485_send((uint8_t*)ptr, len);
        len = 0;
    }

    // Build input 4-20ma
    for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; i++)
    {
        len += snprintf((char *)(ptr + len), 
                        128, 
                        "\"Input1_J3_%u\":%.3f,",
                        i+1,
                        data->input_4_20mA[0]); // dau vao 4-20mA 0
    }
    
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    
    // Build analog pressure
    for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; i++)
    {
        len += snprintf((char *)(ptr + len), 
                        128, 
                        "\"analogInput%u\":%u,",
                        i,
                        data->input_pressure_analog[0]); // dau vao 4-20mA 0
    }
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    
    // Build input on/off digital
    len += snprintf((char *)(ptr + len), 128, "\"Input1_J9_%u\":%u,",
                                                            1,
                                                            data->on_off.name.input_on_off_0);
        
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += snprintf((char *)(ptr + len), 128, "\"Input1_J9_%u\":%u,",
                                            2,
                                            data->on_off.name.input_on_off_1);

    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;				

    //485			
    for (uint32_t index = 0; index < HARDWARE_RS485_MAX_SLAVE_ON_BUS; index++)
    {
        if (data->rs485[index].slave_addr == 0)
        {
            continue;
        }
        len += snprintf((char *)(ptr + len), 
                        128, 
                        "\"SlaveID%u\":%u,", 
                        index+1, 
                        data->rs485[index].slave_addr);
        
        // Scan modbus register
        for (uint32_t sub_idx = 0; sub_idx < HARDWARE_RS485_MAX_SUB_REGISTER; sub_idx++)
        {
            if (data->rs485[index].sub_reg[sub_idx].read_ok
                && data->rs485[index].sub_reg[sub_idx].data_type.name.valid)
            {
                len += snprintf((char *)(ptr + len), 128, "\"Register%u_%u\":", index+1, sub_idx+1);
                usart_lpusart_485_send((uint8_t*)ptr, len);
                len = 0;

                if (data->rs485[index].sub_reg[sub_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_INT16 
                    || data->rs485[index].sub_reg[sub_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_INT32)
                {
                    len += snprintf((char *)(ptr + len), 
                                128,
                                "%u,", 
                                data->rs485[index].sub_reg[sub_idx].value.int32_val);
                }
                else
                {
                    len += snprintf((char *)(ptr + len),
                                128,
                                "%.2f,", 
                                (float)data->rs485[index].sub_reg[sub_idx].value.float_val);
                }
            }
            else if (data->rs485[index].sub_reg[sub_idx].data_type.name.valid)
            {
                if (strlen((char*)data->rs485[index].sub_reg[sub_idx].unit))
                {
                    len += snprintf((char *)(ptr + len), 128, 
                                    "\"Unit%u_%u\":\"%s\",",
                                    index+1, sub_idx+1, data->rs485[index].sub_reg[sub_idx].unit);
                }
                len += snprintf((char *)(ptr + len), 128,  "\"Register%u_%u\":%s,", index+1, sub_idx+1, "-1");
                usart_lpusart_485_send((uint8_t*)ptr, len);
                len = 0;
            }	
            usart_lpusart_485_send((uint8_t*)ptr, len);
            len = 0;
        }
    }

    // output 4-20mA
    len += snprintf((char*)(ptr + len),
        128,
        "\"Output4_20\":%.3f", data->output_4_20mA[0]);   // dau ra 4-20mA 0
    usart_lpusart_485_send((uint8_t*)ptr, len);

    if (len && ptr[len-1] == ',')
    {
        ptr[len-1] = 0;
        len--;
    }
    len = 0;

    len += snprintf(ptr+len, 128, "%s", "}");
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    ptr[len] = 0;
}

void* memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len)
{
#ifndef _LIBC
# define __builtin_expect(expr, val)   (expr)
#endif

    /* not really Rabin-Karp, just using additive hashing */
    char* haystack_ = (char*)haystack;
    char* needle_ = (char*)needle;
    int hash = 0;   /* this is the static hash value of the needle */
    int hay_hash = 0;	/* rolling hash over the haystack */
    char* last;
    size_t i;

    if (haystack_len < needle_len)
        return NULL;

    if (!needle_len)
        return haystack_;

    /* initialize hashes */
    for (i = needle_len; i; --i)
    {
        hash += *needle_++;
        hay_hash += *haystack_++;
    }

    /* iterate over the haystack */
    haystack_ = (char*)haystack;
    needle_ = (char*)needle;
    last = haystack_ + (haystack_len - needle_len + 1);
    for (; haystack_ < last; ++haystack_)
    {
        if (__builtin_expect(hash == hay_hash, 0)  	/* prevent calling memcmp, was a optimization from existing glibc */
            && *haystack_ == *needle_
            && !memcmp(haystack_, needle_, needle_len))
        {
            return haystack_;
        }

        /* roll the hash */
        hay_hash -= *haystack_;
        hay_hash += *(haystack_ + needle_len);
    }

    return NULL;
}


/**
 * DTG01
    {
        "TotalPacket": 12345,               // S? d? li?u s? g?i
        "CurrentPacket": 12345,             // Th? t? packet dang g?i hi?n t?i
        "Error": "cam_bien_xung_dut",       // ?? debug, kh?ng c?n x? l?
        "Timestamp": 1629200614,
        "ID": "G1-860262050125363",
        "Input1": 124511,                   // Ki?u int
        "Inputl_J3_1":	0.01,				// Ki?u float
        "BatteryLevel": 80,
        "Vbat": 4101,                      // ?? debug, kh?ng c?n x? l?


        "SlaveID1" : 3,                         // ??a ch? slave
        "Register1_1": 64,
        "Unit1_1":"m3/s",

        "Register1_2": 339,
        "Unit1_2":"jun",

// C?c tru?ng Register_x_y c? th? l? s? int ho?c float, N?u modbus d?c l?i th? gi? tr? l? FFFF c?i n?y do server y?u c?u, ki?u int hay float cung l? server y?u c?u
        "Register1_3": 64,                      
        "Unit1_3":"m3/s",

        "Register1_4": 12.3,	
        "Unit1_4":"jun",                        // ?on vi tinh 
                    
        "SlaveID2" : 4,                         // ?ia chi salve
        "Register2_1": 0.0000,                  // Luu ?, n?u server k c?i d?t modbus th? c?c tru?ng n?y s? ko c? ? json
        "Unit2_1":"kg",

        "Register2_2": 0,
        "Unit2_2":"1it",
        
        "Register2_3": 0.0000,
        "Unit2_3":"kg",
        
        "Register2_4": 0,
        "Unit2_4":"lit",

        "Temperature": 26,                  // Nhi?t d?
        "SIM": 452018100001935,             // Sim imei
        "Uptime": 7,                        // ?? debug, kh?ng c?n x? l?
        "FW": "0.0.5",                      // Firmware version
        "HW": "0.0.1"                       // Hardware version
    }
 */
uint32_t scan_for_logger_data(app_flash_drv_t *driver, uint32_t total_msg, bool output_to_rs485)
{
    if (driver->error)
    {
        DEBUG_ERROR("Flash error\r\n");
        return 0;
    }
    uint32_t valid_counter = 0;
    uint32_t number_of_sector = (driver->info.size)/driver->info.sector_size - 1; // -1 : sector 0 user for ringbuffer pointer, we start from sector 1
    
    const uint16_t header2 = APP_FLASH_DATA_BUFFER_VALID;
    uint8_t* q = driver->working_buffer;
    uint32_t header_struct_size = sizeof(app_flash_log_data_header_t);

    // i = 1 : sector 0 user for ringbuffer pointer, we start from sector 1
    for (uint32_t i = 1; i < number_of_sector+1; i++)
    {
        HARDWARE_LED1_TOGGLE();
        FLASH_FEED_WATCHDOG();
        app_spi_flash_read_bytes(driver, 
                                i*driver->info.sector_size, 
                                driver->working_buffer, 
                                driver->info.sector_size);
        int32_t size = driver->info.sector_size;
        uint8_t *tmp = driver->working_buffer;
        uint32_t offset = 0;
        bool found = false;
        while (size > 0)
        {
            q = memmem(tmp, size, &header2, 2);

            if (q)
            {
                found = true;
                offset = (size_t)q - (size_t)driver->working_buffer;
                size -= ((size_t)q - (size_t)tmp);
                tmp = q + 2;

                // Case 1 : all header data is stored at 1 sector
                app_flash_log_data_header_t header_data;
                if (offset <= (driver->info.sector_size - header_struct_size))
                {
                    memcpy(&header_data, q, header_struct_size);
                    // Data in 2 sector
                    if (offset + header_data.size >= driver->info.sector_size)
                    {
                        app_flash_log_data_t wr;
                        uint32_t remain;
                        uint8_t* ptr = (uint8_t*)& wr;
                        uint32_t crc;
                        uint32_t need_first = driver->info.sector_size - offset - header_struct_size;
                        uint32_t next_sector_addr = (i + 1)*driver->info.sector_size;
                        if (next_sector_addr >= driver->info.size)
                        {
                            next_sector_addr = LOG_DATA_OFFSET;
                        }
                        
                        remain = driver->info.sector_size - offset - header_struct_size;
                        
                        if (need_first <= sizeof(app_flash_log_data_t))
                        {
                            memcpy(ptr, driver->working_buffer + offset + header_struct_size, need_first);
                            remain = header_data.size - need_first;

                            // Read data in the next sector
                            app_spi_flash_read_bytes(driver, next_sector_addr, driver->working_buffer, remain);
                            memcpy(ptr+ need_first, driver->working_buffer, remain);

                            crc = utilities_calculate_crc32(ptr, sizeof(app_flash_log_data_t) - HARDWARE_CRC32_SIZE);
                            if (crc == wr.crc)
                            {
                                //size = 0;
                                valid_counter++;
                                if (output_to_rs485)
                                {
                                    DEBUG_INFO("Sending data, counter %u/%u\r\n", valid_counter, total_msg);
                                    build_json_to_rs485(&wr, total_msg, valid_counter);
    //                                sys_delay_ms(1);
                                }
                            }
                        }
 /*                       else
                        {
                            DEBUG_ERROR("Invalid CRC\r\n");
                        }*/

                        // No need to scan sector again
                        size = 0;
                    }
                    else        // data is in one sector
                    {
                        if (header_data.size)
                        {
                            uint32_t crc;
                            app_flash_log_data_t* ptr = (app_flash_log_data_t*)(q + header_struct_size);
                            crc = utilities_calculate_crc32((uint8_t*)ptr, sizeof(app_flash_log_data_t) - HARDWARE_CRC32_SIZE);
                            if (crc == ptr->crc)
                            {
                                //size = 0;
                                valid_counter++;
                                if (output_to_rs485)
                                {
                                    DEBUG_INFO("Sending data, counter %u/%u\r\n", valid_counter, total_msg);
                                    build_json_to_rs485(ptr, total_msg, valid_counter);
                                    // sys_delay_ms(1);
                                }
                            }
                        }
                    }
                }
     
            }
            else 
            {
                size = 0;
            }
        }
        if (!found 
            && driver->working_buffer[driver->info.sector_size-1] == (header2 >> 8)) 
        {
            // Case 2 : header nam o 2 sector =>> luoi nen ko xu li
        }
    }
    return valid_counter;
}


uint32_t app_flash_log_send_to_host(app_flash_drv_t *driver)
{
    if (driver->error)
    {
        return 0;
    }

    //RS485_POWER_EN(1);
    //usart_rs485_control(true);
    //sys_delay_ms(100);
    //RS485_DIR_TX();
    usart_lpusart_485_send((uint8_t*)"Hawaco.Datalogger.PingMessage", strlen("Hawaco.Datalogger.PingMessage"));
    uint32_t total_msg; // = scan_for_logger_data(0, false);
    sys_delay_ms(100);
    
    usart_rs485_change_baudrate(115200);
    FLASH_FEED_WATCHDOG();
    sys_delay_ms(5000); // delay for winform app process data
    total_msg = scan_for_logger_data(driver, 0, true);
    uint8_t config[128];
    uint32_t len = 0;
    
    char *ptr = (char*)config;     // TODO check malloc result
    len = 0;
   
    app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
    len += snprintf((char*)(config + len), 
                    APP_EEPROM_MAX_SERVER_ADDR_LENGTH, 
                    "{\"Svr0\":\"%s\",", 
                    eeprom_cfg->http_server_address[0]);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    
    len += snprintf((char*)(config + len), 
                    APP_EEPROM_MAX_SERVER_ADDR_LENGTH, 
                    "\"Svr1\":\"%s\",", 
                    eeprom_cfg->http_server_address[1]);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    
    len += sprintf((char *)(config + len), 
                    "\"CycleSendWeb\":%u,", 
                    eeprom_cfg->send_to_server_interval_ms/1000/60);
    
    len += sprintf((char *)(config + len), 
                    "\"DelaySendToServer\":%u,", 
                    eeprom_cfg->send_to_server_delay_s);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    
    len += sprintf((char *)(config + len), 
                    "\"Cyclewakeup\":%u,", 
                    eeprom_cfg->measure_interval_ms/1000/60);
    len += sprintf((char *)(config + len), 
                    "\"MaxSmsOneday\":%u,", 
                    eeprom_cfg->max_sms_1_day);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += sprintf((char *)(config + len), "\"Phone\":\"%s\",", eeprom_cfg->phone);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;
    
    len += sprintf((char *)(config + len), "\"PollCfg\":%u,", eeprom_cfg->poll_config_interval_hour);
    len += sprintf((char *)(config + len), "\"DirLvl\":%u,", eeprom_cfg->io_enable.name.cyber_direction_level);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    for (uint32_t i = 0; i < APP_EEPROM_METER_MODE_MAX_ELEMENT; i++)
    {
        len += sprintf((char *)(config + len), "\"K%u\":%u,", i+1, eeprom_cfg->cyber_config[i].k);
        len += sprintf((char *)(config + len), "\"M%u\":%u,", i+1, eeprom_cfg->cyber_config[i].mode);
        len += sprintf((char *)(config + len), "\"MeterIndicator%u\":%u,", i+1, eeprom_cfg->cyber_config[i].offset);
        usart_lpusart_485_send((uint8_t*)ptr, len);
        len = 0;
    }
   
    len += sprintf((char *)(config + len), "\"Ip0\":%u,", eeprom_cfg->io_enable.name.input0);
    len += sprintf((char *)(config + len), "\"Ip1\":%u,", eeprom_cfg->io_enable.name.input1);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += sprintf((char *)(config + len), "\"SOS\":%u,", eeprom_cfg->io_enable.name.sos);
    len += sprintf((char *)(config + len), "\"Warning\":%u,", eeprom_cfg->io_enable.name.warning);
    len += sprintf((char *)(config + len), "\"485_E\":%u,", eeprom_cfg->io_enable.name.rs485_en);
    if (eeprom_cfg->io_enable.name.rs485_en)
    {
        for (uint32_t slave_idx = 0; slave_idx < HARDWARE_RS485_MAX_SLAVE_ON_BUS; slave_idx++)
        {
            for (uint32_t sub_reg_index = 0; sub_reg_index < HARDWARE_RS485_MAX_SUB_REGISTER; sub_reg_index++)
            {
                if (eeprom_cfg->rs485[slave_idx].sub_reg[sub_reg_index].data_type.name.valid == 0)
                {
                    uint32_t slave_addr = eeprom_cfg->rs485[slave_idx].sub_reg[sub_reg_index].read_ok = 0;
                    continue;
                }
                len += sprintf((char *)(config + len), "\"485_%u_Sl\":%u,", 
                                slave_idx, 
                                eeprom_cfg->rs485[slave_idx].slave_addr);
                len += sprintf((char *)(config + len), "\"485_%u_Rg\":%u,", 
                                slave_idx, eeprom_cfg->rs485[slave_idx].sub_reg[sub_reg_index].reg_addr);
                usart_lpusart_485_send((uint8_t*)ptr, len);
                len = 0;
                
                len += snprintf((char *)(config + len), 12, 
                                "\"485_%u_U\":\"%s\",", 
                                slave_idx, (char*)eeprom_cfg->rs485[slave_idx].sub_reg[sub_reg_index].unit);
                usart_lpusart_485_send((uint8_t*)ptr, len);
                len = 0;

            }
        }
    }

    len += sprintf((char *)(config + len), 
                    "\"ip4_20mA_0\":%u,", 
                    eeprom_cfg->io_enable.name.input_4_20ma_0_enable);
    len += sprintf((char *)(config + len), 
                    "\"ip4_20mA_1\":%u,", 
                    eeprom_cfg->io_enable.name.input_4_20ma_1_enable);
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += sprintf((char *)(config + len), "\"Ot4_20mA_En\":%u,", 
                    eeprom_cfg->io_enable.name.output_4_20ma_enable);
    if (eeprom_cfg->io_enable.name.output_4_20ma_enable)
    {
        len += sprintf((char *)(config + len), 
                        "\"Ot4_20mA_Val\":%.3f,", 
                        eeprom_cfg->output_4_20ma);
    }
   
    len += sprintf((char *)(config + len), "\"FW\":\"%s\",", VERSION_CONTROL_FW);
    len += sprintf((char *)(config + len), "\"HW\":\"%s\",", VERSION_CONTROL_HW);
   
    usart_lpusart_485_send((uint8_t*)ptr, len);
    len = 0;

    len += sprintf((char *)(config + len), 
                    "\"Fsv\":\"%s\",", 
                    app_eeprom_read_factory_data()->server);
   
    len--;      // Skip ','
    len += sprintf((char *)(config + len), "%s", "}");     
    usart_lpusart_485_send((uint8_t*)config, len);
   
    usart_rs485_change_baudrate(9600);
    
    HARDWARE_RS485_DIR_RX();
    usart_rs485_control(false);
    HARDWARE_RS485_POWER_EN(0);

    return total_msg;
}


app_flash_ringbuffer_pointer_t m_ringbuffer_read_write;


static uint32_t log_available_space_for_write_operation(uint32_t write_index, 
                                                        uint32_t read_index, 
                                                        uint32_t max_ringbuffer_size)
{
    uint32_t size;

    /* Use temporary values in case they are changed during operations */
    if (write_index == read_index)
    {
        size = max_ringbuffer_size;
    }
    else if (read_index > write_index)
    {
        size = read_index - write_index;
    }
    else
    {
        size = max_ringbuffer_size - (write_index - read_index);
    }

    /* Buffer free size is always 1 less than actual size */
    return size - 1;
}

static uint32_t log_available_space_for_read_operation(uint32_t write_index, 
                                                        uint32_t read_index, 
                                                        uint32_t max_ringbuffer_size)
{
    uint32_t size;

    if (write_index == read_index)
    {
        size = 0;
    }
    else if (write_index > read_index)
    {
        size = write_index - read_index;
    }
    else
    {
        size = max_ringbuffer_size - (read_index - write_index);
    }
    return size;
}


static void commit_ringbuffer_pointer(app_flash_drv_t *driver) // commit flash_read_write pointer
{
    if (driver->error)
    {
        return;
    }
    
    m_ringbuffer_read_write.valid_flag = APP_FLASH_HEADER_BUFFER_VALID;
    uint32_t next_addr = m_current_header_addr + m_sizeof_ring_buffer_pointer;
    if (next_addr >= (uint32_t)(driver->info.sector_size - m_sizeof_ring_buffer_pointer))
    {
        DEBUG_WARN("Log ringbuffer pointer : Reach to end of header sector\r\n");
        app_spi_flash_erase_sector_4k(driver, 0);
        // Write initial header data
        uint8_t header_data[4 + sizeof(app_flash_ringbuffer_pointer_t)];
        uint32_t commit_size = 0;
        // m_ringbuffer_read_write.wr_index = driver->info.sector_size;
        // m_ringbuffer_read_write.rd_index = driver->info.sector_size;
        uint32_t need_erase_header = APP_FLASH_HEADER_DONT_NEED_ERASE_FLASH;

        memcpy(header_data, &need_erase_header, 4);
        memcpy(header_data + 4, &m_ringbuffer_read_write, m_sizeof_ring_buffer_pointer);
        commit_size = 4 + m_sizeof_ring_buffer_pointer;

        app_spi_flash_write(driver, 0, header_data, commit_size);
        m_current_header_addr = 4;
    }
    else
    {
//      DEBUG_INFO("Log pointer at addr %u\r\n", next_addr);
        app_spi_flash_write(driver,
                        next_addr,
                        (uint8_t*)&m_ringbuffer_read_write,
                        m_sizeof_ring_buffer_pointer);
        m_current_header_addr = next_addr;
    }
}


void flash_store_new_message(app_flash_drv_t *driver, uint8_t* data, uint16_t size)
{
    FLASH_LOG_LOCK();
    
    if (driver->error)
    {
        FLASH_LOG_UNLOCK();
        return;
    }
    
    FLASH_FEED_WATCHDOG();
    app_flash_log_data_header_t new_header_data;
    uint32_t log_header_size = sizeof(app_flash_log_data_header_t);
    new_header_data.header = APP_FLASH_DATA_BUFFER_VALID;
    new_header_data.size = size;
    //new_header_data.resend_to_server = 1;
    // Do 3.1
    /*
        Step0
        - Kiem tra xem con bao nhieu bytes free trong flash
        - Neu thieu bo nho =>> Xoa bot cac du lieu cu
                                    =>> Xoa bot bang cach tang con tro read_index
        3.1  - Con tro write pointer se tang dan moi khi co du lieu moi
                - Khi con tro write pointer > Flash size - sizeof(sector0) // sector 0 la sector chua header
                        ->> Quay ve vi tri bat dau (after sector0) de ghi tiep data
                - Neu khi ghi data, ma con tro write = (con tro read -1) =>> Bo dem vong full
                - Neu bo dem vong full, tang con tro read data len += data size
        3.2 - con tro read pointer
            - Moi khi doc xong data, tang con tro rd_data += data size
            - Doc den khi trong thi thoi
    */

    // Step 0
    uint32_t free_space;
    uint32_t write_size = SIZE_OF_LOG_DATA(size, log_header_size);
    while (1)
    {
        free_space = log_available_space_for_write_operation(m_ringbuffer_read_write.wr_index, 
                                                            m_ringbuffer_read_write.rd_index, 
                                                            driver->info.size - driver->info.sector_size);
        DEBUG_VERBOSE("Free space %u bytes, need %u bytes, prev rx index %u\r\n", 
                    free_space, write_size, m_ringbuffer_read_write.rd_index);
        if (free_space > write_size)
        {
            break;
        }
        else
        {
            return;
        }
    }

    // Write data
    // a. If we have enough space in flash =>> commit immediately data
    // b. Else we write last sector in flash, and return the first sector to write data
    uint32_t write_sector_index = m_ringbuffer_read_write.wr_index / driver->info.sector_size;
    uint32_t sector_address = write_sector_index * driver->info.sector_size;
    if (m_ringbuffer_read_write.wr_index + write_size > driver->info.size) // flash memory is full
    {
        DEBUG_WARN("Flash full\r\n");
        if (1)
        {
            // DEBUG_WARN("Sector is not empty\r\n");
            // Flash need erase before write =>> copy content of current sector
            char* sector_content = (char*)driver->working_buffer;
            uint32_t header_written_bytes = 0;
            uint8_t* header_ptr = (uint8_t*)&new_header_data;
            m_ringbuffer_read_write.previous_write_index = m_ringbuffer_read_write.wr_index;
            while (1)
            {
                // Readback data in current sector
                if (sector_address == driver->info.size)
                {
                    sector_address = LOG_DATA_OFFSET;
                }
                
                app_spi_flash_read_bytes(driver, 
                                        sector_address, 
                                        (uint8_t*)sector_content, 
                                        driver->info.sector_size);

                // Insert write data : header + data
                uint32_t start_write_point_in_sector = m_ringbuffer_read_write.wr_index - sector_address;
                for (uint32_t i = start_write_point_in_sector; i < driver->info.sector_size && size; i++)
                {
                    if (header_written_bytes < log_header_size) // Write header
                    {
                        sector_content[i] = header_ptr[header_written_bytes];
                        header_written_bytes++;
                    }
                    else // Write data
                    {
                        sector_content[i] = *data++;
                        size--;
                    }
                    m_ringbuffer_read_write.wr_index++;
                    if (m_ringbuffer_read_write.wr_index == driver->info.size)
                    {
                        // Flash full, goto sector offset
                        m_ringbuffer_read_write.wr_index = LOG_DATA_OFFSET;
                    }
                }
                // Commit data
                FLASH_FEED_WATCHDOG();
                app_spi_flash_erase_sector_4k(driver, write_sector_index);
                app_spi_flash_write(driver, 
                                    sector_address, 
                                    (uint8_t*)sector_content, 
                                    driver->info.sector_size);

                uint32_t remain = size + log_header_size - header_written_bytes;
                if (remain)
                {
                    // If data still remain =>> goto sector 1 (sector 0 used for read_write pointer)
                    sector_address = LOG_DATA_OFFSET;
                    m_ringbuffer_read_write.wr_index = LOG_DATA_OFFSET + remain;
                    write_sector_index = 1;

                    // Fix bug
                    app_spi_flash_read_bytes(driver, 
                                            sector_address, 
                                            (uint8_t*)sector_content, 
                                            driver->info.sector_size);
                    for (uint32_t i = 0; i < remain; i++)
                    {
                        if (header_written_bytes < log_header_size) // Write header
                        {
                            sector_content[i] = header_ptr[header_written_bytes];
                            header_written_bytes++;
                        }
                        else // Write data
                        {
                            sector_content[i] = *data++;
                        }
                    }
                    app_spi_flash_erase_sector_4k(driver, LOG_DATA_OFFSET/driver->info.sector_size);
                    app_spi_flash_read_bytes(driver, 
                                            sector_address, 
                                            (uint8_t*)sector_content, 
                                            driver->info.sector_size);
                    break;
                    // End
                }
                else
                {
                    DEBUG_VERBOSE("0 wr %u, rd %u\r\n", 
                                m_ringbuffer_read_write.wr_index,
                                m_ringbuffer_read_write.rd_index);
                    break;
                }
            }
        }
    }
    else // Flash is not full and not overflow
    {
        // Flash need erase before write =>> copy content cof current sector
        char* sector_content = (char*)driver->working_buffer;
        uint32_t header_written_bytes = 0;
        uint8_t* header_ptr = (uint8_t*)&new_header_data;
        m_ringbuffer_read_write.previous_write_index = m_ringbuffer_read_write.wr_index;
        FLASH_FEED_WATCHDOG();
        // TODO think about sector empty and ignore write all page data
        while (1)
        {
            app_spi_flash_read_bytes(driver, 
                                    sector_address, 
                                    (uint8_t*)sector_content, 
                                    driver->info.sector_size);

            // Insert write data : header + data
            // Beginning addr = write pointer address - sector address
            uint32_t start_write_point_in_sector = m_ringbuffer_read_write.wr_index - sector_address;
            for (uint32_t i = start_write_point_in_sector; 
                        i < driver->info.sector_size && size; 
                        i++)
            {
                if (header_written_bytes < log_header_size) // Write header
                {
                    sector_content[i] = header_ptr[header_written_bytes];
                    header_written_bytes++;
                }
                else // Write data
                {
                    sector_content[i] = *data++;
                    size--;
                }
                m_ringbuffer_read_write.wr_index++;
                if (m_ringbuffer_read_write.wr_index == driver->info.size)
                {
                    m_ringbuffer_read_write.wr_index = LOG_DATA_OFFSET;
                }
            }

            FLASH_FEED_WATCHDOG();
            app_spi_flash_erase_sector_4k(driver, write_sector_index);
            app_spi_flash_write(driver, 
                                sector_address, 
                                (uint8_t*)sector_content, 
                                driver->info.sector_size);

            if (header_written_bytes < log_header_size || size)
            {
                // If data still remain =>> goto next sector
                sector_address += driver->info.sector_size;
                write_sector_index++;
            }
            else
            {
                DEBUG_VERBOSE("1 - wr %u, rd %u\r\n", 
                            m_ringbuffer_read_write.wr_index,
                            m_ringbuffer_read_write.rd_index);
                break;
            }
        }
    }
    FLASH_LOG_UNLOCK();
    commit_ringbuffer_pointer(driver);
    DEBUG_VERBOSE("Log operation complete, wr %u, rd %u\r\n", 
                            m_ringbuffer_read_write.wr_index,
                            m_ringbuffer_read_write.rd_index);
}

bool app_flash_memory_log_data_is_available(app_flash_drv_t *driver)
{
    // return false;       // test huytv
    FLASH_LOG_LOCK();
    if (driver->error)
    {
        FLASH_LOG_UNLOCK();
        return false;
    }
    
    DEBUG_VERBOSE("WR %u, RD %u, ringbuffer size %u\r\n", m_ringbuffer_read_write.wr_index, 
                                    m_ringbuffer_read_write.rd_index,
                                    driver->info.size - driver->info.sector_size);      // 1 sector for ringbuffer pointer
    bool retval = log_available_space_for_read_operation(m_ringbuffer_read_write.wr_index,
                                                        m_ringbuffer_read_write.rd_index,
                                                        driver->info.size - driver->info.sector_size) ? true : false;
    FLASH_LOG_UNLOCK();
    return retval;
}

app_flash_mem_error_t app_flash_log_the_get_last_message(app_flash_drv_t *driver, uint8_t** message, uint16_t* size)
{
    FLASH_FEED_WATCHDOG();
    FLASH_LOG_LOCK();
    if (driver->error)
    {
        FLASH_LOG_UNLOCK();
        return APP_SPI_FLASH_MEM_PHYSIC_ERROR;
    }
    
    app_flash_mem_error_t retval = APP_SPI_FLASH_MEM_OK;
    // *message = NULL;
    *size = 0;
    uint32_t bytes_availble = log_available_space_for_read_operation(m_ringbuffer_read_write.wr_index,
                                                                m_ringbuffer_read_write.previous_write_index,
                                                                driver->info.size - driver->info.sector_size);
    if (bytes_availble) // Flash ringbuffer is not empty
    {
        // DEBUG_INFO("Log buffer %u bytes\r\n", bytes_availble);
        // Read header first to get data size
        app_flash_log_data_header_t header_data;
        uint32_t header_size = sizeof(app_flash_log_data_header_t);
        memset(&header_data, 0, header_size);
        uint32_t real_data_index;
        if (m_ringbuffer_read_write.previous_write_index == driver->info.size)
        {
            m_ringbuffer_read_write.previous_write_index = driver->info.sector_size;
        }
        uint32_t rd_index = m_ringbuffer_read_write.previous_write_index;
        // case0: read data will overflow flash
        if (rd_index + header_size > driver->info.size)
        {
            DEBUG_INFO("RD overflow\r\n");
            uint32_t read_counter_over_flow = rd_index + header_size - driver->info.size;
            uint8_t* ptr = (uint8_t*)&header_data;
            uint32_t read_first = driver->info.size - rd_index;
            if (read_first)
            {
                app_spi_flash_read_bytes(driver, rd_index, ptr, read_first);
                ptr += read_first;
            }
            if (read_counter_over_flow)
            {
                app_spi_flash_read_bytes(driver, 
                                        driver->info.sector_size, 
                                        ptr, 
                                        read_counter_over_flow);
            }
            real_data_index = read_counter_over_flow + driver->info.sector_size;
        }
        else // case 1 data not overflow flash
        {
            app_spi_flash_read_bytes(driver, rd_index, (uint8_t*)&header_data, header_size);
            real_data_index = rd_index + header_size;
        }

        if (header_data.header == APP_FLASH_DATA_BUFFER_VALID)
        {
            if (header_data.size == 0)
            {
                retval = APP_SPI_FLASH_MEM_ERROR_RING_BUFFER_EMPTY;
                *message = NULL;
                // TODO find next valid data
            }
            else
            {
                if (real_data_index == driver->info.size)
                {
                    real_data_index = driver->info.sector_size;
                }

                // Case 1 message is in 1 sector
                if (real_data_index + header_data.size < driver->info.size)
                {
                    app_spi_flash_read_bytes(driver, 
                                            real_data_index, 
                                            driver->working_buffer, 
                                            header_data.size);
                    *message = driver->working_buffer;
                    *size = header_data.size;
                    retval = APP_SPI_FLASH_MEM_OK;
                }
                else // message in 2 sector : the last sector and 1st sector
                {
                    uint32_t sector_addr = real_data_index / driver->info.sector_size;

                    sector_addr *= driver->info.sector_size;
                    uint32_t read_first = driver->info.sector_size - (real_data_index - sector_addr);
                    app_spi_flash_read_bytes(driver, 
                                            real_data_index, 
                                            driver->working_buffer, 
                                            read_first);
                    app_spi_flash_read_bytes(driver, 
                                            driver->info.sector_size, 
                                            &driver->working_buffer[read_first], 
                                            header_data.size - read_first);

                    *message = (uint8_t*)driver->working_buffer;
                    *size = header_data.size;
                    retval = APP_SPI_FLASH_MEM_OK;
                }
            }
        }
        else
        {
            DEBUG_INFO("Invalid header 0x%08x\r\n", header_data.header);
            retval = APP_SPI_FLASH_MEM_ERROR_DATA_CORRUPT;
        }
    }
    else
    {
        retval = APP_SPI_FLASH_MEM_ERROR_RING_BUFFER_EMPTY;
    }
    FLASH_LOG_UNLOCK();
    return retval;
}

app_flash_mem_error_t api_flash_log_read_current_message(app_flash_drv_t *driver, uint8_t** message, uint16_t* size)
{
    FLASH_FEED_WATCHDOG();
    FLASH_LOG_LOCK();
    app_flash_mem_error_t retval = APP_SPI_FLASH_MEM_OK;
    if (driver->error)
    {
        retval = APP_SPI_FLASH_MEM_ERROR_DATA_CORRUPT;
        FLASH_LOG_UNLOCK();
        return retval;
    }
    
    // *message = NULL;
    *size = 0;
    uint32_t bytes_available = log_available_space_for_read_operation(m_ringbuffer_read_write.wr_index,
                                                                    m_ringbuffer_read_write.rd_index,
                                                                    driver->info.size - driver->info.sector_size);      // reserve 1 sector for ringbuffer pointer
    if (bytes_available) // Flash ringbuffer is not empty
    {
        // DEBUG_INFO("Log buffer %u bytes\r\n", bytes_available);
        // Read header first to get data size
        app_flash_log_data_header_t header_data;
        uint32_t header_size = sizeof(app_flash_log_data_header_t);
        memset(&header_data, 0, header_size);
        uint32_t real_data_index;
        if (m_ringbuffer_read_write.rd_index == driver->info.size)
        {
            m_ringbuffer_read_write.rd_index = driver->info.sector_size;
        }
        uint32_t rd_index = m_ringbuffer_read_write.rd_index;
        // case0: read data will overflow flash
        if (rd_index + header_size > driver->info.size)
        {
            DEBUG_INFO("RD overflow\r\n");
            uint32_t read_counter_over_flow = rd_index + header_size - driver->info.size;
            uint8_t* ptr = (uint8_t*)&header_data;
            uint32_t read_first = driver->info.size - rd_index;
            if (read_first)
            {
                app_spi_flash_read_bytes(driver, rd_index, ptr, read_first);
                ptr += read_first;
            }
            if (read_counter_over_flow)
            {
                app_spi_flash_read_bytes(driver, driver->info.sector_size, ptr, read_counter_over_flow);
            }
            real_data_index = read_counter_over_flow + driver->info.sector_size;
        }
        else // case 1 data not overflow flash
        {
            app_spi_flash_read_bytes(driver, rd_index, (uint8_t*)&header_data, header_size);
            real_data_index = rd_index + header_size;
        }

        if (header_data.header == APP_FLASH_DATA_BUFFER_VALID)
        {
            if (header_data.size == 0)
            {
                retval = APP_SPI_FLASH_MEM_OK;
                *message = (void*)0;
                m_ringbuffer_read_write.rd_index = real_data_index;
                // TODO find next valid data
            }
            else
            {
                if (real_data_index == driver->info.size)
                {
                    real_data_index = driver->info.sector_size;
                }

                // Case 1 message is in 1 sector
                if (real_data_index + header_data.size < driver->info.size)
                {
                    // Read data
                    app_spi_flash_read_bytes(driver, 
                                            real_data_index, 
                                            driver->working_buffer, 
                                            header_data.size);
                    
                    m_ringbuffer_read_write.rd_index = real_data_index + header_data.size;
                    *message = driver->working_buffer;
                    *size = header_data.size;
                    retval = APP_SPI_FLASH_MEM_OK;
                }
                else // message in 2 sector : the last sector and 1st sector
                {
                    uint32_t sector_addr = real_data_index / driver->info.sector_size;

                    sector_addr *= driver->info.sector_size;
                    uint32_t read_first = driver->info.sector_size - (real_data_index - sector_addr);
                    DEBUG_WARN("Flash content in last sector and the first sector %u-%u-%u\r\n", 
                                read_first, header_data.size - read_first, header_data.size);
                    
                    app_spi_flash_read_bytes(driver, 
                                            real_data_index, driver->working_buffer, 
                                            read_first);
                    app_spi_flash_read_bytes(driver, 
                                            driver->info.sector_size, 
                                            &driver->working_buffer[read_first], 
                                            header_data.size - read_first);

                    *message = (uint8_t*)driver->working_buffer;
                    *size = header_data.size;
                    m_ringbuffer_read_write.rd_index = driver->info.sector_size + header_data.size - read_first;
                    retval = APP_SPI_FLASH_MEM_OK;
                }
            }
        }
        else
        {
            DEBUG_INFO("Invalid header 0x%04x\r\n", header_data.header);
            retval = APP_SPI_FLASH_MEM_ERROR_DATA_CORRUPT;
        }
    }
    else
    {
        retval = APP_SPI_FLASH_MEM_ERROR_RING_BUFFER_EMPTY;
    }
    FLASH_LOG_UNLOCK();
    return retval;
}

void flash_read_pointer_skip_bytes(app_flash_drv_t *driver, uint32_t bytes)
{
    FLASH_LOG_LOCK();
    if (driver->error)
    {
        FLASH_LOG_UNLOCK();
        return;
    }
    
    FLASH_FEED_WATCHDOG();
    
    uint32_t bytes_available = log_available_space_for_read_operation(m_ringbuffer_read_write.wr_index,
                                                                    m_ringbuffer_read_write.rd_index,
                                                                    driver->info.size - driver->info.sector_size);      // reserver 1 sector
    if (bytes_available)
    {
        uint32_t skipped = BUF_MIN(bytes, bytes_available);
        m_ringbuffer_read_write.rd_index += skipped;
        if (m_ringbuffer_read_write.rd_index >= driver->info.size)
        {
            m_ringbuffer_read_write.rd_index -= driver->info.size;
            m_ringbuffer_read_write.rd_index += LOG_DATA_OFFSET;
        }
        commit_ringbuffer_pointer(driver);
    }
    FLASH_LOG_UNLOCK();
}

void app_flash_log_find_read_write_pointer(app_flash_drv_t *driver)
{
    
    /*  Sector                                  MEMORY
     *  0    -----4 bytes flag ----/---------Header0------Size0-------/ ..... ---------HeaderN------SizeN-------
     *  1   User data
     *  2   User data
     *
     *
     *                                      QUY LUAT TIM POINTER
     * 1. Neu 4 bytes flag khong hop le -> Flash moi tinh -> 
                                            1.1 Erase flash
                                            1.2 update header
     * 2. Neu 4 byte hop le -> Scan 1 sector de tim con tro hop le
     * 3. Neu khong scan tim ra duoc con tro hop le -> Erase sector, quay lai buoc 1
     */
    FLASH_LOG_LOCK();
    
    if (driver->error)
    {
        FLASH_LOG_UNLOCK();
        return;
    }
    
    uint32_t need_erase_header = 0;
    // Read 4 byte flag
    app_spi_flash_read_bytes(driver, 0, (uint8_t*)&need_erase_header, 4);
    
    // Init default value
    m_ringbuffer_read_write.valid_flag = APP_FLASH_HEADER_BUFFER_VALID;
    m_ringbuffer_read_write.previous_write_index = LOG_DATA_OFFSET;
    m_ringbuffer_read_write.wr_index = LOG_DATA_OFFSET;
    m_ringbuffer_read_write.rd_index = LOG_DATA_OFFSET;

    app_flash_ringbuffer_pointer_t temporary_pointer;
    memset(&temporary_pointer, 0, m_sizeof_ring_buffer_pointer);
    
    // Case 1
    if (need_erase_header != APP_FLASH_HEADER_DONT_NEED_ERASE_FLASH)
    {
        DEBUG_INFO("Erase all flash, header 0x%08X\r\n", need_erase_header);
        
        // Case 2
        need_erase_header = APP_FLASH_HEADER_DONT_NEED_ERASE_FLASH;
        
        // Erase flash
        FLASH_FEED_WATCHDOG();
        
        // Case 1.2 Erase flash with timeout
        app_spi_flash_erase_all(driver, APP_FLASH_LOG_ERASE_TIME, true);
        
        // Case 1.1
        m_ringbuffer_read_write.wr_index = LOG_DATA_OFFSET + sizeof(app_flash_log_data_header_t);
        // Write header : Key + first struct app_flash_ringbuffer_pointer_t
        uint8_t header_data[64];
        uint32_t commit_size = 0;

        memcpy(header_data, &need_erase_header, sizeof(need_erase_header));

        memcpy(header_data + 4, &m_ringbuffer_read_write, m_sizeof_ring_buffer_pointer);
        commit_size = sizeof(need_erase_header) + m_sizeof_ring_buffer_pointer;
    
        // commit to flash
        app_spi_flash_write(driver, 0, header_data, commit_size);
        

        // Write first data : size = 0, data = 0;
        app_flash_log_data_header_t new_data;
        new_data.header = APP_FLASH_DATA_BUFFER_VALID;
        new_data.size = 0;
        app_spi_flash_write(driver, 
                            LOG_DATA_OFFSET, 
                            (uint8_t*)&new_data, 
                            sizeof(app_flash_log_data_header_t));
    }

    // Case 3
    uint32_t scan_addr = 4; // danh 4 byte dau tien cua flash cho header key
    bool found_valid = false;
    while (1)
    {
        // Read data
        app_spi_flash_read_bytes(driver, 
                                scan_addr, 
                                (uint8_t*)&temporary_pointer, 
                                m_sizeof_ring_buffer_pointer);
        scan_addr += m_sizeof_ring_buffer_pointer;

        // If found a valid data =>> Set current write_addr = scan_address - sizeof_pointer, and scan next packet
        if (temporary_pointer.valid_flag == APP_FLASH_HEADER_BUFFER_VALID)
        {
            m_ringbuffer_read_write.previous_write_index = temporary_pointer.previous_write_index;
            m_ringbuffer_read_write.wr_index = temporary_pointer.wr_index;
            m_ringbuffer_read_write.rd_index = temporary_pointer.rd_index;
            m_current_header_addr = scan_addr - m_sizeof_ring_buffer_pointer;
            found_valid = true;
        }
        // Else if dont found valid data
        // 0.1 : If scan address >= sector size =>> Finish scan sequence
        // 0.2 : If scan address < sector size =>> do 0.2.1 and 0.2.2
        // 0.2.1 : If previous data is valid =>> quit
        // 0.2.2 : If previous data is invalid =>> continues scan
        else
        {
            // 0.2.1
            if (scan_addr >= driver->info.sector_size)
            {
                DEBUG_WARN("Reach to the end of 1st sector, erase now\r\n");
                scan_addr = 4;
                m_current_header_addr = 4;
                
                app_spi_flash_erase_sector_4k(driver, 0);

                // Write initial header data
                uint8_t header_data[4 + sizeof(app_flash_ringbuffer_pointer_t)];
                m_ringbuffer_read_write.wr_index = LOG_DATA_OFFSET; // dia diem bat dau doc-ghi la sector 1
                m_ringbuffer_read_write.rd_index = LOG_DATA_OFFSET;
                need_erase_header = APP_FLASH_HEADER_DONT_NEED_ERASE_FLASH;

                memcpy(header_data, &need_erase_header, 4);

                memcpy(header_data + 4, &m_ringbuffer_read_write, m_sizeof_ring_buffer_pointer);

                app_spi_flash_write(driver, 0, header_data, sizeof(header_data));
                break;
            }
            // 0.2.2
            else if (found_valid)
            {
                break;
            }
        }
    }
    FLASH_LOG_UNLOCK();
    DEBUG_VERBOSE("Flash : header at addr %u, write pointer %u, read pointer %u\r\n",
                m_current_header_addr,
                m_ringbuffer_read_write.wr_index,
                m_ringbuffer_read_write.rd_index);
}
