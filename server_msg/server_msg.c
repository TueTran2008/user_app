#include "server_msg.h"
#include "string.h"
#include "gsm_utilities.h"
#include "app_bkup.h"
#include "gsm.h"
#include "utilities.h"
#include "app_eeprom.h"
#include "hardware.h"
#include "version_control.h"
#include "app_debug.h"
#include "app_spi_flash.h"
#include "app_flash_log.h"
#include "app_rtc.h"
#include "measure_input.h"
#include "umm_malloc.h"
#include "spi.h"

#define RS485_STRCAT_ID(str,id)				str##id##"\":"		

static app_eeprom_config_data_t *m_eeprom_config = NULL; //app_eeprom_read_config_data();
static sys_ctx_t *m_ctx = NULL;// sys_ctx();
static uint8_t process_flow_speed_enable(char *buffer);
static bool poll_server = 0;

/**
	 {
		"shared": {
			"CycleSendWeb": 60,
			"Cyclewakeup": 15,
			"ID485_1": 8,
			"ID485_2": 8,
			"IMEI": "860262050125777",
			"Input_J1": 1,
			"Input_J2": 0,
			"K_J1": 1,
			"K_J2": 1,
			"Link": "https://iot.wilad.vn/login",
			"MeterIndicator_J1": 7649,
			"MeterIndicator_J2": 7649,
			"Output1": 0,
			"Output2": 0,
			"Output3": 0,
			"Output4": 0,
			"Output4_20": 0,
			"Register_1_1": 30108,			// upto 4 register * 2 device
			"Register_1_2": 30110,
			"Register_2_1": 30112,
			"Register_2_2": 30113,
			"RS485": 1,
			"SOS": "0916883454",
			"Type": "G2",
			"Type_1_1": "int32",		// int32, int16, float
			"Type_1_2": "int32",
			"Type_2_1": "float",
			"Type_2_2": "int16",
			"Unit_1_1": "m3/s",
			"Unit_1_2": "jun",
			"Unit_2_1": "kg",
			"Unit_2_2": "lit",
			"Update": 0,
			"Version": "0.0.1",
			"Warning": 1,
			"Server":"http://123.com",
			"Auto_config":12,		// hour
            "NetFl_1":123124      // Thanh ghi so nuoc
						"DataTime":
		}
	}
 */
 
// "Server":"https://123.com" 
static void process_server_addr_change(char *buffer)
{
    char *server_update = strstr(buffer, "\"ServerUpdate\":1");
    if (server_update)
    {
        DEBUG_RAW("Found server update command\r\n");
        poll_server = true;
    }
    else
    {
        DEBUG_ERROR("Cannot found server update command:%sr\n", buffer);
        return;
    }
    
	buffer = strstr(buffer, "\"Server\":");
    if (buffer == NULL)
    {
        DEBUG_ERROR("Found no server config\r\n");
        return;
    }
    
    buffer += strlen("\"Server\":");
    uint8_t tmp[APP_EEPROM_MAX_SERVER_ADDR_LENGTH] = {0};
    if (poll_server &&
        gsm_utilities_copy_parameters(buffer, (char*)tmp, '"', '"')
        && (strstr((char*)tmp, "http://") || strstr((char*)tmp, "https://")))
    {
        poll_server = false;
        uint32_t server_addr_len = strlen((char*)tmp);
        --server_addr_len;
        if (tmp[server_addr_len] == '/')		// Change https://acb.com/ to https://acb.com
        {
            tmp[server_addr_len] = '\0';
        }
        
        if (strcmp((char*)tmp, (char*)m_eeprom_config->http_server_address[APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX])
            && m_ctx->status.try_new_server == 0)
        {
            if (m_ctx->status.new_server)
            {
    //                umm_free(m_ctx->status.new_server);
                m_ctx->status.new_server = NULL;
            }
            static char new_server[APP_EEPROM_MAX_SERVER_ADDR_LENGTH];
            m_ctx->status.new_server = new_server;
            if (m_ctx->status.new_server)
            {
                m_ctx->status.try_new_server = 2;
                snprintf((char*)m_ctx->status.new_server, APP_EEPROM_MAX_SERVER_ADDR_LENGTH - 1, "%s", (char*)tmp);
                DEBUG_INFO("Server changed to %s\r\n", (char*)m_ctx->status.new_server);
            }
            else
            {
                m_ctx->status.try_new_server = 0;
        //				DEBUG_ERROR("Server changed : No memory\r\n");
            }
        }
        else
        {
//            DEBUG_INFO("New server is the same with old server\r\n");
        }
    }
    else
    {
        DEBUG_ERROR("Invalid server\r\n");
    }
}

static uint8_t process_output_config(char *buffer)
{
	uint8_t new_cfg = 0;
    char *output4_20mA = strstr(buffer, "Output4_20\":");
    if (output4_20mA != NULL)
    {
        char output_value[16];
        memset(output_value, 0, sizeof(output_value));
        
        gsm_utilities_copy_parameters(output4_20mA+strlen("Output4_20\""), output_value, ':', ',');
        DEBUG_VERBOSE("Output 4-20ma %s\r\n", output_value);
        float new_value = atof(output_value);
//        uint32_t interger = (uint32_t)new_value;
//        uint32_t dec = (uint32_t)((new_value-interger)/10);
//        uint8_t out_4_20 = gsm_utilities_get_number_from_string(strlen("Output4_20\":"), output4_20mA);
        
        if (m_eeprom_config->output_4_20ma != new_value)
        {
            m_eeprom_config->io_enable.name.output_4_20ma_enable = 1;
            m_eeprom_config->output_4_20ma = new_value;
            new_cfg++;
            DEBUG_INFO("Output 4-20ma changed to %.2f\r\n", m_eeprom_config->output_4_20ma);
        }
        
        if (m_eeprom_config->output_4_20ma < 4.0f
            || m_eeprom_config->output_4_20ma > 20.0f)
        {
            m_eeprom_config->io_enable.name.output_4_20ma_enable = 0;
        }
    }

	return new_cfg;
}

