#include "app_eeprom.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_flash_ex.h"
#include "app_debug.h"
#include <string.h>
#include "flash_if.h"
#include "utilities.h"

#define APP_EEPROM_STORE_DATA_ADDR	        0x08080000
#define APP_EEPROM_STORE_FACTORY_ADDR       (0x08080000 + 2048)
#define APP_EEPROM_EXT_FLASH_POINTER_ADDR


static app_eeprom_config_data_t m_cfg;
static void app_eeprom_factory_data_initialize(void);
static bool m_fixed_wakeup_time = false;

uint32_t app_eeprom_get_cyber_pulse_ms(void)
{
    return m_cfg.cyber_pulse_ms;
}

void app_eeprom_init(void)
{
    app_eeprom_config_data_t *tmp = (app_eeprom_config_data_t*)APP_EEPROM_STORE_DATA_ADDR;
    uint32_t crc = utilities_calculate_crc32((uint8_t*)tmp, sizeof(app_eeprom_config_data_t) - HARDWARE_CRC32_SIZE);        // last 2 bytes old is crc
	if (tmp->valid_flag != APP_EEPROM_VALID_FLAG
        || tmp->crc != crc)
	{
		memset(&m_cfg, 0, sizeof(m_cfg));
        // for low power application, set transistor output to 1
		m_cfg.poll_config_interval_hour = 24;
		m_cfg.battery_low_percent = 20;
		m_cfg.max_sms_1_day = 4;
        
        // Cyber config
		m_cfg.io_enable.name.cyber_direction_level = 1;
       
		for (uint32_t i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
		{
			m_cfg.cyber_config[i].k = 1;
			m_cfg.cyber_config[i].mode = APP_EEPROM_METER_MODE_PWM_PLUS_DIR_MIN;
			m_cfg.cyber_config[i].offset = 0;
		}
        
        // Measure interval
        m_cfg.measure_interval_ms = APP_EEPROM_MEASURE_INTERVAL_MS;
        
        m_cfg.io_enable.name.input0 = 1;
        m_cfg.io_enable.name.input1 = 1;
        
        m_cfg.io_enable.name.input_4_20ma_0_enable = 1;
        m_cfg.io_enable.name.input_4_20ma_1_enable = 1;
        m_cfg.io_enable.name.input_pressure_sensor_0 = 1;
        m_cfg.io_enable.name.input_pressure_sensor_1 = 1;

        // RS485
        m_cfg.rs485_baudrate = APP_EEPROM_DEFAULT_BAUD;
        m_cfg.io_enable.name.rs485_byte_order = APP_EEPROM_MODBUS_MSB_FIRST;
        
        // Cyber pulse filter
        m_cfg.cyber_pulse_ms = APP_EEPROM_RECHECK_PULSE_TIMEOUT_MS;
        
        // time to sync clock
        m_cfg.sync_clock_in_hour = 24;      // 1 day
        
        memcpy(m_cfg.phone, "0", 1);
        m_cfg.send_to_server_interval_ms = APP_EEPROM_SEND_TO_SERVER_INTERVAL_S;
        m_cfg.valid_flag = APP_EEPROM_VALID_FLAG;
        sprintf((char*)&m_cfg.http_server_address[APP_EEPROM_MAIN_SERVER_ADDR_INDEX][0], 
                "%s", 
                APP_EEPROM_DEFAULE_SERVER_ADDR);
		m_cfg.http_server_address[1][0] = '\0'; 
        
        // Set invalid sync time
        for (int i = 0; i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME; i++)
        {
            m_cfg.sync_web_in_min[i] = APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME;
        }
        
        m_cfg.debug_timeout_second = 600;
        app_eeprom_save_config();
	}
    else
    {
        memcpy(&m_cfg, tmp, sizeof(app_eeprom_config_data_t));
        if (m_cfg.send_to_server_interval_ms == 0)
        {
            m_cfg.send_to_server_interval_ms = APP_EEPROM_SEND_TO_SERVER_INTERVAL_S;
        }
        if (strlen((char*)m_cfg.http_server_address[APP_EEPROM_MAIN_SERVER_ADDR_INDEX]) < 12
			&& strlen((char*)m_cfg.http_server_address[APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX]) < 12)
        {
            sprintf((char*)&m_cfg.http_server_address[APP_EEPROM_MAIN_SERVER_ADDR_INDEX], 
                    "%s", 
                    APP_EEPROM_DEFAULE_SERVER_ADDR); 
        }
		
		if (m_cfg.poll_config_interval_hour == 0)
		{
			m_cfg.poll_config_interval_hour = 24;
		}
		
		if (m_cfg.battery_low_percent == 0)
		{
			m_cfg.battery_low_percent = 20;
		}
    }
    
    app_eeprom_factory_data_initialize();
    
    uint32_t has_fixed_time = 0;
    for (int i = 0; i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME; i++)
    {
        if (m_cfg.sync_web_in_min[i] != APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME
            && m_cfg.sync_web_in_min[i] < 1441)      // max 1440 min/day
        {
            has_fixed_time++;
        }
    }
    
    if (has_fixed_time)
    {
        m_fixed_wakeup_time = true;
    }
    
}

void app_eeprom_erase(void)
{
    flash_if_init();
	uint32_t err;
	HAL_FLASHEx_DATAEEPROM_Unlock();
	for (uint32_t i = 0; (i < sizeof(app_eeprom_config_data_t)+3)/4; i++)
	{
		err = HAL_FLASHEx_DATAEEPROM_Erase(APP_EEPROM_STORE_DATA_ADDR + i*4);
		if (HAL_OK != err)
		{
//			DEBUG_ERROR("Erase eeprom failed at addr 0x%08X, err code %08X\r\n", EEPROM_STORE_DATA_ADDR + i*4, err);
			break;
		}
	}
	HAL_FLASHEx_DATAEEPROM_Lock();
}

void app_eeprom_save_config(void)
{	
	uint32_t err;
	uint8_t *tmp = (uint8_t*)&m_cfg;
    m_cfg.crc = utilities_calculate_crc32((uint8_t*)&m_cfg, sizeof(app_eeprom_config_data_t) - HARDWARE_CRC32_SIZE);        // last 4 bytes old is crc
    
	app_eeprom_erase();
    flash_if_init();
    
	
	HAL_FLASHEx_DATAEEPROM_Unlock();


	for (uint32_t i = 0; i < sizeof(app_eeprom_config_data_t); i++)
	{
		err = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, APP_EEPROM_STORE_DATA_ADDR + i, tmp[i]);
		if (HAL_OK != err)
		{
//			DEBUG_ERROR("Write eeprom failed at addr 0x%08X, err code %08X\r\n", EEPROM_STORE_DATA_ADDR + i, err);
			break;
		}
	}
	
//    if (err == HAL_OK)
//    {
//        DEBUG_VERBOSE("Store data success\r\n");
//    }
	HAL_FLASHEx_DATAEEPROM_Lock();
}

