#include "gsm_application.h"
#include "sys_ctx.h"
#include "app_eeprom.h"
#include "ota_update.h"
#include "flash_if.h"
#include "string.h"
#include "stdio.h"
#include "gsm.h"
#include "hardware_manager.h"
#include "app_debug.h"
#include "app_rtc.h"
#include "server_msg.h"

static uint32_t m_sent_to_http_counter = 0;

// Build hardware ID string
static inline uint32_t build_device_id(uint32_t offset, char *ptr, char *imei)
{
    uint32_t len = 0;
    len = sprintf((char *)(ptr + offset), "\"ID\":\"G2V4-%s\",", imei);
    return len;
}

static inline uint32_t build_error_code(char *ptr, measure_input_peripheral_data_t *msg)
{
    bool found_break_pulse_input = false;
    char *tmp_ptr = ptr;
    sys_ctx_t *ctx = sys_ctx();
    app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
    uint32_t gsm_err_code;
    
    for (int i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
    {
        if (msg->counter[i].cir_break)
        {
            found_break_pulse_input = true;
            tmp_ptr += sprintf(tmp_ptr, "cir %d,", i + 1);
        }
    }

    if (found_break_pulse_input)
    {
        ctx->error_not_critical.detail.circuit_break = 1;
    }
    else
    {
        ctx->error_not_critical.detail.circuit_break = 0;
    }

    // Build error msg
    if (msg->vin_in_percent < eeprom_cfg->battery_low_percent)
    {
        tmp_ptr += sprintf(tmp_ptr, "%s", "pin yeu,");
    }

    // Flash
    if (ctx->error_not_critical.detail.flash)
    {
        tmp_ptr += sprintf(tmp_ptr, "%s", "flash,");
    }

    // Nhiet do
    if (msg->temperature > 60)
    {
        tmp_ptr += sprintf(tmp_ptr, "%s", "nhiet cao,");
    }
    
    // Build OTA error code
    ota_flash_cfg_t *cfg = ota_update_get_config();
    if (cfg->flag == OTA_UPDATE_FLAG_INVALID_BINARY_FIRMWARE)
    {
        tmp_ptr += sprintf(tmp_ptr, "%s", "OTA invalid,");
        
        // Clear error code
        ota_flash_cfg_t new_cfg;
        new_cfg.flag = OTA_FLAG_NO_NEW_FIRMWARE;
        new_cfg.firmware_size = 0;
        new_cfg.reserve[0] = 0;
        flash_if_write_ota_info_page((uint32_t *)&new_cfg, 
                                    sizeof(ota_flash_cfg_t)/sizeof(uint32_t));
    }
    
    // RS485
    bool found_rs485_err = false;
    
    // Scan for all RS485 slave
    for (uint32_t slave_idx = 0; 
        slave_idx < HARDWARE_RS485_MAX_SLAVE_ON_BUS; 
        slave_idx++)
    {
        // Scan for all register
        for (uint32_t sub_register_index = 0; 
            sub_register_index < HARDWARE_RS485_MAX_SUB_REGISTER; 
            sub_register_index++)
        {
            if (!eeprom_cfg->rs485[slave_idx].sub_reg[sub_register_index].data_type.name.valid)
            {
                continue;
            }
            if (!msg->rs485[slave_idx].sub_reg[sub_register_index].read_ok)
            {
                found_rs485_err = true;
                break;
            }
        }
    }

    if (found_rs485_err)
    {
        tmp_ptr += sprintf(tmp_ptr, "rs485-%u,", measure_input_485_error_code);
    }
    
    gsm_err_code = gsm_get_error_code();
    if (gsm_err_code)
    {
        tmp_ptr += sprintf(tmp_ptr, "gsm-%lu,", gsm_err_code);
    }

    // Cam bien
    if (ctx->error_critical.detail.sensor_out_of_range)
    {
        tmp_ptr += sprintf(tmp_ptr, "%s", "qua nguong,");
    }

    if (ptr[tmp_ptr - ptr - 1] == ',')
    {
        ptr[tmp_ptr - ptr - 1] = 0;
        tmp_ptr--; // remove char ','
    }

    tmp_ptr += sprintf(tmp_ptr, "%s", "\",");

    return tmp_ptr - ptr;
}

/* DTG01
{
    "Error": "cam_bien_xung_dut",
    "Timestamp": 1629200614,
    "ID": "G1-860262050125363",
    "Input1": 124511,
    "Outl": 0,
    "Out2": 0.00,
    "BatteryLevel": 80,
    "Vbat": 4101,
    "Temperature": 26,
    "SIM": 452018100001935,
    "Uptime": 7,
    "FW": "0.0.5",
    "HW": "0.0.1"
}
*/

/* DTG02
{
    "Timestamp": "1628275184",
    "ID": "G2-860262050125777",
    "Phone": "0916883454",
    "Money": 7649,
    "Inputl_J1": 7649,
    "Inputl_J3_1": 0.01,
    "Input1_J3_2": 0.00,
    "Input1_J3_3": 0.01,
    "Input1_J3_4 ": 0.01,
    "Input1_J9_1 ": 1,
    "Input1_g9_2 ": 1,
    "Inputl_J9_3 ": 1,
    "Input_J9_4 ": 71,
    "Output1 ": 0,
    "Out2": 0,
    "Out3": 0,
    "Out4": 0,
    "Out4_20": 0.0,
    "WarningLevel ": "0,0,0,0,1,0,1",
    "BatteryLevel ": 100,
    "Vin": 23.15,
    "Temperature ": 29,
    "Regl_1 ": 64,
    "Unitl_1 ": "m3/s",
    "Reg1_2 ": 339,
    "Unit1_2 ": "jun ",
    "Reg2_1": 0.0000,
    "Unit2_1": "kg",
    "Reg2_2": 0,
    "Unit2_2": "1it",
    "SIM": "452040700052210",
    "Uptime": "3",
    "FW": "0.0.5",
    "HW": "0.0.1"
}
 */

uint16_t gsm_application_build_http_post_message(char *ptr, measure_input_peripheral_data_t *msg)
{
    // app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
    //	measure_input_peripheral_data_t *measure_input = measure_input_current_data();
    //    DEBUG_VERBOSE("Free mem %u bytes\r\n", umm_free_heap_size());
    // sys_ctx_t *ctx = sys_ctx();

    //	char alarm_str[128];
    //    char *p = alarm_str;
    volatile uint16_t total_length = 0;

    uint64_t ts = msg->measure_timestamp;
    ts *= 1000;

    total_length += sprintf((char *)(ptr + total_length), "{\"ts\":%llu,\"values\":", ts);
    total_length += sprintf((char *)(ptr + total_length), "%s", "{\"Err\":\"");

    total_length += build_error_code(ptr + total_length, msg);

    // Build timestamp
    total_length += sprintf((char *)(ptr + total_length), "\"Timestamp\":%u,", msg->measure_timestamp); // second since 1970

    int32_t temp_counter;

    total_length += build_device_id(total_length, ptr, gsm_get_module_imei());
    
    // Build all meter input data
    for (int i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
    {
        // Build input pulse counter
        if (msg->counter[i].k == 0)
        {
            msg->counter[i].k = 1;
        }
        // Counter = real_counter / k_div + offset
        temp_counter = msg->counter[i].real_counter / msg->counter[i].k + msg->counter[i].indicator;
        total_length += sprintf((char *)(ptr + total_length), "\"Input1_J%d\":%d,",
                                i + 1,
                                temp_counter);

        temp_counter = msg->counter[i].reverse_counter / msg->counter[i].k /* + msg->counter[i].indicator */;
        total_length += sprintf((char *)(ptr + total_length), "\"Input1_J%d_D\":%u,",
                                i + 1,
                                temp_counter);

        // Total forward flow
        if (i)
        {
            total_length += sprintf((char *)(ptr + total_length), "\"ForwardFlow%d\":%.1f,",
                                    i + 1,
                                    msg->counter[i].flow_speed_fwd_agv_cycle_wakeup);

            total_length += sprintf((char *)(ptr + total_length), "\"ReverseFlow%d\":%.1f,",
                                    i + 1,
                                    msg->counter[i].flow_speed_reverse_agv_cycle_wakeup);

//            float tmp = msg->counter[i].fw_flow;
//            tmp /= msg->counter[i].k;
//            total_length += sprintf((char *)(ptr + total_length), "\"ForwardFlow%u\":%.2f,",
//                                                i+1,
//                                                tmp);
//
//            tmp = msg->counter[i].reverse_flow;
//            tmp /= msg->counter[i].k;
//            total_length += sprintf((char *)(ptr + total_length), "\"ReverseFlow%u\":%.2f,",
//                                                i+1,
//                                                tmp);
        }

        else
        {
//            float tmp = msg->counter[i].fw_flow;
//            tmp /= msg->counter[i].k;
//            total_length += sprintf((char *)(ptr + total_length), "\"ForwardFlow\":%.2f,",
//                                                tmp);
//
//            tmp = msg->counter[i].reverse_flow;
//            tmp /= msg->counter[i].k;
//            total_length += sprintf((char *)(ptr + total_length), "\"ReverseFlow\":%.2f,",
//                                                tmp);
            total_length += sprintf((char *)(ptr + total_length), "\"ForwardFlow\":%.1f,",
                                    msg->counter[i].flow_speed_fwd_agv_cycle_wakeup);

            total_length += sprintf((char *)(ptr + total_length), "\"ReverseFlow\":%.1f,",
                                    msg->counter[i].flow_speed_reverse_agv_cycle_wakeup);
        }

        // Total forward/reserve index : tong luu luong tich luy thuan/nguoc
        if (i)
        {
            total_length += sprintf((char *)(ptr + total_length), "\"ForwardIndex%d\":%u,",
                                    i + 1,
                                    msg->counter[i].total_fwd_idx);
            total_length += sprintf((char *)(ptr + total_length), "\"ReverseIndex%d\":%u,",
                                    i + 1,
                                    msg->counter[i].total_reverse_index);
        }
        else
        {
            total_length += sprintf((char *)(ptr + total_length), "\"ForwardIndex\":%u,",
                                    msg->counter[i].total_fwd_idx);
            total_length += sprintf((char *)(ptr + total_length), "\"ReverseIndex\":%u,",
                                    msg->counter[i].total_reverse_index);
        }

        if (msg->counter[i].flow_avg_cycle_send_web.valid)
        {
            if (i)
            {
                total_length += sprintf((char *)(ptr + total_length), "\"FwIdxHour%d\":%u,",
                                        i + 1,
                                        msg->counter[i].total_fwd_idx);
                total_length += sprintf((char *)(ptr + total_length), "\"RvsIdxHour%d\":%u,",
                                        i + 1,
                                        msg->counter[i].total_reverse_index);
            }
            else
            {
                total_length += sprintf((char *)(ptr + total_length), "\"FwIdxHour\":%u,",
                                        msg->counter[i].total_fwd_idx);
                total_length += sprintf((char *)(ptr + total_length), "\"RvsIdxHour\":%u,",
                                        msg->counter[i].total_reverse_index);
            }

            // Min max
            if (i)
            {
                // min max forward flow
                total_length += sprintf((char *)(ptr + total_length), "\"MinForwardFlow%d\":%.1f,",
                                        i + 1,
                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_min);

                total_length += sprintf((char *)(ptr + total_length), "\"MaxForwardFlow%d\":%.1f,",
                                        i + 1,
                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_max);

                // min max reserve flow
                total_length += sprintf((char *)(ptr + total_length), "\"MinReverseFlow%d\":%.1f,",
                                        i + 1,
                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_min);

                total_length += sprintf((char *)(ptr + total_length), "\"MaxReverseFlow%d\":%.1f,",
                                        i + 1,
                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_max);
            }
            else
            {
                // min max forward flow
                total_length += sprintf((char *)(ptr + total_length), "\"MinForwardFlow\":%.1f,",
                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_min);

                total_length += sprintf((char *)(ptr + total_length), "\"MaxForwardFlow\":%.1f,",
                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_max);

                // min max reserve flow
                total_length += sprintf((char *)(ptr + total_length), "\"MinReverseFlow\":%.1f,",
                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_min);

                total_length += sprintf((char *)(ptr + total_length), "\"MaxReverseFlow\":%.1f,",
                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_max);
            }

            // Hour
            if (i)
            {
                total_length += sprintf((char *)(ptr + total_length), "\"ForwardFlowSum%d\":%.1f,",
                                        i + 1,
                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_sum);
                total_length += sprintf((char *)(ptr + total_length), "\"ReverseFlowSum%d\":%.1f,",
                                        i + 1,
                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_sum);
            }

            else
            {
                total_length += sprintf((char *)(ptr + total_length), "\"ForwardFlowSum\":%.1f,",
                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_sum);
                total_length += sprintf((char *)(ptr + total_length), "\"ReverseFlowSum\":%.1f,",
                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_sum);
            }
//            DEBUG_WARN("Send to server Min-max fw flow%u %.2f %.2f\r\n", i+1, msg->counter[i].flow_avg_cycle_send_web.fw_flow_min,
//                                                        msg->counter[i].flow_avg_cycle_send_web.fw_flow_max);
//
//            DEBUG_WARN("Send to server Min-max resv flow%u %.2f %.2f\r\n", i+1, msg->counter[i].flow_avg_cycle_send_web.reverse_flow_min,
//                                                        msg->counter[i].flow_avg_cycle_send_web.reverse_flow_max);
        }

        //		// K : he so chia cua dong ho nuoc, input 1
        //		// Offset: Gia tri offset cua dong ho nuoc
        //		// Mode : che do hoat dong
        //		total_length += sprintf((char *)(ptr + total_length), "\"K%u\":%u,", i+1, msg->counter[i].k);
        //		//total_length += sprintf((char *)(ptr + total_length), "\"Offset%u\":%u,", i+1, eeprom_cfg->offset[i]);
        //		total_length += sprintf((char *)(ptr + total_length), "\"M%u\":%u,", i+1, msg->counter[i].mode);
    }

    // Build input 4-20ma
    for (uint8_t nb_of_input_4_20ma = 0; 
                nb_of_input_4_20ma < HARDWARE_NUMBER_OF_INPUT_4_20MA; 
                nb_of_input_4_20ma++)
    {
        total_length += sprintf((char *)(ptr + total_length), "\"Input1_J3_%d\":%.1f,",
                                nb_of_input_4_20ma, msg->input_4_20mA[nb_of_input_4_20ma]); // dau vao 4-20mA 0
    }

    // Build input analog pressure sensor
    for (uint8_t nb_of_input_pressure = 0; 
            nb_of_input_pressure < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; 
            nb_of_input_pressure++)
    {
        total_length += sprintf((char *)(ptr + total_length), "\"PA_%d\":%u,",
                                nb_of_input_pressure, msg->input_pressure_analog[nb_of_input_pressure]); // dau vao 4-20mA 0
    }


    // Build input on/off
    for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_INPUT_ON_OFF; i++)
    {
        total_length += sprintf((char *)(ptr + total_length), "\"Input1_J9_%d\":%u,",
                                i + 1,
                                msg->input_on_off[i]); // dau vao 4-20mA 0
    }

    // Build output 4-20ma
    if (msg->output_4_20ma_enable && (msg->output_4_20mA[0] < 4.0f))
    {
        total_length += sprintf((char *)(ptr + total_length), 
                                "\"Output4_20\":%.1f,", 
                                msg->output_4_20mA[0]); // dau ra 4-20mA
    }
    
    for (uint8_t analogInput = 0; analogInput < HARDWARE_NUMBER_OF_ANALOG_INPUT; analogInput++)
    {
        // Convert analog input to pressure
//        float tmp = msg->analog_input[analogInput];
//        tmp *= 0.003367f;       // hardcode, dont care about it
        total_length += sprintf((char *)(ptr + total_length), 
                                "\"Anl%u\":%u,", 
                                analogInput, 
                                msg->analog_input[analogInput]);
        // total_length += sprintf((char *)(ptr + total_length), "\"PAnl\":%.2f,", tmp);
    }


    // CSQ in percent
//    if (msg->csq_percent)
    {
        total_length += sprintf((char *)(ptr + total_length), 
                                "\"SignalStrength\":%d,", 
                                msg->csq_percent);
    }

    // Warning level
    //    total_length += sprintf((char *)(ptr + total_length), "\"WarningLevel\":\"%s\",", alarm_str);
    total_length += sprintf((char *)(ptr + total_length), "\"BatteryLevel\":%d,", msg->vin_in_percent);
    total_length += sprintf((char *)(ptr + total_length), "\"Vbat\":%u,", msg->internal_battery_voltage);
    //#ifndef DTG01    // DTG02 : Vinput 24V
    //    total_length += sprintf((char *)(ptr + total_length), "\"Vin\":%.1f,", msg->vin_mv/1000);
    //#endif
    // Temperature
    total_length += sprintf((char *)(ptr + total_length), "\"Temperature\":%d,", msg->temperature);

//    // Reset reason
//    total_length += sprintf((char *)(ptr + total_length), "\"RST\":%u,", hardware_manager_get_reset_reason()->value);
// Reg0_1:1234, Reg0_2:4567, Reg1_0:12345
#if 1
    for (uint32_t index = 0; index < HARDWARE_RS485_MAX_SLAVE_ON_BUS; index++)
    {
        // Value
        //        total_length += sprintf((char *)(ptr + total_length), "\"SlID%u\":%u,", index+1, msg->rs485[index].slave_addr);
        for (uint32_t sub_idx = 0; sub_idx < HARDWARE_RS485_MAX_SUB_REGISTER; sub_idx++)
        {
            if (msg->rs485[index].sub_reg[sub_idx].read_ok 
                && msg->rs485[index].sub_reg[sub_idx].data_type.name.valid)
            {
                // uint32_t tmp_len = total_length;
                total_length += sprintf((char *)(ptr + total_length), "\"Rg%u_%u\":", index + 1, sub_idx + 1);
                if (msg->rs485[index].sub_reg[sub_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_INT16 
                    || msg->rs485[index].sub_reg[sub_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_INT32)
                {
                    total_length += sprintf((char *)(ptr + total_length), 
                                            "%u,", 
                                            msg->rs485[index].sub_reg[sub_idx].value.int32_val);
                }
                else
                {
                    // dump hexa
                    total_length += sprintf((char *)(ptr + total_length), 
                                            "%.1f,", 
                                            msg->rs485[index].sub_reg[sub_idx].value.float_val);
//                    total_length += sprintf((char *)(ptr + total_length), "\"%.1f - 0x%02X%02X%02X%02X\",",
                                            // msg->rs485[index].sub_reg[sub_idx].value.t_float_val,
                                            // msg->rs485[index].sub_reg[sub_idx].value.raw[0],
                                            // msg->rs485[index].sub_reg[sub_idx].value.raw[1],
                                            // msg->rs485[index].sub_reg[sub_idx].value.raw[2],
                                            // msg->rs485[index].sub_reg[sub_idx].value.raw[3]);
                }
//            if (strlen((char*)msg->rs485[index].sub_reg[sub_idx].unit))
//            {
//                total_length += sprintf((char *)(ptr + total_length), "\"U%u_%u\":\"%s\",", 
//                                          index+1, sub_idx+1, msg->rs485[index].sub_reg[sub_idx].unit);
//            }
//            DEBUG_WARN("%s\r\n", (ptr+tmp_len));
            }
            else if (msg->rs485[index].sub_reg[sub_idx].data_type.name.valid)
            {
                uint32_t tmp_len = total_length;
//                if (strlen((char*)msg->rs485[index].sub_reg[sub_idx].unit))
//                {
//                    total_length += sprintf((char *)(ptr + total_length), "\"U%u_%u\":\"%s\",", 
//                                              index+1, sub_idx+1, msg->rs485[index].sub_reg[sub_idx].unit);
//                }
                total_length += sprintf((char *)(ptr + total_length), 
                                        "\"Rg%u_%u\":\"%s\",", 
                                        index + 1, 
                                        sub_idx + 1, 
                                        "FFFF");
//                DEBUG_WARN("%s\r\n", (ptr+tmp_len));
            }
        }
    }
#endif
    // Sim imei
    total_length += sprintf((char *)(ptr + total_length), "\"SIM\":%s,", gsm_get_sim_imei());
    // total_length += sprintf((char *)(ptr + total_length), "\"CCID\":\"%s\",", gsm_get_sim_ccid());

    // Uptime
    //    total_length += sprintf((char *)(ptr + total_length), "\"Uptime\":%u,", m_wake_time);
    total_length += sprintf((char *)(ptr + total_length), "\"Sendtime\":%u,", ++m_sent_to_http_counter);
    DEBUG_INFO("Send time %u, ts %u\r\n", m_sent_to_http_counter, msg->measure_timestamp);
    
    // app_eeprom_factory_data_t *factory = app_eeprom_read_factory_data();
    //    if (factory->baudrate.baudrate_valid_key == EEPROM_BAUD_VALID)
    //    {
    //        total_length += sprintf((char *)(ptr + total_length), "\"baud\":%u,", factory->baudrate.value);
    //    }
    //    else
    //    {
    //        total_length += sprintf((char *)(ptr + total_length), "\"baud\":%u,", APP_EEPROM_DEFAULT_BAUD);
    //    }

    //    total_length += sprintf((char *)(ptr + total_length), "\"bytes\":%u,", factory->byte_order);
    //    total_length += sprintf((char *)(ptr + total_length), "\"pulse\":%u,", factory->pulse_ms);

    //	// Release date
    //	total_length += sprintf((char *)(ptr + total_length), "\"Build\":\"%s %s\",", __DATE__, __TIME__);

    //    total_length += sprintf((char *)(ptr + total_length), "\"FacSVR\":\"%s\",", app_eeprom_read_factory_data()->server);

    //    // Firmware and hardware
    static uint8_t send_reset = 1;
    if (send_reset)
    {
        send_reset = 0;
        total_length += sprintf((char *)(ptr + total_length), 
                                "\"Rst\":%u,", 
                                hardware_manager_get_reset_reason()->value);
    }
    total_length += sprintf((char *)(ptr + total_length), "\"FW\":\"%s\",", VERSION_CONTROL_FW);
    
    // Send time to internet
    // "fixedWakeTime":"12:34,12:36",
    int16_t *fixed_wakeup_time = app_eeprom_get_fixed_time_wakeup_setting();
    char tmp[32];
    memset(tmp, 0, sizeof(tmp));
    int tmp_len = 0;
    for (uint32_t i = 0 ; i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME; i++)
    {
        if (fixed_wakeup_time[i] != APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME)
        {
            tmp_len += sprintf(tmp+tmp_len, "%02u:%02u,", fixed_wakeup_time[i]/60, fixed_wakeup_time[i]%60);
        }
    }
    if (tmp_len)
    {
        tmp_len--;
        tmp[tmp_len] = 0;
        total_length += sprintf((char *)(ptr + total_length), "\"fixedWakeTime\":\"%s\",", tmp);
    }
    
    total_length += sprintf((char *)(ptr + total_length), "\"FW\":\"%s\",", VERSION_CONTROL_FW);
    
    total_length += sprintf((char *)(ptr + total_length), "\"HW\":\"%s\"}}", VERSION_CONTROL_HW);

    //    hardware_manager_get_reset_reason()->value = 0;
    // DEBUG_INFO("Size %u, data %s\r\n", total_length, (char*)ptr);
    //    usart_lpusart_485_control(true);
    //    sys_delay_ms(500);
    //    usart_lpusart_485_send((uint8_t*)ptr, total_length);

    return total_length;
}

uint32_t gsm_application_estimate_wakeup_time(void)
{
    uint32_t wake_time;
    app_eeprom_config_data_t *cfg = app_eeprom_read_config_data();
    uint32_t current_sec = app_rtc_get_counter();
    uint32_t send_interval_s = cfg->send_to_server_interval_ms / 1000 + cfg->send_to_server_delay_s;
    if (send_interval_s == 0)
    {
        send_interval_s = 1;
    }
    
    // Thoi gian wakeup la 1 so chan cua chu ki, tinh tu moc 0h00p
    // Vi du chu ki la 15p ->> thoi gian la 0, 15, 30,...
    // Thoi gian 1, 16, 31, 46 la khong hop le
    // Yeu cau cua khach hang la nhu vay nen dung hoi nhe :D
    // Tuong tu thoi gian thuc day doc cam bien cung phai la so chan
    wake_time = send_interval_s * (current_sec / send_interval_s + 1) + cfg->send_to_server_delay_s;
//        DEBUG_VERBOSE("Estimate next wakeup time %us\r\n", estimate_wakeup_time);
    return wake_time;
}

bool gsm_application_send_sms_to_master_phone(void)
{
    bool allow_send_sms = false;
    sys_ctx_t *ctx = sys_ctx();
    app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
    char msg[128];
    char *p = msg;
    char *phone;
    
    // If enable master phone number ->> Send active message
    if (eeprom_cfg->io_enable.name.register_sim_status == 0 
        && strlen((char *)eeprom_cfg->phone) > 9 
        && eeprom_cfg->io_enable.name.warning 
        && (ctx->status.total_sms_in_24_hour < eeprom_cfg->max_sms_1_day))
    {
        rtc_date_time_t time;
        app_rtc_get_time(&time);
        
        p += sprintf(p, "%04u/%02u/%02u %02uh",
                     time.year + 2000,
                     time.month,
                     time.day,
                     time.hour);
        
        p += sprintf(p, "%s %s, sim %s, ccid %s active", 
                    OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE, 
                    gsm_get_module_imei(), 
                    gsm_get_sim_imei(),
                    gsm_get_sim_ccid());
        

        // Send sms
        phone = (char *)eeprom_cfg->phone;
        allow_send_sms = true;
        eeprom_cfg->io_enable.name.register_sim_status = 1;
    }
    else if (eeprom_cfg->io_enable.name.notify_imei == 0
            && (ctx->status.total_sms_in_24_hour < eeprom_cfg->max_sms_1_day))
    {
        rtc_date_time_t time;
        app_rtc_get_time(&time);
        p += sprintf(p, "%04u/%02u/%02u %02u:%02u ",
                     time.year + 2000,
                     time.month,
                     time.day,
                     time.hour,
                     time.minute);
        p += sprintf(p, "TB %s %s %s %s", OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE, 
                                            gsm_get_module_imei(), 
                                            gsm_get_sim_imei(),
                                            gsm_get_sim_ccid());

        // Send sms
        phone = "0942018895";
        
        eeprom_cfg->io_enable.name.notify_imei = 1;
        allow_send_sms = true;
    }   
    
    if (allow_send_sms)
    {
        ctx->status.total_sms_in_24_hour++;
        app_eeprom_save_config();
        gsm_send_sms(phone, msg);
        gsm_send_sms((char *)eeprom_cfg->phone, msg);
    }
    
    return allow_send_sms;
}

uint32_t gsm_appication_next_time_poll_boardcast_command(void)
{
    /*
     Dinh ki phai poll 1 server mac dinh de lay cau hinh
    */
    sys_ctx_t *ctx = sys_ctx();
    uint32_t current_sec = app_rtc_get_counter();
    uint32_t retval = 0;
    
    if (current_sec >= ctx->status.next_time_get_data_from_server)
    {
        retval = current_sec / 3600 
                + app_eeprom_read_config_data()->poll_config_interval_hour;
        retval *= 3600;
    }    
    return retval;
}

void gsm_application_on_http_get_data(uint8_t *data, uint32_t len, uint8_t *has_new_config)
{
    // app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
    sys_ctx_t *ctx = sys_ctx();
    *has_new_config = 0;
    
    // Blink led indicator : customer requirement
    HARDWARE_LED1_CTRL(1);
    
    if (!ctx->status.enter_ota_update)
    {
        // Neu dang o trang thai quet boardcast message tu server
        if (sys_ctx()->status.poll_broadcast_msg_from_server)
        {
            sys_ctx()->status.poll_broadcast_msg_from_server = 0;
            server_msg_process_boardcast_cmd((char*)data);
        }
        else    // Dang o http get dinh ki
        {   
            // Parse server command
            server_msg_process_cmd((char*)data, has_new_config);
        }
    }
    else
    {
#if OTA_VERSION == 0
        ota_update_write_next((uint8_t *)get_data->data, get_data->data_length);
#endif
    }
}