uint8_t process_input_config(char *buffer)
{
	uint8_t new_cfg = 0;
#ifdef DTG01
    char *mode_j1 = strstr(buffer, "InputMode0\":");		// mode
    if (mode_j1 != NULL)
    {
        uint8_t mode = gsm_utilities_get_number_from_string(strlen("InputMode0\":"), mode_j1);
        if (m_eeprom_config->meter_mode[0] != mode)
        {
            DEBUG_INFO("PWM1 mode changed\r\n");
            m_eeprom_config->meter_mode[0] = mode;
            new_cfg++;
        }
    }
    
    char *output2 = strstr(buffer, "\"Output2\":");
    if (output2 != NULL)
    {
        char output_value[16];
        memset(output_value, 0, sizeof(output_value));
        
        gsm_utilities_copy_parameters(output2+strlen("Output2\""), output_value, ':', ',');
        DEBUG_INFO("Output 4-20ma %s\r\n", output_value);
        float new_value = atof(output_value);
        if (m_eeprom_config->output_4_20ma != new_value)
        {
            m_eeprom_config->io_enable.name.output_4_20ma_enable = 1;
            m_eeprom_config->output_4_20ma = new_value;
            new_cfg++;
            DEBUG_INFO("Output 4-20ma changed to %.2f\r\n", m_eeprom_config->output_4_20ma);
        }
        
        if (m_eeprom_config->output_4_20ma < 4.0f
            || m_eeprom_config->output_4_20ma > 20.0f)
        {
            m_eeprom_config->io_enable.name.output_4_20ma_enable = 0;
        }
    }
	
	// 4-20mA
	char *input_4_20mA = strstr(buffer, "Input2\":");		// mode
    if (input_4_20mA != NULL)
    {
        uint8_t enable = gsm_utilities_get_number_from_string(strlen("Input2\":"), input_4_20mA);
        if (m_eeprom_config->io_enable.name.input_4_20ma_0_enable != enable)
        {
            DEBUG_INFO("Input4_20mA 1 changed to %u\r\n", enable);
            m_eeprom_config->io_enable.name.input_4_20ma_0_enable = enable;
            new_cfg++;
        }
    }
		// Dir active level
	char *dir = strstr(buffer, "Dir\":");
    if (dir != NULL)
    {
        uint8_t dir_level = gsm_utilities_get_number_from_string(strlen("Dir\":"), dir);
        if (m_eeprom_config->dir_level != dir_level)
        {
            DEBUG_INFO("dir_level mode changed\r\n");
            m_eeprom_config->dir_level = dir_level;
            new_cfg++;
        }
    }
	
#else
    char *mode_j1 = strstr(buffer, "Input_J1\":");		// mode
    if (mode_j1 != NULL)
    {
        uint8_t mode = gsm_utilities_get_number_from_string(strlen("Input_J1\":"), mode_j1);
        if (m_eeprom_config->cyber_config[0].mode != mode)
        {
            DEBUG_INFO("PWM1 mode changed\r\n");
            m_eeprom_config->cyber_config[0].mode = mode;
            new_cfg++;
        }
    }
    
    char *mode_j2 = strstr(buffer, "Input_J2\":");
    if (mode_j2 != NULL)
    {
        uint8_t mode = gsm_utilities_get_number_from_string(strlen("Input_J2\":"), mode_j2);
        if (m_eeprom_config->cyber_config[1].mode != mode)
        {
            DEBUG_INFO("PWM2 mode changed\r\n");
            m_eeprom_config->cyber_config[1].mode = mode;
            new_cfg++;
        }
    }
		
	
	// Input 4-20mA
    char *input_4_20ma = strstr(buffer, "Input_J3_1\":");
    if (input_4_20ma != NULL)
    {
        uint8_t enable = gsm_utilities_get_number_from_string(strlen("Input_J3_1\":"), input_4_20ma) ? 1 : 0;
        if (m_eeprom_config->io_enable.name.input_4_20ma_0_enable != enable)
        {
            DEBUG_INFO("Input4_20mA 1 changed to %u\r\n", enable);
            m_eeprom_config->io_enable.name.input_4_20ma_0_enable = enable;
            new_cfg++;
        }
    }
	input_4_20ma = strstr(buffer, "Input_J3_2\":");
    if (input_4_20ma != NULL)
    {
        uint8_t enable = gsm_utilities_get_number_from_string(strlen("Input_J3_2\":"), input_4_20ma) ? 1 : 0;
        if (m_eeprom_config->io_enable.name.input_4_20ma_1_enable != enable)
        {
            DEBUG_INFO("Input4_20mA 2 changed to %u\r\n", enable);
            m_eeprom_config->io_enable.name.input_4_20ma_1_enable = enable;
            new_cfg++;
        }
    }
	
    input_4_20ma = strstr(buffer, "Input_J3_3\":");
    if (input_4_20ma != NULL)
    {
        uint8_t enable = gsm_utilities_get_number_from_string(strlen("Input_J3_3\":"), input_4_20ma) ? 1 : 0;
        if (m_eeprom_config->io_enable.name.input_pressure_sensor_0 != enable)
        {
            DEBUG_INFO("Pressure0 4-20mA - changed to %u\r\n", enable);
            m_eeprom_config->io_enable.name.input_pressure_sensor_0 = enable;
            new_cfg++;
        }
    }

    input_4_20ma = strstr(buffer, "Input_J3_4\":");
    if (input_4_20ma != NULL)
    {
        uint8_t enable = gsm_utilities_get_number_from_string(strlen("Input_J3_4\":"), input_4_20ma) ? 1 : 0;
        if (m_eeprom_config->io_enable.name.input_pressure_sensor_1 != enable)
        {
            DEBUG_INFO("Pressure1 4-20mA changed to %u\r\n", enable);
            m_eeprom_config->io_enable.name.input_pressure_sensor_1 = enable;
            new_cfg++;
        }
    }
	
#endif // DTG01
    return new_cfg;
}

static uint8_t process_flow_speed_enable(char *buffer)
{
   	uint8_t new_cfg = 0;
    char *cal = strstr(buffer, "\"FlowCal\":");
    if (cal)
    {
        cal += strlen("\"FlowCal\":");
        uint32_t enable = gsm_utilities_get_number_from_string(strlen("\"FlowCal\":"), cal) ? 1 : 0;
        if (enable != m_eeprom_config->io_enable.name.calculate_flow_speed)
        {
            m_eeprom_config->io_enable.name.calculate_flow_speed = enable;
            DEBUG_INFO("Flow cal %u\r\n", enable);
            new_cfg = 1;
        }
    }
    return new_cfg;
}