void app_eeprom_factory_data_initialize(void)
{
    app_eeprom_factory_data_t *factory_data = app_eeprom_read_factory_data(); 
    uint32_t crc = utilities_calculate_crc32((uint8_t*)factory_data, sizeof(app_eeprom_factory_data_t) - HARDWARE_CRC32_SIZE);        // last 4 bytes old is crc
    if (crc != factory_data->crc)
    {
        app_eeprom_factory_data_t new_data;
        memset(&new_data, 0, sizeof(app_eeprom_factory_data_t));
        memcpy(new_data.server, APP_EEPROM_DEFAULE_SERVER_ADDR, strlen(APP_EEPROM_DEFAULE_SERVER_ADDR));
        app_eeprom_save_factory_data(&new_data);
    }
}

void app_eeprom_save_factory_data(app_eeprom_factory_data_t *factory_data)
{
    uint32_t err;
	uint8_t *tmp = (uint8_t*)factory_data;
    factory_data->crc = utilities_calculate_crc32((uint8_t*)factory_data, sizeof(app_eeprom_factory_data_t) - HARDWARE_CRC32_SIZE);        // last 4 bytes old is crc
    
    flash_if_init();
    
	HAL_FLASHEx_DATAEEPROM_Unlock();


	for (uint32_t i = 0; i < sizeof(app_eeprom_factory_data_t); i++)
	{
		err = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, APP_EEPROM_STORE_FACTORY_ADDR + i, tmp[i]);
		if (HAL_OK != err)
		{
//			DEBUG_ERROR("Write eeprom failed at addr 0x%08X, err code %08X\r\n", EEPROM_STORE_FACTORY_ADDR + i, err);
			break;
		}
	}

	HAL_FLASHEx_DATAEEPROM_Lock();
}

app_eeprom_factory_data_t *app_eeprom_read_factory_data(void)
{
    return (app_eeprom_factory_data_t*)(APP_EEPROM_STORE_FACTORY_ADDR);
}

app_eeprom_config_data_t *app_eeprom_read_config_data(void)
{
	return &m_cfg;
} 


static int16_t m_sync_web_in_min[APP_EEPROM_MAX_FIXED_SEND_WEB_TIME];
int16_t *app_eeprom_get_fixed_time_wakeup_setting()
{
    for (int i = 0; i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME; i++)
    {
        m_sync_web_in_min[i] = m_cfg.sync_web_in_min[i];
    }
    
    return &m_sync_web_in_min[0];
}

void app_eeprom_set_fixed_time_wakeup_setting(int16_t *data)
{
    uint32_t has_fixed_time = 0;
    for (int i = 0; i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME; i++)
    {
        if (data[i] != APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME
            && data[i] < 1440)      // max 1440 min/day
        {
            has_fixed_time++;
        }
        if (data[i] != APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME
            && data[i] >= 1440)
        {
            data[i] = 1439;
        }
        m_cfg.sync_web_in_min[i] = data[i];
    }
    app_eeprom_save_config();
    
    m_fixed_wakeup_time = has_fixed_time ? true : false;
}

bool app_eeprom_has_fixed_time_wakeup(void)
{
    return m_fixed_wakeup_time;
}