static uint8_t process_meter_indicator(char *buffer, uint8_t *factor_change)
{
	uint8_t new_cfg = 0;
    
    // Cyber offset 0
    char *counter_offset = strstr(buffer, "MeterIndicator_J1\":");
    if (counter_offset)
    {
        uint32_t offset = gsm_utilities_get_number_from_string(strlen("MeterIndicator_J1\":"), counter_offset);
        if (m_eeprom_config->cyber_config[0].offset != offset)
        {
            new_cfg++;   
            m_eeprom_config->cyber_config[0].offset = offset;
            DEBUG_WARN("PWM1 offset changed to %u\r\n", offset);
            measure_input_reset_counter(0);
            measure_input_reset_indicator(0, offset);
            measure_input_cyber_counter_t counter[MEASURE_NUMBER_OF_WATER_METER_INPUT];
            app_bkup_read_pulse_counter(&counter[0]);
            counter[0].real_counter = 0;
            counter[0].reverse_counter = 0;
            app_bkup_write_pulse_counter(&counter[0]);
            (*factor_change) |= (1 << 0);
        }
    }
    
    // Cyber offset 1
	counter_offset = strstr(buffer, "MeterIndicator_J2\":");
    if (counter_offset)
    {
        uint32_t offset = gsm_utilities_get_number_from_string(strlen("MeterIndicator_J2\":"), counter_offset);
        if (m_eeprom_config->cyber_config[1].offset != offset)
        {
            new_cfg++;   
            m_eeprom_config->cyber_config[1].offset = offset;
            DEBUG_INFO("PWM2 offset changed to %u\r\n", offset);
            measure_input_reset_counter(1);
            measure_input_reset_indicator(1, offset);
            measure_input_cyber_counter_t counter[MEASURE_NUMBER_OF_WATER_METER_INPUT];
            counter[1].real_counter = 0;
            counter[1].reverse_counter = 0;
            app_bkup_write_pulse_counter(&counter[0]);
            (*factor_change) |= (1 << 1);
        }
    }
    
    // Cyber K0 factor
    char *k_factor = strstr(buffer, "K_J1\":");
    if (k_factor)
    {
        uint32_t k = gsm_utilities_get_number_from_string(strlen("K_J1\":"), k_factor);
        if (k == 0)
        {
            k = 1;
        }

        if (m_eeprom_config->cyber_config[0].k != k)
        {
            m_eeprom_config->cyber_config[0].k = k;
            measure_input_reset_k(0, k);
            DEBUG_INFO("K1 factor changed to %u\r\n", k);
            new_cfg++; 
        }
    }
    
    // Cyber K1 factor
    k_factor = strstr(buffer, "K_J2\":");
	if (k_factor)
    {
        uint32_t k = gsm_utilities_get_number_from_string(strlen("K_J2\":"), k_factor);
        if (k == 0)
        {
            k = 1;
        }

        if (m_eeprom_config->cyber_config[1].k != k)
        {
            m_eeprom_config->cyber_config[1].k = k;
            measure_input_reset_k(1, k);
            DEBUG_INFO("K2 factor changed to %u\r\n", k);
            new_cfg++; 
        }
    }
	return new_cfg;
}

static void process_ota_update(char *buffer)
{
    char tmp_hw[16];
    char *do_ota = NULL;
    char *version = NULL;
    char *link = NULL;
    char *hardware = strstr(buffer, "\"Hardware\":");  
    if (!hardware)
    {
        return;
    }
        
    memset(tmp_hw, 0, sizeof(tmp_hw));
    hardware += strlen("\"Hardware\":");  
    if (gsm_utilities_copy_parameters(hardware, tmp_hw, '"', '"'))
    {
        // Only update if valid hardware
        if (strcmp(tmp_hw, VERSION_CONTROL_HW))
        {
            return;
        }
    }
    
    do_ota = strstr(buffer, "\"Update\":1");
    version = strstr(buffer, "\"Version\":\"");
    link = strstr(buffer, "\"Link\":\"http");
    
    // Check if valid OTA info
    if (do_ota 
        && version 
        && link)
    {
        DEBUG_INFO("2");
        version += strlen("\"Version\":\"");
        link += strlen("\"Link\":");
        uint8_t version_compare;
        version = strtok(version, "\"");
        version_compare = version_control_compare(version);
        if (version_compare == VERSION_CONTROL_FW_NEWER)
        {
            link = strtok(link, "\"");
            // Valid link
            if (link && strlen(link) && strstr(link, "http"))       // http:// or https://
            {
                m_ctx->status.delay_ota_update = 5;
                m_ctx->status.enter_ota_update = true;
                sprintf((char*)m_ctx->status.ota_url, "%s", strstr(link, "http"));
            }
        }
        else
        {
            DEBUG_INFO("Version %d\r\n", version_compare);
        }
    }
}


static uint8_t process_auto_get_config_interval(char *buffer)
{
    uint8_t has_new_cfg = 0;
    char *auto_config_interval = strstr(buffer, "Auto_config\":");  
    if (auto_config_interval)
    {
        auto_config_interval += strlen("Auto_config\":");  
        uint32_t tmp = gsm_utilities_get_number_from_string(0, auto_config_interval);
        if (tmp && tmp != m_eeprom_config->poll_config_interval_hour)
        {
            m_eeprom_config->poll_config_interval_hour = tmp;		// 24hour
            has_new_cfg++;
        }
    }
    return has_new_cfg;
}


static void process_debug_log_timeout(char *buffer)
{
    char *console = strstr(buffer, "console\":");  
    if (console)
    {
        console += strlen("console\":");  
        uint32_t tmp = gsm_utilities_get_number_from_string(0, console);
        if (tmp && tmp != m_eeprom_config->debug_timeout_second)
        {
            m_eeprom_config->debug_timeout_second = tmp;		// 24hour
        }
    }
}

static uint8_t process_low_bat_config(char *buffer)
{
    uint8_t has_new_cfg = 0;
    char *auto_config_interval = strstr(buffer, "BatLevel\":");  
    if (auto_config_interval)
    {
        auto_config_interval += strlen("BatLevel\":");  
        uint32_t tmp = gsm_utilities_get_number_from_string(0, auto_config_interval);
        if (tmp && tmp != m_eeprom_config->battery_low_percent)
        {
            m_eeprom_config->battery_low_percent = tmp;		// 24hour
            has_new_cfg++;
        }
    }
    return has_new_cfg;
}

static uint8_t process_max_sms_one_day_config(char *buffer)
{
    uint8_t has_new_cfg = 0;
    char *auto_config_interval = strstr(buffer, "MaxSms1Day\":");  
    if (auto_config_interval)
    {
        auto_config_interval += strlen("MaxSms1Day\":");  
        uint32_t tmp = gsm_utilities_get_number_from_string(0, auto_config_interval);
        if (tmp != m_eeprom_config->max_sms_1_day)
        {
            m_eeprom_config->max_sms_1_day = tmp;		// 24hour
            has_new_cfg++;
        }
    }
    return has_new_cfg;
}

static uint8_t process_sync_clock_duration(char *buffer)
{
    uint8_t has_new_cfg = 0;
    char *rtc_sync_interval = strstr(buffer, "timeSync\":");  
    if (rtc_sync_interval)
    {
        rtc_sync_interval += strlen("timeSync\":");  
        uint32_t tmp = gsm_utilities_get_number_from_string(0, rtc_sync_interval);
        if (tmp != m_eeprom_config->sync_clock_in_hour
            && tmp)
        {
            m_eeprom_config->sync_clock_in_hour = tmp;		// 24hour
            has_new_cfg++;
        }
    }
    return has_new_cfg;
}


static uint8_t process_modbus_register_config(char *buffer)
{
    // Process RS485
    // Total 2 device
    // "ID485_0":1,
    // Register_1_1:30001
    // Unit_1_1:"kg/m3",
    // Type_1_1:"int16/int32/float",
    // Register_1_3:30003,
    // 
    //"ID485_1":1,
    // Register_2_1:40001
    //  Register_2_3:40003
    uint8_t new_cfg = 0;
    for (int slave_count = 0; 
        slave_count < HARDWARE_RS485_MAX_SLAVE_ON_BUS && m_eeprom_config->io_enable.name.rs485_en; 
        slave_count++)
    {
        char search_str[32];
        sprintf(search_str, "ID485_%u\":", slave_count+1);
        char *rs485_id_str = strstr(buffer, search_str);
        
        for (uint32_t sub_reg_idx = 0; sub_reg_idx < HARDWARE_RS485_MAX_SUB_REGISTER; sub_reg_idx++)
        {			
            sprintf(search_str, "Register_%u_%u\":", slave_count+1, sub_reg_idx+1);
            char *rs485_reg_str = strstr(buffer, search_str);
            
            sprintf(search_str, "\"Type_%u_%u\":", slave_count+1, sub_reg_idx+1);
            char *rs485_type_str = strstr(buffer, search_str);
            if (rs485_type_str)
            {
                rs485_type_str += strlen(search_str);
            }
            
            sprintf(search_str, "\"Unit_%u_%u\":", slave_count+1, sub_reg_idx+1);
            char *rs485_unit = strstr(buffer, search_str);
            
            if (rs485_id_str && rs485_reg_str && rs485_type_str)
            {
                uint32_t temp;
                // Get rs485 slave id
                temp = gsm_utilities_get_number_from_string(9, rs485_id_str);  //7 = strlen(ID485_1":)
                if (temp != m_eeprom_config->rs485[slave_count].slave_addr)
                {
                    m_eeprom_config->rs485[slave_count].slave_addr = temp;
                    new_cfg++;
                }
                DEBUG_INFO("Slave addr %u\r\n", temp);
                // Get RS485 data type
                temp = MEASURE_INPUT_RS485_DATA_TYPE_INT16;
                m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].data_type.name.valid = 1;
                if (memcmp(rs485_type_str, "\"int32\"", strlen("\"int32\"")) == 0)
                {
                    temp = MEASURE_INPUT_RS485_DATA_TYPE_INT32;
                }
                if (memcmp(rs485_type_str, "\"float\"", strlen("\"float\"")) == 0)
                {
                    temp = MEASURE_INPUT_RS485_DATA_TYPE_FLOAT;
                }
                
                if (temp != m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].data_type.name.type)
                {
                    m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].data_type.name.type = temp;
                    new_cfg++;
                    DEBUG_INFO("Type %u\r\n", temp);
                }
                
                // Get RS485 data addr 
                temp = gsm_utilities_get_number_from_string(14, rs485_reg_str);  //14 = strlen(Register_2_1":)
                if (temp != m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].reg_addr)
                {
                    m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].reg_addr = temp;
                    new_cfg++;
                    DEBUG_INFO("Reg addr %u\r\n", temp);
                }
                
                // Unit
                if (rs485_unit == NULL)
                {
                    m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].unit[0] = 0;
                }
                else
                {
                    // strlen("\"Unit_%u_%u\":\"") = 11
                    rs485_unit += 11;
                    uint32_t copy_len = HARDWARE_RS485_MAX_UNIT_NAME_LENGTH - 1;
                    char *p = strstr(rs485_unit, "\"");
                    if (p - rs485_unit < copy_len)
                    {
                        copy_len = p - rs485_unit;
                    }
                    if (strstr(rs485_unit, 
                            (char*)m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].unit) == 0)
                    {
                        new_cfg++;
                    }
                    strncpy((char*)m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].unit, 
                            (char*)rs485_unit, 
                            copy_len); 
    //					DEBUG_INFO("Unit %s\r\n", (char*)m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].unit);
                }
            }
            else
            {
                m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].data_type.name.valid = 0;
                m_eeprom_config->rs485[slave_count].sub_reg[sub_reg_idx].read_ok = 0;
            }
            
            // process forward-reserve flow index
            sprintf(search_str, "ForwardFlow_%u\":", slave_count+1);
            char *p_fw_flow = strstr(buffer, search_str);
            if (p_fw_flow)
            {
                p_fw_flow += strlen(search_str);
                uint32_t fw_flow_reg = gsm_utilities_get_number_from_string(0, p_fw_flow);
                if (fw_flow_reg && fw_flow_reg != m_eeprom_config->rs485[slave_count].fw_flow_reg)
                {
                    new_cfg++;
                    DEBUG_INFO("Forward flow changed to %u\r\n", fw_flow_reg);
                    m_eeprom_config->rs485[slave_count].fw_flow_reg = fw_flow_reg;
                }
            }
            
            // Net flow register
            sprintf(search_str, "NetFl_%u\":", slave_count+1);
            char *p_net_flow_reg = strstr(buffer, search_str);
            if (p_net_flow_reg)
            {
                p_net_flow_reg += strlen(search_str);
                uint32_t net_totalizer_reg = gsm_utilities_get_number_from_string(0, p_net_flow_reg);
                if (net_totalizer_reg 
                    && net_totalizer_reg != m_eeprom_config->rs485[slave_count].net_totalizer_reg)
                {
                    new_cfg++;
                    DEBUG_INFO("Forward flow changed to %u\r\n", net_totalizer_reg);
                    m_eeprom_config->rs485[slave_count].net_totalizer_reg = net_totalizer_reg;
                }
            }
            
            // Net forward flow register
            sprintf(search_str, "NetFwFl_%d\":", slave_count+1);
            char *p_net_fw_flow_reg = strstr(buffer, search_str);
            if (p_net_fw_flow_reg)
            {
                p_net_fw_flow_reg += strlen(search_str);
                uint32_t net_totalizer_fw_reg = gsm_utilities_get_number_from_string(0, p_net_fw_flow_reg);
                if (net_totalizer_fw_reg && 
                    net_totalizer_fw_reg != m_eeprom_config->rs485[slave_count].net_totalizer_fw_reg)
                {
                    new_cfg++;
                    DEBUG_INFO("Net forward flow changed to %u\r\n", net_totalizer_fw_reg);
                    m_eeprom_config->rs485[slave_count].net_totalizer_fw_reg = net_totalizer_fw_reg;
                }
            }
            
            // Net reverse flow register
            sprintf(search_str, "NetRvsFl_%u\":", slave_count+1);
            char *p_net_rvs_flow_reg = strstr(buffer, search_str);
            if (p_net_rvs_flow_reg)
            {
                p_net_rvs_flow_reg += strlen(search_str);
                uint32_t net_totalizer_reverse_reg = gsm_utilities_get_number_from_string(0, p_net_rvs_flow_reg);
                if (net_totalizer_reverse_reg 
                    && net_totalizer_reverse_reg != m_eeprom_config->rs485[slave_count].net_totalizer_reverse_reg)
                {
                    new_cfg++;
                    DEBUG_INFO("Net reverse flow changed to %u\r\n", net_totalizer_reverse_reg);
                    m_eeprom_config->rs485[slave_count].net_totalizer_reverse_reg = net_totalizer_reverse_reg;
                }
            }
            
            // Modbus method
            sprintf(search_str, "MbMethod_%u\":", slave_count+1);   // TODO handle modbus flow calculate method for multi device
                                                                    // Due to my laziness, i dont handle this
            char *p_mb_method = strstr(buffer, search_str);
            
            if (p_mb_method)
            {
                p_mb_method += strlen(search_str);
                uint32_t fw_flow_method = gsm_utilities_get_number_from_string(0, p_mb_method);
                if (fw_flow_method != m_eeprom_config->io_enable.name.modbus_cal_method)
                {
                    new_cfg++;
    //                    DEBUG_INFO("Forward flow method to %u\r\n", fw_flow_method);
                    m_eeprom_config->io_enable.name.modbus_cal_method = fw_flow_method ? 1 : 0;
                }
            }
            
            sprintf(search_str, "ReverseFlow_%u\":", slave_count+1);
            char *p_reverse_flow = strstr(buffer, search_str);
            if (p_reverse_flow)
            {
                p_reverse_flow += strlen(search_str);
                uint32_t reverse_flow_reg = gsm_utilities_get_number_from_string(0, p_reverse_flow);
                if (reverse_flow_reg &&reverse_flow_reg != m_eeprom_config->rs485[slave_count].reverse_flow_reg)
                {
                    new_cfg++;
                    m_eeprom_config->rs485[slave_count].reverse_flow_reg = reverse_flow_reg;
                    DEBUG_INFO("Reverse flow changed to %u\r\n", reverse_flow_reg);
                }
            }
        }
    }

    // Get RS485 baudrate
    char *baud_str = strstr(buffer, "Baud\":");
    if (baud_str)
    {
        baud_str += strlen("Baud\":");
        uint32_t baudrate = gsm_utilities_get_number_from_string(0, baud_str);
        app_eeprom_factory_data_t *factory = app_eeprom_read_factory_data();
        if (m_eeprom_config->rs485_baudrate != baudrate)
        {
            m_eeprom_config->rs485_baudrate = baudrate;
            new_cfg++;
        }
    }
    
    // Modbus byte order : little endian or big endian
    char *bytes_order = strstr(buffer, "\"Bytes\":");
    if (bytes_order)
    {
        bytes_order += strlen("\"Bytes\":");
        uint32_t order = gsm_utilities_get_number_from_string(0, bytes_order) ? 1 : 0;
        app_eeprom_factory_data_t *factory = app_eeprom_read_factory_data();
        if (m_eeprom_config->io_enable.name.rs485_byte_order != order)
        {
            m_eeprom_config->io_enable.name.rs485_byte_order = order;
            new_cfg++;
        }
    }
    
    return new_cfg;
}

static uint8_t process_pulse_config(char *buffer)
{
    uint8_t new_cfg = 0;
    char *pulse_cfg = strstr(buffer, "\"pulse\":");
    if (pulse_cfg)
    {
        pulse_cfg += strlen("\"pulse\":");
        uint32_t pulse_ms = gsm_utilities_get_number_from_string(0, pulse_cfg);
        if (pulse_ms && m_eeprom_config->cyber_pulse_ms != pulse_ms)
        {
            m_eeprom_config->cyber_pulse_ms = pulse_ms;
            new_cfg++;
        }
    } 
    return new_cfg;
}

// https://viblo.asia/p/tong-hop-thuat-toan-sort-co-ban-vi-du-trong-ngon-ngu-c-Qbq5QqpE5D8
void bubble_sort(int16_t* N, uint8_t len)
{
    int16_t i,j,temp;

    for (i=0; i<len; i++)
    {
        for (j = len-1; j > i; j--)
        {
            if (N[j] < N[j-1])
            {
                temp = N[j];
                N[j] = N[j-1];
                N[j-1] = temp;
            }
        }
    }
}

/**
* "fixedTime":"12:34,12:35,12:36,12:37"
* "fixedTime":"12:34,12:35,12:36,,"
 */
static uint8_t process_message_fixed_cycle_send_web(char *buffer)
{
    // maximum 4 setting
    int16_t config_time[APP_EEPROM_MAX_FIXED_SEND_WEB_TIME];
    
    // Set to invalid value
    memset(config_time, APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME, sizeof(config_time));
    
    uint8_t new_config_count = 0;
    char tmp[48];
    
    // Found valid json keyword
    char *set_time = strstr(buffer, "\"fixedTime\":\"");
    if (!set_time)
    {
        return 0;
    }
    set_time += strlen("\"fixedTime\":\"") - 1;
    memset(tmp, 0, sizeof(tmp));

    if (gsm_utilities_copy_parameters(set_time, tmp, '"', '"'))
    {
        char list_time_t[APP_EEPROM_MAX_FIXED_SEND_WEB_TIME][8];
        memset(list_time_t, 0, sizeof(list_time_t));
        uint8_t found = 0;
        
        char *token = strtok(tmp, ",");
        while (token != NULL)
        {
            snprintf(list_time_t[found++], 8, "%s", token);
            token = strtok(NULL, ",");

            if (found >= APP_EEPROM_MAX_FIXED_SEND_WEB_TIME)
                break;
        }
        
        DEBUG_VERBOSE("Found %u setting\r\n", found);
        for (uint32_t i = 0; 
                        (i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME) 
                        && (i < found); 
                        i++)
        {
            int hour, min;
            sscanf(list_time_t[i], "%d:%d", &hour, &min);
            int convert_to_min = hour*60 + min;
            if (convert_to_min < 1440)      // 1 day = 1440 min
            {
                config_time[i] = hour*60 + min;
                DEBUG_VERBOSE("Min : %d:%d = %d\r\n", hour, min, config_time[i]);
            }
        }     
        
        // Sort tu be den lon
        bubble_sort(config_time, APP_EEPROM_MAX_FIXED_SEND_WEB_TIME);
        
        int16_t *current_setting = app_eeprom_get_fixed_time_wakeup_setting();
       
        if (memcmp(current_setting, config_time, sizeof(config_time)))
        {
            DEBUG_INFO("Store new setting\r\n");
            app_eeprom_set_fixed_time_wakeup_setting(config_time);
            new_config_count++;
        }
    }
    return new_config_count;
}



#if(1)
static timestamp timedata;
timestamp m_last_timedata;

uint8_t server_msg_process_timestamp_cmd (char *buffer)
{
    char *time_str = strstr (buffer, "\"DateTime\":");
    if (time_str == NULL)
    {
        return 0;
    }
    else
    {
        unsigned char count = 0;
        char buffer_temp[128];
        char buff_tim[24];
        strncpy ((char *) buffer_temp, time_str, 127);
        time_str = strstr (buffer_temp, "\"DateTime\":");

        
        char *buffer_date;
        char *buffer_time;

        strncpy (buff_tim, time_str, 23);
        time_str += (strlen ("\"DateTime\":") + 1);
        char *terminate = strstr(time_str+1, "\"");
        if (terminate)
        {
            *terminate = 0;
        }
        
        buffer_time = strstr ((char *) time_str, ",");
        buffer_date = strtok (time_str, "/");

        if (buffer_date != NULL)
        {
            if (strcmp (buff_tim, buffer_date) == 0)
            {
                //goto exit;
            }
            timedata.value[count] = atoi (buffer_date);
            count++;
        }
        else
        {
            DEBUG_RAW ("Wrong format\r\n");
            return 0;
        }

        while (buffer_date != NULL && count < 3)
        {
            buffer_date = strtok (NULL, "/");
            if (buffer_date != NULL)
            {
                if (strcmp (time_str, buffer_date) == 0)
            {
                DEBUG_WARN ("Wrong format find \r\n");
                return 0;
            }
                timedata.value[count] = atoi (buffer_date);
                DEBUG_WARN ("Timedata[%u]: %d\r\n", count,
                            timedata.value[count]);
                count++;
            }
        }
        buffer_time++;
        buffer_time = strtok (buffer_time, ":");
        if (buffer_time != NULL)
        {
            char *tmp = strstr ((char *) time_str, ",");
            if (tmp && strcmp (tmp, buffer_time) == 0)
            {
                DEBUG_WARN ("Wrong format\r\n");
                return 0;
            }
            timedata.value[count] = atoi (buffer_time);
            DEBUG_WARN ("Hour %d\r\n", timedata.value[count]);
            count++;
        }
        while (buffer_time != NULL && count < 6)
        {
            buffer_time = strtok (NULL, ":");
            if (buffer_time != NULL)
            {
                timedata.value[count] = atoi (buffer_time);
                count++;
            }
        }
        // exit:
        if (count < 5)
        {
            DEBUG_ERROR ("Config time from server wrong format\r\n");
        }
        else
        {			
            // receive hour and second wrong => vice versa hour and sec :v
            /*uint8_t temp;
            temp = timedata.time.second;
            timedata.time.second = timedata.time.hour;
            timedata.time.hour = temp; */
            //            DEBUG_RAW("Year:%d-Month:%d-Day:%d-Hour:%d-Minute:%d-Second:%d\r\n",
            //                timedata.time.year, timedata.time.month, timedata.time.day,
            //                timedata.time.hour, timedata.time.minute, timedata.time.second);
            if ((timedata.time.year < 100 && timedata.time.year >= 20)
                && timedata.time.second <= 59 && timedata.time.minute <= 59
                && timedata.time.hour <= 23 && (timedata.time.month >= 1
                                && timedata.time.month <= 12)
                && (timedata.time.day >= 1 && timedata.time.day <= 30))
            {
                if (memcmp(&m_last_timedata, &timedata, sizeof (m_last_timedata)))
                {
                    DEBUG_INFO("Set new counter\r\n");
                    memcpy(&m_last_timedata, &timedata, sizeof (m_last_timedata));
                    app_rtc_set_counter(&timedata.time);
                    return 0;
                }
            }
            else
            {
                DEBUG_ERROR ("Config time is not legal\r\n");
            }
        }
    }
    return 0;
}
#else
void server_msg_process_timestamp_cmd(char *buffer)
{
    timestamp timedata;
    unsigned char count = 0;
	char buffer_temp[128];
    char buff_tim[24];
    char buff_day[24];
    strcpy((char*)buffer_temp,buffer);
    char *date_ptr;
    char *time_ptr;
    DEBUG_INFO("buffer temp:%s",buffer_temp);
    char *time_str = strstr(buffer_temp, "\"DateTime\":");
    if(time_str == NULL)
    {
        DEBUG_INFO("Canot not find key DateTime\r\n");
        return;
	}
    else
    {
        //strcpy(buff_tim, time_str);
		time_str += (strlen("\"DateTime\":") + 1);
        time_ptr = strstr((char*)time_str, ",");
        strcpy(buff_day, time_str);
        strcpy(buff_tim, time_ptr);
        DEBUG_INFO("time_str:%s\r\n", time_str);
	    DEBUG_INFO("Get time config from server\r\n");
	    date_ptr = strtok(buff_day,"/");
        DEBUG_INFO("date_ptr:%s\r\n", date_ptr);
        DEBUG_INFO("time_ptr:%s\r\n", time_ptr);
        DEBUG_INFO("buff_day :%s\r\n", buff_day);
	    if(date_ptr != NULL)
	    {
           if(strcmp(buff_tim, date_ptr) == 0)
           {
               DEBUG_INFO("Wrong format find / \r\n");
               //goto exit;
           }
	       timedata.value[count] = atoi(date_ptr);
	       count++;
	    }
	    else
	    {
	       DEBUG_INFO("Wrong format\r\n");
	       return;
	    }
	    while(date_ptr != NULL)
	    {
	       date_ptr = strtok(NULL, "/");
	       if(date_ptr != NULL)
	       {
               if(strcmp(time_str, date_ptr) == 0)
               {
                   DEBUG_INFO("Wrong format find \r\n");
                   //goto exit;
               }
	            timedata.value[count] = atoi(date_ptr);
	            count++;
	       }
	    }
	    time_ptr++;
	    time_ptr = strtok(time_ptr, ":");
	    if( time_ptr != NULL)
	    {
            if(strcmp(strstr((char*)time_str, ","), time_ptr) == 0)
            {
                DEBUG_INFO("Wrong format\r\n");
                //goto exit;
             }
            timedata.value[count] = atoi(time_ptr);
            count++;
	    }
	    while(time_ptr != NULL)
	    {
	       time_ptr = strtok(NULL, ":");
	       if(time_ptr != NULL)
	       {
	            timedata.value[count] = atoi(time_ptr);
	            count++;
	       }
	    }
exit:
	    if( count < 5)
	    {
                DEBUG_INFO("Config time from server wrong format\r\n");
	    }
		else
            {       // receive hour and second wrong => vice versa hour and sec :v
                /*unsigned char temp;
                temp = timedata.time.second;
                timedata.time.second = timedata.time.hour;
                timedata.time.hour = temp;*/
				DEBUG_INFO("Check the time config\r\n");
				DEBUG_INFO("Year:%d-Month:%d-Day:%d-Hour:%d-Minute:%d-Second:%d\r\n",
				timedata.time.year, timedata.time.month, timedata.time.day,
			    timedata.time.hour, timedata.time.minute, timedata.time.second);
			    if(( timedata.time.year < 100 && timedata.time.year >= 20) &&
					timedata.time.second <= 59 &&
				    timedata.time.minute <= 59&&
				    timedata.time.hour <= 23 &&
					(timedata.time.month >= 1 && timedata.time.month <= 12)&&
					(timedata.time.day >= 1 && timedata.time.day <= 30))
					{
							    uint32_t new_counter = rtc_struct_to_counter(&timedata.time);
							    new_counter += (946681200 + 3600);
								uint32_t current_counter = app_rtc_get_counter();
								static uint32_t m_last_time_update = 0;
								DEBUG_INFO("New counter %u, current counter %u\r\n", new_counter, current_counter);
								if (m_last_time_update == 0 
                                    || (current_counter - m_last_time_update >= (uint32_t)(3600 * 12)) 
                                    || ((new_counter >= current_counter) && (new_counter - current_counter >= (uint32_t)60)) 
                                    || ((new_counter < current_counter) && (current_counter - new_counter >= (uint32_t)60)))
								{
										DEBUG_INFO("Update time\r\n");
										m_last_time_update = new_counter;
									  //DEBUG_INFO("");
										app_rtc_set_counter(&timedata.time);
								}
                                else
                                {
                                     DEBUG_INFO("The config time is lower than the current time.Please change timeconfig\r\n");
                                }
                                DEBUG_INFO("Update time\r\n");
						}
                        else
                        {
                            DEBUG_INFO("Config time is not legal\r\n");
                        }
		}
	}
}
#endif

void server_msg_process_cmd(char *buffer, uint8_t *new_config)
{
    uint8_t has_new_cfg = 0;
    uint8_t factor_change = 0;	

    if (m_eeprom_config == NULL)
    {
        m_eeprom_config = app_eeprom_read_config_data();
    }

    if (m_ctx == NULL)
    {
        m_ctx = sys_ctx();
    }

    m_ctx->status.disconnected_count = 0;

    char *cycle_wakeup = strstr(buffer, "\"Cyclewakeup\":");
    if (cycle_wakeup != NULL)
    {
        uint32_t wake_time_measure_data_ms = 1000*60*gsm_utilities_get_number_from_string(strlen("\"Cyclewakeup\":"), cycle_wakeup);
        
        if (m_eeprom_config->measure_interval_ms != wake_time_measure_data_ms 
            && wake_time_measure_data_ms)
        {
            m_eeprom_config->measure_interval_ms = wake_time_measure_data_ms;
            has_new_cfg++;
        }
    }

    char *cycle_send_web = strstr(buffer, "\"CycleSendWeb\":");
    if (cycle_send_web != NULL)
    {
        uint32_t send_time_ms = 1000*60*gsm_utilities_get_number_from_string(strlen("\"CycleSendWeb\":"), cycle_send_web);
        if (m_eeprom_config->send_to_server_interval_ms != send_time_ms && send_time_ms)
        {
            DEBUG_INFO("CycleSendWeb changed\r\n");
            m_eeprom_config->send_to_server_interval_ms = send_time_ms;
            has_new_cfg++;
        }
    }

    // Process output config
    has_new_cfg += process_output_config(buffer);

    // Process input
    has_new_cfg += process_input_config(buffer);

    // Process RS485
    char *rs485_ptr = strstr(buffer, "\"RS485\":");
    if (rs485_ptr != NULL)
    {
        uint8_t in485 = gsm_utilities_get_number_from_string(strlen("\"RS485\":"), rs485_ptr) ? 1 : 0;
        if (m_eeprom_config->io_enable.name.rs485_en != in485)
        {
            DEBUG_INFO("in485 changed\r\n");
            m_eeprom_config->io_enable.name.rs485_en = in485;
            has_new_cfg++;
        }
    }

    char *alarm = strstr(buffer, "\"Warning\":");
    if (alarm != NULL)
    {
        uint8_t alrm = gsm_utilities_get_number_from_string(strlen("\"Warning\":"), alarm) ? 1 : 0;
        if (m_eeprom_config->io_enable.name.warning != alrm)
        {
            DEBUG_INFO("Warning changed\r\n");
            m_eeprom_config->io_enable.name.warning = alrm;
            has_new_cfg++;
        }
    }

    // SOS phone number
    char *phone_num = strstr(buffer, "\"SOS\":");
    if (phone_num != NULL)
    {
        phone_num += strlen("\"SOS\":");
        char tmp_phone[30] = {0};
        if (gsm_utilities_copy_parameters(phone_num, tmp_phone, '"', '"'))
        {
    #if 1
            uint8_t changed = 0;
            int32_t len = strlen(tmp_phone);
            if (len > 15)
            {
                len = 15;
            }
            
            for(uint8_t i = 0; i < len; i++)
            {
                if(tmp_phone[i] != m_eeprom_config->phone[i]
                    && changed == 0) 
                {
                    changed = 1;
                    has_new_cfg++;
                }
                m_eeprom_config->phone[i] = tmp_phone[i];
            }
            
            m_eeprom_config->phone[15] = 0;
            if (changed)
            {
                DEBUG_INFO("Phone changed to %s\r\n", m_eeprom_config->phone);
            }
    #endif
        }
    }

    // process meter indicator
    has_new_cfg += process_meter_indicator(buffer, &factor_change);

    // Delay send message
    char *p_delay = strstr(buffer, "\"Delay\":");
    if (p_delay)
    {
        uint32_t delay = gsm_utilities_get_number_from_string(strlen("\"Delay\":"), p_delay);
        delay = delay & 0xFF;       // max 255s
        if (delay != m_eeprom_config->send_to_server_delay_s)
        {
            m_eeprom_config->send_to_server_delay_s = delay;
            DEBUG_INFO("Delay changed to %us\r\n", delay);
            has_new_cfg++;
        }
    }

    // Process flow speed mode
    has_new_cfg += process_flow_speed_enable(buffer);

    // Server addr changed
    process_server_addr_change(buffer);

    // Process time config from server
    has_new_cfg += server_msg_process_timestamp_cmd(buffer);

    // Modbus register configuration
    has_new_cfg += process_modbus_register_config(buffer);

    has_new_cfg += process_pulse_config(buffer);


    // Luu config moi
    if (has_new_cfg && sys_ctx()->status.try_new_server == 0)
    {
        DEBUG_INFO("Save eeprom config\r\n");
        app_eeprom_save_config();
    }
    else
    {
        DEBUG_VERBOSE("CFG: has no new config\r\n");
    }

    // process auto config interval setup
    has_new_cfg += process_auto_get_config_interval(buffer);

    if (factor_change)
    {
        app_flash_log_data_t last_data;
        
        // Wakeup flash
        if (!m_ctx->peripheral_running.name.flash_running)
        {
            app_spi_flash_wakeup(sys_flash());
            spi_init();
            m_ctx->peripheral_running.name.flash_running = 1;
        }
        
        bool flash_has_data = app_flash_memory_log_data_is_available((app_flash_drv_t*)sys_flash());
        
        
        if (flash_has_data)
        {
            uint8_t *mem;
            uint16_t size = 0;
            bool valid_crc = false;
            uint32_t crc;
            
            // Find lastest message
            app_flash_mem_error_t err = app_flash_log_the_get_last_message((app_flash_drv_t*)sys_flash(), &mem, &size);
            
            if (mem)
            {
                 // Calculate CRC
                crc = utilities_calculate_crc32((uint8_t *)&last_data, sizeof(app_flash_log_data_t) - HARDWARE_CRC32_SIZE);
                
                if (last_data.resend_to_server_flag != APP_FLASH_DONT_NEED_TO_SEND_TO_SERVER_FLAG
                    && last_data.crc == crc
                    && size)
                {
                    valid_crc = true;
                }

                if (err == APP_SPI_FLASH_MEM_OK 
                    && size
                    && valid_crc)
                {
                    memcpy(&last_data, mem, sizeof(app_flash_log_data_t));
                }   
            }
            
            if (factor_change & 0x01)		// 0x01 mean we need to store new data of pulse counter[0] to eeprom
            {
                last_data.counter[0].real_counter = 0;
                last_data.counter[0].reverse_counter = 0;
            }

            if (factor_change & 0x02)		// 0x02 mean we need to store new data of pulse counter[1] to eeprom
            {
                last_data.counter[1].real_counter = 0;
                last_data.counter[1].reverse_counter = 0;
            }

            last_data.timestamp = app_rtc_get_counter();
            DEBUG_INFO("Save new config to flash\r\n");
            app_flash_log_measurement_data((app_flash_drv_t*)sys_flash(), &last_data);
           
        }
        
        // Shutdown flash
        if (m_ctx->peripheral_running.name.flash_running)
        {
            app_spi_flash_shutdown(sys_flash());
            spi_deinit();
            m_ctx->peripheral_running.name.flash_running = 0;
        }
    }
    
    has_new_cfg += process_message_fixed_cycle_send_web(buffer);

    // Process low battery config
    has_new_cfg += process_low_bat_config(buffer);

    // Process max sms in 1 day
    has_new_cfg += process_max_sms_one_day_config(buffer);
    
   // Process RTC sync duration
    has_new_cfg += process_sync_clock_duration(buffer);
    
    // debug timeout
    process_debug_log_timeout(buffer);
    
    if (has_new_cfg)
    {
        app_eeprom_save_config();
    }
    
    // OTA update
    process_ota_update(buffer);

    *new_config = has_new_cfg;
}

void server_msg_process_boardcast_cmd(char *buffer)
{
    uint8_t save_config = 0;
    // Server addr changed
    process_server_addr_change(buffer);

    // OTA update
    process_ota_update(buffer);


    // Process auto config interval

    if (process_auto_get_config_interval(buffer))
    {
        save_config++;
    }


    // Process low battery config
    if (process_low_bat_config(buffer))
    {
        save_config++;
    }

    // Process max sms in 1 day
    if (process_max_sms_one_day_config(buffer))
    {
        save_config++;
    }
    if (save_config)
    {
        app_eeprom_save_config();
    }
}
