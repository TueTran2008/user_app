/******************************************************************************
 * @file    	Measurement.c
 * @author
 * @version 	V1.0.0
 * @date    	15/01/2014
 * @brief
 ******************************************************************************/

/******************************************************************************
                                   INCLUDES
 ******************************************************************************/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "measure_input.h"
#include "hardware.h"
#include "gsm_utilities.h"
#include "hardware_manager.h"
#include "main.h"
#include "app_bkup.h"
#include "gsm.h"
#include "lwrb.h"
#include "app_eeprom.h"
#include "adc.h"
#include "usart.h"
#include "modbus_master.h"
#include "sys_ctx.h"
#include "modbus_memory.h"
#include "app_debug.h"
#include "app_rtc.h"
#include "rtc.h"
#include "stm32l0xx_ll_rtc.h"
#include "app_spi_flash.h"
#include "spi.h"
#include "modbus_master.h"
#include "jig.h"
#include "tim.h"
#include "utilities.h"
#include "app_flash_log.h"

#define VBAT_DETECT_HIGH_MV 9000
#define STORE_MEASURE_INVERVAL_SEC 30
#define ADC_MEASURE_INTERVAL_MS 30000
#define PULSE_STATE_WAIT_FOR_FALLING_EDGE 0
#define PULSE_STATE_WAIT_FOR_RISING_EDGE 1
//#define PULSE_DIR_FORWARD_LOGICAL_LEVEL         1
#define ADC_OFFSET_MA 0 // 0.6mA, mul by 10

#define CHECK_BOTH_INPUT_EDGE 0

#if CHECK_BOTH_INPUT_EDGE
#define PULSE_MINMUM_WITDH_MS 30
#else
#define PULSE_MINMUM_WITDH_MS 2
#endif

#define ALWAYS_SAVE_DATA_TO_FLASH 1
#define CALCULATE_FLOW_SPEED 1
#define MODBUS_FLOW_CAL_METHOD // Method = 0 =>> modbus flow = gia tri thanh ghi
                               // method = 1 =>> modbus flow = gia tri thanh ghi sau - gia tri thanh ghi truoc
#define MODBUS_READ_REGISTER_RETRY  2
#define MEASUREMENT_CYCLE_EXPIRE()  (m_this_is_the_first_time || (current_sec >= estimate_measure_timestamp))
#define INPUT_4_20MA_INVALID_VALUE                  (-1.0f)
#define INPUT_485_INVALID_FLOAT_VALUE               (-1.0f)    
#define INPUT_485_INVALID_INT_VALUE                 (-1)    


typedef measure_input_cyber_counter_t backup_pulse_data_t;

typedef struct
{
    uint32_t counter_pwm;
    int32_t subsecond_pwm;
    uint32_t counter_dir;
    int32_t subsecond_dir;
} measure_input_timestamp_t;

typedef struct
{
    uint8_t pwm;
    uint8_t dir;
} measure_input_pull_state_t;

volatile uint8_t m_is_pulse_trigger = 0;

measure_input_timestamp_t m_begin_pulse_timestamp[MEASURE_NUMBER_OF_WATER_METER_INPUT];
measure_input_timestamp_t m_end_pulse_timestamp[MEASURE_NUMBER_OF_WATER_METER_INPUT];

// PWM, DIR interrupt timestamp between 2 times interrupt called
static uint32_t m_pull_diff[MEASURE_NUMBER_OF_WATER_METER_INPUT];

static backup_pulse_data_t m_pulse_cnt_in_bkp[MEASURE_NUMBER_OF_WATER_METER_INPUT];
static backup_pulse_data_t m_pre_pulse_counter_in_bkp[MEASURE_NUMBER_OF_WATER_METER_INPUT];
static measure_input_peripheral_data_t m_measure_data;
volatile uint32_t store_measure_result_timeout = 0;
static bool m_this_is_the_first_time = true;
measure_input_peripheral_data_t m_sensor_msq[MEASUREMENT_MAX_MSQ_IN_RAM];
volatile uint32_t delay_turn_on_power_for_external_sensor = MEASURE_INTPUT_WAIT_4_20MA_SENSOR_POWER_ON_MS;

// Min-max value of water flow speed between 2 times send data to web
static float fw_flow_min_in_cycle_send_web[MEASURE_NUMBER_OF_WATER_METER_INPUT];
static float fw_flow_max_in_cycle_send_web[MEASURE_NUMBER_OF_WATER_METER_INPUT];
static float reverse_flow_min_in_cycle_send_web[MEASURE_NUMBER_OF_WATER_METER_INPUT];
static float reverse_flow_max_in_cycle_send_web[MEASURE_NUMBER_OF_WATER_METER_INPUT];


// Forward flow
static float mb_fw_flow_index[HARDWARE_RS485_MAX_SLAVE_ON_BUS];
// Reverse flow
static float mb_rvs_flow_index[HARDWARE_RS485_MAX_SLAVE_ON_BUS];

// Total forward flow
static float mb_net_totalizer_fw_flow_index[HARDWARE_RS485_MAX_SLAVE_ON_BUS];

// Total reverse flow
static float mb_net_totalizer_reverse_flow_index[HARDWARE_RS485_MAX_SLAVE_ON_BUS];

// Total flow = forward - reverse
static float mb_net_totalizer[HARDWARE_RS485_MAX_SLAVE_ON_BUS];

// RS485 error code
uint8_t measure_input_485_error_code = MODBUS_MASTER_OK;

static volatile bool m_allow_blink_indicator_when_pulse = true;

// Read data from input register
static uint8_t read_input_register(uint8_t slave_addr, 
                                    uint16_t reg_addr, 
                                    uint8_t number_of_register, 
                                    uint32_t delay_time)
{
    uint8_t err;
    
//                        DEBUG_INFO("MB id %u, offset %u, size %u\r\n", slave_addr, reg_addr, number_of_half_word);
    for (uint32_t i = 0; i < MODBUS_READ_REGISTER_RETRY; i++)
    {
        err = modbus_master_read_input_register(slave_addr,
                                                 reg_addr,
                                                 number_of_register);
        if (err != MODBUS_MASTER_OK) // Read data error
        {
            sys_delay_ms(delay_time);
        }
        else        // Read success -> break
        {
            break;
        }
    }
    return err;
}

static void process_rs485(measure_input_modbus_register_t *reg_value)
{
    app_eeprom_config_data_t *ee_cfg = app_eeprom_read_config_data();
    sys_ctx_t *ctx = sys_ctx();
    bool do_stop = true;

    // If rs485 enable =>> Start measure data
    if (ee_cfg->io_enable.name.rs485_en && ctx->status.is_enter_test_mode)
    {
        goto exit;
    }
    
    // Power on RS485 hardware & initialize USART
    ctx->peripheral_running.name.rs485_running = 1;
    HARDWARE_RS485_POWER_EN(1);
    usart_rs485_control(1);
    HARDWARE_RS485_DIR_RX();
    
    // Scan every slave id in RS485 bus
    for (uint32_t slave_cnt = 0; slave_cnt < HARDWARE_RS485_MAX_SLAVE_ON_BUS; slave_cnt++)
    {
        uint16_t modbus_timeout = 1000;     // ms
        
        for (uint32_t sub_reg_idx = 0; sub_reg_idx < HARDWARE_RS485_MAX_SUB_REGISTER; sub_reg_idx++)
        {
            if (ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].data_type.name.valid == 0)
            {
                ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].read_ok = 0;
                continue;
            }
            
            sys_delay_ms(15); // delay between 2 transaction
            // convert register to function code and offset
            // ex : 30001 =>> function code = 04, offset = 01
            uint32_t function_code = ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].reg_addr / 10000;
            uint32_t reg_addr = ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].reg_addr - function_code * 10000;
            function_code += 1; // 30001 =>> function code = 04 read input register
            
            // Get slave address
            uint32_t slave_addr = ee_cfg->rs485[slave_cnt].slave_addr;
            reg_value[slave_cnt].sub_reg[sub_reg_idx].data_type.name.valid = 1;
            if (reg_addr) // For example, we want to read data at addr 30100 =>> reg_addr = 99
            {
                reg_addr--;
            }
            switch (function_code)
            {
            case MODBUS_MASTER_FUNCTION_READ_INPUT_REGISTER:
            {
//                        uint8_t result;

                modbus_master_reset(modbus_timeout);
                
                // Default data is halfword
                uint32_t number_of_half_word = 1;
                
                // If type == float or int -> read 4 byte
                if (ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_FLOAT 
                    || ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_INT32)
                {
                    number_of_half_word = 2;
                }
                
                // Read input register
                measure_input_485_error_code = read_input_register(slave_addr, reg_addr, number_of_half_word, modbus_timeout);

                
                // Copy slave address
                reg_value[slave_cnt].slave_addr = slave_addr;
                
                // 30000 = read input register
                uint16_t estimated_reg = (30000 + reg_addr + 1);
                
                if (measure_input_485_error_code != MODBUS_MASTER_OK) // Read data error
                {
//                            DEBUG_ERROR("Modbus read input register failed code %d\r\n", measure_input_485_error_code);
                    modbus_timeout = 100; // if 1 register failed =>> maybe other register will be fail =>> Reduce delay time
                    reg_value[slave_cnt].sub_reg[sub_reg_idx].read_ok = 0;
                    modbus_master_clear_response_buffer();
                    
                    mb_fw_flow_index[slave_cnt] = INPUT_485_INVALID_FLOAT_VALUE;
                    mb_rvs_flow_index[slave_cnt] = INPUT_485_INVALID_FLOAT_VALUE;

                    ctx->error_not_critical.detail.rs485_err = 1;
                }
                else // Read data ok
                {
                    // Read ok, copy result to buffer
                    reg_value[slave_cnt].sub_reg[sub_reg_idx].read_ok = 1;
                    reg_value[slave_cnt].sub_reg[sub_reg_idx].value.int32_val = modbus_master_get_response_buffer(0);

                    if (number_of_half_word == 2)
                    {
                        // If LSB first
                        if (ee_cfg->io_enable.name.rs485_byte_order == APP_EEPROM_MODBUS_LSB_FIRST)
                        {
                            // Byte order : Float, int32 (2-1,4-3)
                            // int16 2 - 1
                            uint32_t tmp = modbus_master_get_response_buffer(1);
                            reg_value[slave_cnt].sub_reg[sub_reg_idx].value.int32_val |= (tmp << 16);
                        }
                        else        // MSB
                        {
                            reg_value[slave_cnt].sub_reg[sub_reg_idx].value.int32_val <<= 16;
                            reg_value[slave_cnt].sub_reg[sub_reg_idx].value.int32_val |= modbus_master_get_response_buffer(1);
                        }
//                                DEBUG_RAW("\r\nInt32 register =>> %08X\r\n", reg_value[slave_cnt].sub_reg[sub_reg_idx].value);
                    }
                    
//                      DEBUG_RAW("%u-0x%08X\r\n", ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].reg_addr,
//                              reg_value[slave_cnt].sub_reg[sub_reg_idx].value);
                    reg_value[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type = ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type;
                    
                    // Current flow value, default data type is is float
                    float current_flow_idx = reg_value[slave_cnt].sub_reg[sub_reg_idx].value.float_val;
                    
                    // If type int 32bit
                    if (ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type 
                        == MEASURE_INPUT_RS485_DATA_TYPE_INT32)
                    {
                        current_flow_idx = reg_value[slave_cnt].sub_reg[sub_reg_idx].value.int32_val;
                    }

//                            DEBUG_WARN("Flow set %.3f\r\n", current_flow_idx);

                    // Get net totalizer forward and reverse value
                    for (uint32_t slave_on_bus = 0; slave_on_bus < HARDWARE_RS485_MAX_SLAVE_ON_BUS; slave_on_bus++)
                    {
                        measure_input_modbus_register_t *modbus_reg = &ee_cfg->rs485[slave_on_bus];

                        // Net forward value
                        // If register == net  totalizer forward flow register, 30000 = read input reg
                        if (estimated_reg == modbus_reg->net_totalizer_fw_reg) 
                        {
                            if (mb_net_totalizer_fw_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                            {
                                mb_net_totalizer_fw_flow_index[slave_cnt] = current_flow_idx;
                            }
                            else
                            {
                            }
                        }

                        // Net reverse
                        // If register == net  totalizer forward flow register, 30000 = read input reg
                        if (estimated_reg == modbus_reg->net_totalizer_reverse_reg) 
                        {

                            if (mb_net_totalizer_reverse_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                            {
                                mb_net_totalizer_reverse_flow_index[slave_cnt] = current_flow_idx;
                            }
                            else
                            {
                            }
                        }

                        // Net total
                         // If register == net totalizer forward flow register, 30000 = read input reg
                        if (estimated_reg == modbus_reg->net_totalizer_reg)
                        {
                            if (mb_net_totalizer[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                            {
                                mb_net_totalizer[slave_cnt] = current_flow_idx;
                            }
                            else
                            {
                            }
                        }
                    }

                    if (ee_cfg->io_enable.name.modbus_cal_method) // ref MODBUS_FLOW_CAL_METHOD
                    {
                        // Scan for all slave on bus
                        for (uint32_t slave_on_bus = 0; 
                                        slave_on_bus < HARDWARE_RS485_MAX_SLAVE_ON_BUS; 
                                        slave_on_bus++)
                        {
                             // If register == forward flow register, 30000 = read input reg
                            measure_input_modbus_register_t *modbus_reg = &ee_cfg->rs485[slave_on_bus];
                            if (estimated_reg == modbus_reg->fw_flow_reg)
                            {
                                bool data_is_valid = false;
                                float tmp = current_flow_idx;
                                if (tmp < 0.0f) // neu ma so nuoc < 0, thi tuc la dong ho quay nguoc =>> forward flow = 0
                                {
                                    tmp = 0;
                                }

                                if (mb_fw_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                                {
                                    // neu la lan dau tien =>> set gia tri mac dinh
                                    mb_fw_flow_index[slave_cnt] = tmp;
                                }
                                else
                                {
                                    // luu luong = so nuoc hien tai -  so nc cu
                                    mb_fw_flow_index[slave_cnt] = tmp;
//                                             DEBUG_WARN("New flow %.1f\r\n", m_485_min_max[slave_on_bus].forward_flow.t_float);
                                    data_is_valid = true;
                                }

                                if (reg_value[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_FLOAT 
                                    && data_is_valid) // If data type is float
                                {

                                }
                            }
                            
                            // Reverse flow register
                            if (estimated_reg == modbus_reg->reverse_flow_reg 
                                && (modbus_reg->reverse_flow_reg == modbus_reg->fw_flow_reg)) // If register == reserve flow register
                            {
                                float tmp = current_flow_idx;
                                // Min-max
                                //  neu ma so nuoc < 0, thi tuc la dong ho quay nguoc =>> forward flow = 0, reverse flow can doi nguoc lai
                                if (tmp > 0.0f)
                                {
                                    tmp = 0.0f;
                                }
                                tmp = fabs(tmp);
                                if (mb_rvs_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                                {
                                    // neu la lan dau tien =>> set gia tri mac dinh
                                    mb_rvs_flow_index[slave_cnt] = tmp;
                                }
                                else
                                {
                                    // luu luong = so nuoc hien tai -  so nc cu
                                    mb_rvs_flow_index[slave_cnt] = tmp;
                                }
                            }
                            else if (estimated_reg == modbus_reg->reverse_flow_reg 
                                    && (modbus_reg->reverse_flow_reg != modbus_reg->fw_flow_reg)) // If register == reserve flow register
                            {
                                bool data_is_valid = false;
                                // Min-max
                                //  neu ma so nuoc < 0, thi tuc la dong ho quay nguoc =>> forward flow = 0, reverse flow can doi nguoc lai
                                if (mb_rvs_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                                {
                                    // neu la lan dau tien =>> set gia tri mac dinh
                                    mb_rvs_flow_index[slave_cnt] = current_flow_idx;
                                }
                                else
                                {
                                    mb_rvs_flow_index[slave_cnt] = current_flow_idx;
                                    data_is_valid = true;
                                }

                                if (reg_value[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_FLOAT 
                                    && data_is_valid) // If data type is float
                                {

                                }
                            }
                        }
                    }
                    else
                    {
                        for (uint32_t slave_on_bus = 0; slave_on_bus < HARDWARE_RS485_MAX_SLAVE_ON_BUS; slave_on_bus++)
                        {
                            measure_input_modbus_register_t *modbus_reg = &ee_cfg->rs485[slave_on_bus];
                            if (estimated_reg == modbus_reg->fw_flow_reg) // If register == forward flow register, 30000 = read input reg
                            {
                                bool data_is_valid = false;
                                // Min-max
                                if (mb_fw_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                                {
                                    // neu la lan dau tien =>> set gia tri mac dinh
                                    mb_fw_flow_index[slave_cnt] = current_flow_idx;
                                }
                                else
                                {
                                    mb_fw_flow_index[slave_cnt] = current_flow_idx;
                                    data_is_valid = true;
                                }

                                if (reg_value[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_FLOAT 
                                    && data_is_valid) // If data type is float
                                {
                                }
                            }
                            if (estimated_reg == modbus_reg->reverse_flow_reg) // If register == reserve flow register
                            {
                                bool data_is_valid = false;
                                // Min-max
                                if (mb_rvs_flow_index[slave_cnt] == INPUT_485_INVALID_FLOAT_VALUE)
                                {
                                    // neu la lan dau tien =>> set gia tri mac dinh
                                    mb_rvs_flow_index[slave_cnt] = current_flow_idx;
                                }
                                else
                                {
                                    mb_rvs_flow_index[slave_cnt] = current_flow_idx;
                                    data_is_valid = true;
                                }

                                if (reg_value[slave_cnt].sub_reg[sub_reg_idx].data_type.name.type == MEASURE_INPUT_RS485_DATA_TYPE_FLOAT 
                                    && data_is_valid) // If data type is float
                                {

                                }
                            }
                        }
                    }
                }

                reg_value[slave_cnt].sub_reg[sub_reg_idx].reg_addr = ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].reg_addr;
                ctx->error_not_critical.detail.rs485_err = 0;
                
                // Copy register unit
                strncpy((char *)reg_value[slave_cnt].sub_reg[sub_reg_idx].unit,
                        (char *)ee_cfg->rs485[slave_cnt].sub_reg[sub_reg_idx].unit,
                        6);
            }
            break;

            // Unsupported function code
            // Due to my laziness, i dont want to handle these function
            case MODBUS_MASTER_FUNCTION_READ_COILS:
            case MODBUS_MASTER_FUNCTION_READ_DISCRETE_INPUT:
            case MODBUS_MASTER_FUNCTION_READ_HOLDING_REGISTER:
            default:
                break;
            }
        }
    }
    HARDWARE_RS485_POWER_EN(0);
    sys_delay_ms(1000);
    
exit:
    if (ctx->status.is_enter_test_mode)
    {
        do_stop = false;
    }
    
    if (do_stop)
    {
        ctx->peripheral_running.name.rs485_running = 0;
        usart_rs485_control(0);
    }
}

void measure_input_pulse_counter_poll(void)
{
    if (m_is_pulse_trigger)
    {
        char ptr[48];
        uint8_t total_length = 0;
        uint32_t temp_counter;
        app_eeprom_config_data_t *ee_cfg = app_eeprom_read_config_data();

        // Build debug counter message
        for (uint32_t i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
        {
            temp_counter = m_pulse_cnt_in_bkp[i].real_counter / ee_cfg->cyber_config[i].k 
                            + ee_cfg->cyber_config[i].offset;
            total_length += sprintf((char *)(ptr + total_length), "(%u-",
                                    temp_counter);

            temp_counter = m_pulse_cnt_in_bkp[i].reverse_counter / ee_cfg->cyber_config[i].k /* + ee_cfg->offset[i] */;
            total_length += sprintf((char *)(ptr + total_length), "%u),",
                                    temp_counter);

            m_pulse_cnt_in_bkp[i].k = ee_cfg->cyber_config[i].k;
            m_pulse_cnt_in_bkp[i].indicator = ee_cfg->cyber_config[i].offset;
        }

        app_bkup_write_pulse_counter(&m_pulse_cnt_in_bkp[0]);
#if 1

        DEBUG_VERBOSE("Counter(0-1) : (%u-%u), (%u-%u), real value %s\r\n",
                   m_pulse_cnt_in_bkp[0].real_counter, m_pulse_cnt_in_bkp[0].reverse_counter,
                   m_pulse_cnt_in_bkp[1].real_counter, m_pulse_cnt_in_bkp[1].reverse_counter,
                   ptr);
#endif
        m_is_pulse_trigger = 0;
    }
    sys_ctx()->peripheral_running.name.measure_input_pwm_running = 0;
}

static uint32_t m_last_time_measure_data = 0;
uint32_t m_number_of_adc_conversion = 0;
app_eeprom_config_data_t *ee_cfg;
void measure_input_measure_wakeup_to_get_data()
{
    m_this_is_the_first_time = true;
    sys_ctx()->peripheral_running.name.adc = 1;
}

void measure_input_reset_indicator(uint8_t index, uint32_t new_indicator)
{
    if (index == 0)
    {
        memset(&m_measure_data.counter[0], 0, sizeof(measure_input_cyber_counter_t));
        m_measure_data.counter[0].indicator = new_indicator;

        m_pulse_cnt_in_bkp[0].indicator = new_indicator;
        m_pulse_cnt_in_bkp[0].total_reverse = 0;
        m_pulse_cnt_in_bkp[0].total_reverse_index = 0;
        m_pulse_cnt_in_bkp[0].total_fwd_idx = new_indicator;
        m_pulse_cnt_in_bkp[0].total_forward = 0;
    }
#ifndef DTG01
    else
    {
        memset(&m_measure_data.counter[1], 0, sizeof(measure_input_cyber_counter_t));
        m_measure_data.counter[1].indicator = new_indicator;

        m_pulse_cnt_in_bkp[1].indicator = new_indicator;
        m_pulse_cnt_in_bkp[1].total_reverse = 0;
        m_pulse_cnt_in_bkp[1].total_reverse_index = 0;
        m_pulse_cnt_in_bkp[1].total_fwd_idx = new_indicator;
        m_pulse_cnt_in_bkp[1].total_forward = 0;
    }
#endif
    memcpy(m_pre_pulse_counter_in_bkp, 
            m_pulse_cnt_in_bkp, 
            sizeof(m_pulse_cnt_in_bkp));
    app_bkup_write_pulse_counter(&m_pulse_cnt_in_bkp[0]);
}

void measure_input_reset_k(uint8_t index, uint32_t new_k)
{
    if (index == 0)
    {
        m_measure_data.counter[0].k = new_k;
        m_pulse_cnt_in_bkp[0].k = new_k;
    }
#ifndef DTG01
    else
    {
        m_measure_data.counter[1].k = new_k;
        m_pulse_cnt_in_bkp[1].k = new_k;
    }
#endif

    memcpy(m_pre_pulse_counter_in_bkp, 
            m_pulse_cnt_in_bkp, 
            sizeof(m_pulse_cnt_in_bkp));
}

void measure_input_reset_counter(uint8_t index)
{
    if (index == 0)
    {
        m_pulse_cnt_in_bkp[0].real_counter = 0;
        m_pulse_cnt_in_bkp[0].reverse_counter = 0;
    }
#if defined(DTG02) || defined(DTG02V2) || defined(DTG02V3)
    else
    {
        m_pulse_cnt_in_bkp[1].real_counter = 0;
        m_pulse_cnt_in_bkp[1].reverse_counter = 0;
    }
#endif
    app_bkup_write_pulse_counter(&m_pulse_cnt_in_bkp[0]);
    memcpy(m_pre_pulse_counter_in_bkp, 
            m_pulse_cnt_in_bkp, 
            sizeof(m_pulse_cnt_in_bkp));
}

void measure_input_save_all_data_to_flash(void)
{
    static app_flash_log_data_t spi_flash_store_data;

    sys_ctx_t *ctx = sys_ctx();
    spi_flash_store_data.resend_to_server_flag = 0;

    for (uint32_t j = 0; j < MEASUREMENT_MAX_MSQ_IN_RAM; j++)
    {
        if (m_sensor_msq[j].state == MEASUREMENT_QUEUE_STATE_IDLE)
        {
            continue;
        }
        
        DEBUG_INFO("Store data in queue index %u to flash\r\n", j);
        // 4-20mA input
        for (uint32_t i = 0; i < APP_FLASH_NB_OFF_4_20MA_INPUT; i++)
        {
            spi_flash_store_data.input_4_20mA[i] = m_sensor_msq[j].input_4_20mA[i];
        }

        // Meter input
        for (uint32_t i = 0; i < APP_FLASH_NB_OF_METER_INPUT; i++)
        {
            memcpy(&spi_flash_store_data.counter[i], 
                    &m_sensor_msq[j].counter[i], 
                    sizeof(measure_input_cyber_counter_t));
        }

        // On/off
        spi_flash_store_data.on_off.name.input_on_off_0 = m_sensor_msq[j].input_on_off[0];
        spi_flash_store_data.on_off.name.input_on_off_1 = m_sensor_msq[j].input_on_off[1];

        if (ee_cfg->io_enable.name.output_4_20ma_enable)
        {
            spi_flash_store_data.output_4_20mA[0] = m_sensor_msq[j].output_4_20mA[0];
            spi_flash_store_data.output_4_20ma_enable = 1;
        }
        else
        {
            spi_flash_store_data.output_4_20mA[0] = 0.0f;
            spi_flash_store_data.output_4_20ma_enable = 0;
        }
        for (uint8_t  i = 0; i < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; i++)
        {
            spi_flash_store_data.input_pressure_analog[i] = m_sensor_msq [j].input_pressure_analog[i];   
        }

        spi_flash_store_data.timestamp = m_sensor_msq[j].measure_timestamp;
        spi_flash_store_data.valid_flag = APP_FLASH_VALID_DATA_KEY;
        spi_flash_store_data.resend_to_server_flag = 0;
        spi_flash_store_data.internal_battery_voltage = m_sensor_msq[j].internal_battery_voltage;
        spi_flash_store_data.vin_in_percent = m_sensor_msq[j].vin_in_percent;
        spi_flash_store_data.temp = m_sensor_msq[j].temperature;
        spi_flash_store_data.csq_percent = m_sensor_msq[j].csq_percent;

        spi_flash_store_data.analog_input[0] = m_sensor_msq[j].analog_input[0];
        spi_flash_store_data.analog_input[1] = m_sensor_msq[j].analog_input[1];

        // 485
        for (uint32_t nb_485_device = 0; 
            nb_485_device < HARDWARE_RS485_MAX_SLAVE_ON_BUS; 
            nb_485_device++)
        {
            memcpy(&spi_flash_store_data.rs485[nb_485_device],
                   &m_sensor_msq[j].rs485[nb_485_device],
                   sizeof(measure_input_modbus_register_t));
        }

        if (!ctx->peripheral_running.name.flash_running)
        {
            spi_init();
            app_spi_flash_wakeup(sys_flash());
            ctx->peripheral_running.name.flash_running = 1;
        }
        
        // Store to flash
        app_flash_log_measurement_data((app_flash_drv_t*)sys_flash(), &spi_flash_store_data);
        m_sensor_msq[j].state = MEASUREMENT_QUEUE_STATE_IDLE;
    }
    
    // Shutdown flash
    if (ctx->peripheral_running.name.flash_running)
    {
        app_spi_flash_shutdown(sys_flash());
        spi_deinit();
        ctx->peripheral_running.name.flash_running = 0;
    }
}

#if defined(DTG02V2) || defined(DTG02V3)
extern uint32_t last_time_monitor_vin_when_battery_low;
#endif

uint32_t estimate_measure_timestamp = 0;
void measure_input_task(void)
{
#if ENTER_JIG_TEST
      static uint32_t m_measure_tick = 0;
      if (sys_get_ms() - m_measure_tick >= 1000)
      {
          m_measure_tick = sys_get_ms();
      }
      else
      {
          return;
      }
#endif
      
    ee_cfg = app_eeprom_read_config_data();
    // Poll input counter state machine
    measure_input_pulse_counter_poll();
    
    // Get ADC result pointer
    sys_ctx_t *ctx = sys_ctx();

    uint32_t measure_interval_sec = ee_cfg->measure_interval_ms / 1000;
    adc_input_value_t *adc_retval = adc_get_input_result();
    uint32_t current_sec = app_rtc_get_counter();
    uint32_t now_ms;
    bool allow_save_min_max_value = false;
    uint32_t current_counter;
    uint32_t timestamp_send_data_to_internet;
    uint32_t diff;
    bool found_empty_buffer = false;
    
    if (estimate_measure_timestamp == 0)
    {
        estimate_measure_timestamp = (measure_interval_sec * (current_sec / measure_interval_sec + 1));
    }

    if (ctx->status.is_enter_test_mode)
    {
        HARDWARE_ENABLE_INPUT_4_20MA_POWER(1);
        estimate_measure_timestamp = current_sec;
    }

    if (adc_retval->internal_battery_voltage > VBAT_DETECT_HIGH_MV)
    {
        ctx->peripheral_running.name.high_bat_detect = 1;
    }
    else
    {
        ctx->peripheral_running.name.high_bat_detect = 0;
    }

    // Get input and output on/off value
    m_measure_data.input_on_off[0] = LL_GPIO_IsInputPinSet(OPTOIN1_GPIO_Port, OPTOIN1_Pin) ? 1 : 0;
    m_measure_data.input_on_off[1] = LL_GPIO_IsInputPinSet(OPTOIN2_GPIO_Port, OPTOIN2_Pin) ? 1 : 0;
    
    // Check meter input circuit status
    m_measure_data.counter[HARDWARE_MEASURE_INPUT_PORT_2].cir_break = LL_GPIO_IsInputPinSet(CIRIN2_GPIO_Port, CIRIN2_Pin) 
                                                            && (ee_cfg->cyber_config[1].mode != APP_EEPROM_METER_MODE_DISABLE);
    m_measure_data.counter[HARDWARE_MEASURE_INPUT_PORT_1].cir_break = LL_GPIO_IsInputPinSet(CIRIN1_GPIO_Port, CIRIN1_Pin) 
                                                            && (ee_cfg->cyber_config[0].mode != APP_EEPROM_METER_MODE_DISABLE);

    // Get Vbat
    m_measure_data.vin_in_percent = adc_retval->vin_convert_to_percent;
    m_measure_data.internal_battery_voltage = adc_retval->internal_battery_voltage;

    // Vtemp
    if (adc_retval->temp_is_valid)
    {
        m_measure_data.temperature_error = 0;
        m_measure_data.temperature = adc_retval->temp;
    }
    else
    {
        m_measure_data.temperature_error = 1;
    }
    
    if (m_allow_blink_indicator_when_pulse && sys_get_ms() > 30000)
    {
        m_allow_blink_indicator_when_pulse = false;
    }

    // If timeout is over -> measure data
    if ((MEASUREMENT_CYCLE_EXPIRE() && (delay_turn_on_power_for_external_sensor == 0)) 
        == false)
    {
        return;
    }
    
    // If 4-20mA sensor need to measure ->> Power on 4-20mA sensor
    if (ee_cfg->io_enable.name.input_4_20ma_0_enable 
        || ee_cfg->io_enable.name.input_4_20ma_1_enable)
    {
        
        if (!HARDWARE_INPUT_POWER_4_20_MA_IS_ENABLE())
        {
            HARDWARE_ENABLE_INPUT_4_20MA_POWER(1);
            delay_turn_on_power_for_external_sensor = MEASURE_INTPUT_WAIT_4_20MA_SENSOR_POWER_ON_MS;
        }
    }
    
    if (ee_cfg->io_enable.name.input_pressure_sensor_0)
    {
        
        if (HARDWARE_PRESSURE_0_OUTPUT_DISABLE())
        {
            HARDWARE_PRESSURE_0_OUTPUT_POWER(1);
            delay_turn_on_power_for_external_sensor = MEASURE_INTPUT_WAIT_4_20MA_SENSOR_POWER_ON_MS;
        }
    }
    
    if (ee_cfg->io_enable.name.input_pressure_sensor_1)
    {
        
        if (HARDWARE_PRESSURE_1_OUTPUT_DISABLE())
        {
            HARDWARE_PRESSURE_1_OUTPUT_POWER(1);
            delay_turn_on_power_for_external_sensor = MEASURE_INTPUT_WAIT_4_20MA_SENSOR_POWER_ON_MS;
        }
    }
    
    // Waiting for external sensor power on
    if (delay_turn_on_power_for_external_sensor)
    {            
        ctx->peripheral_running.name.wait_for_4_20ma_power_off = 1;
        return;
    }
    ctx->peripheral_running.name.wait_for_4_20ma_power_off = 0;
    // Estimate next time measure data
    estimate_measure_timestamp = (measure_interval_sec * (current_sec / measure_interval_sec + 1));
    uint32_t next_hour = estimate_measure_timestamp / 3600;
    next_hour %= 24;
    uint32_t next_min = (estimate_measure_timestamp / 60);
    next_min %= 60;

#if 1       // Debug next wakeup time
    rtc_date_time_t now;
    app_rtc_get_time(&now);
    if (sys_ctx()->status.is_enter_test_mode == 0)
    {
        DEBUG_WARN("[%02u:%02u] Next measurement time is %02u:%02u\r\n", 
                now.hour, now.minute, next_hour, next_min);
    }
#endif

    // Process rs485
    process_rs485(&m_measure_data.rs485[0]);
    
    // ADC conversion
    last_time_monitor_vin_when_battery_low = 0;
    adc_start();

    if (m_this_is_the_first_time)
    {
        m_this_is_the_first_time = false;
    }
    m_this_is_the_first_time = false;
    m_number_of_adc_conversion = 0;

    // Stop adc
    adc_stop();

    // Put data to msq
    adc_retval = adc_get_input_result();
#if ENTER_JIG_TEST == 0
    // O ban DTG02V4 thi chan output on/off la chan cap nguon cho cam bien 4-20mA
    HARDWARE_PRESSURE_0_OUTPUT_POWER(0);
    HARDWARE_PRESSURE_1_OUTPUT_POWER(0);
#endif
    static bool charge_for_the_first_time = true;
    if (adc_retval->internal_battery_voltage > 4150)      // battery full : 4150 mv
    {
        if (sys_ctx()->status.need_charge)
        {
            DEBUG_VERBOSE("Discharge\r\n");
            sys_ctx()->status.need_charge = 0;
        }
        
#if ENTER_JIG_TEST == 0
         // neu pin day thi ko sac nua      
//        LL_GPIO_ResetOutputPin(CHARGE_EN_GPIO_Port, CHARGE_EN_Pin);
        // #warning "o phien ban nay thi luc nao cung cho phep sac, ngat sac do IC tu quan li"
        // Chi tat nguon 5V khi GSM sleep && (out 4-20mA bi tat)
        if (gsm_data_layer_is_module_sleeping()
            && (ee_cfg->io_enable.name.output_4_20ma_enable == 0 || tim_is_pwm_active() == false))
        {
            HARDWARE_ENABLE_SYS_5V(0);
        }
#else
        HARDWARE_ENABLE_SYS_5V(1);
#endif
    }
    // Battery low, charge now
    else if (ADC_IS_VIN_DETECT() 
            && (adc_retval->internal_battery_voltage < 3800 || charge_for_the_first_time)) 
    {
        charge_for_the_first_time = false;      // Lan dau tien power on thi sac luon cho mat
        if (sys_ctx()->status.need_charge == 0)
        {
            DEBUG_VERBOSE("Charge now\r\n");
        }
        sys_ctx()->status.need_charge = 1;
//        LL_GPIO_ResetOutputPin(CHARGE_EN_GPIO_Port, CHARGE_EN_Pin);
        
        
        // Enable 5V for charger and GSM power
        HARDWARE_ENABLE_SYS_5V(1);
    }

    if (ctx->status.is_enter_test_mode)
    {
        goto end;
    }
    
    // Set timestamp
    m_measure_data.measure_timestamp = app_rtc_get_counter();
    // Internal Vbat 4v2
    m_measure_data.internal_battery_voltage = adc_retval->internal_battery_voltage;
    // Vin : percent
    m_measure_data.vin_in_percent = adc_retval->vin_convert_to_percent;
    
    // Analog input
    for (uint32_t analog_input = 0; analog_input < HARDWARE_NUMBER_OF_ANALOG_INPUT; analog_input++)
    {
        m_measure_data.analog_input[analog_input] = adc_retval->analog_input_io[analog_input];
    }
    
    // Pressure analog input
    for (uint32_t pressure_input = 0; pressure_input < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; pressure_input++)
    {
        m_measure_data.input_pressure_analog[pressure_input] = adc_retval->input_pressure_analog[pressure_input];
    }
    
    m_measure_data.vin_mv = adc_retval->vin_voltage;
//                app_bkup_read_pulse_counter(&m_measure_data.counter[0]);

    // CSQ
    m_measure_data.csq_percent = gsm_get_csq_in_percent();
    
    // Input 4-20mA
    // If input is disable, we set value of input 4-20mA to zero
    if (ee_cfg->io_enable.name.input_4_20ma_0_enable)
    {
        m_measure_data.input_4_20mA[0] = adc_retval->in_4_20ma_in[0];
    }
    else
    {
        m_measure_data.input_4_20mA[0] = 0.0f;
    }

    if (ee_cfg->io_enable.name.input_4_20ma_1_enable)
    {
        m_measure_data.input_4_20mA[1] = adc_retval->in_4_20ma_in[1];
    }
    else
    {
        m_measure_data.input_4_20mA[1] = 0.0f;
    }

    // Output 4-20mA
    if (ee_cfg->io_enable.name.output_4_20ma_enable)
    {
        m_measure_data.output_4_20mA[0] = ee_cfg->output_4_20ma;
        m_measure_data.output_4_20ma_enable = 1;
    }
    else
    {
        m_measure_data.output_4_20mA[0] = 0.0f;
        m_measure_data.output_4_20ma_enable = 0;
    }

    now_ms = sys_get_ms();

    // Doan nay dai vai, ngay xua vua lam vua phat sinh tai hien truong
    // Nen ai ma doc code thi chiu kho nhe :D
    // Calculate avg flow speed between 2 times measurement data
    diff = (uint32_t)(now_ms - m_last_time_measure_data);
    diff /= 1000; // Convert ms to s
    static uint32_t m_last_time_measure_data_cycle_send_web;

    // Copy backup data into current measure data
    allow_save_min_max_value = false;
    current_counter = app_rtc_get_counter();
    timestamp_send_data_to_internet = gsm_data_layer_estimate_wakeup_timestamp();

    // Quet tat ca dau vao input
    for (uint32_t cnt_idx = 0; cnt_idx < MEASURE_NUMBER_OF_WATER_METER_INPUT; cnt_idx++)
    {
        backup_pulse_data_t *pulse_ptr = &m_pulse_cnt_in_bkp[cnt_idx];
        backup_pulse_data_t *pre_pulse_ptr = &m_pre_pulse_counter_in_bkp[cnt_idx];
        
        // Ensure no div by zero
        if (diff && pulse_ptr->k)
        {
            // Forward direction
            // speed = ((counter  forward - pre_counter forward)/k)/diff (hour)
            pulse_ptr->flow_speed_fwd_agv_cycle_wakeup = (float)(pulse_ptr->fw_flow 
                                                                - pre_pulse_ptr->fw_flow);
            pulse_ptr->flow_speed_fwd_agv_cycle_wakeup /= pulse_ptr->k;

            //                        DEBUG_VERBOSE("Diff forward%u - %.2f\r\n", cnt_idx, pulse_ptr->flow_speed_fwd_agv_cycle_wakeup);

            // Neu ma cho phep tinh toc do nuoc thi moi tinh
            if (ee_cfg->io_enable.name.calculate_flow_speed)
            {
                pulse_ptr->flow_speed_fwd_agv_cycle_wakeup *= 3600.0f / (float)diff;
            }
//                        pulse_ptr->flow_avg_cycle_send_web.fw_flow_sum += pulse_ptr->flow_speed_fwd_agv_cycle_wakeup;

            // Tuong tu cho chieu nguoc kim dong ho
            // Reverse direction
            pulse_ptr->flow_speed_reverse_agv_cycle_wakeup = (float)(pulse_ptr->reverse_flow 
                                                                    - pre_pulse_ptr->reverse_flow);
            pulse_ptr->flow_speed_reverse_agv_cycle_wakeup /= pulse_ptr->k;
//                        pulse_ptr->flow_avg_cycle_send_web.reverse_flow_sum += pulse_ptr->flow_speed_reverse_agv_cycle_wakeup;

//                        DEBUG_VERBOSE("Diff reserve%u - %.2f\r\n", cnt_idx, pulse_ptr->flow_speed_reverse_agv_cycle_wakeup);
            if (ee_cfg->io_enable.name.calculate_flow_speed)
            {
                pulse_ptr->flow_speed_reverse_agv_cycle_wakeup *= 3600.0f / (float)diff;
            }

            // tinh gia tri min-max trong 1 cycle send web
            // min-max of forward/reserve direction flow speed
            // If value is invalid =>> Initialize new value
            // If min max value is invalid =>> Copy the first data
            if (fw_flow_min_in_cycle_send_web[cnt_idx] == MEASURE_INPUT_FLOW_INVALID_VALUE 
                || fw_flow_max_in_cycle_send_web[cnt_idx] == MEASURE_INPUT_FLOW_INVALID_VALUE)
            {
                fw_flow_min_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_fwd_agv_cycle_wakeup;
                fw_flow_max_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_fwd_agv_cycle_wakeup;
                //                            DEBUG_VERBOSE("Set MinFwFlow%u - %.2f\r\n", cnt_idx, fw_flow_max_in_cycle_send_web[cnt_idx]);
                //                            DEBUG_VERBOSE("Set MaxFwFlow%u - %.2f\r\n", cnt_idx, fw_flow_min_in_cycle_send_web[cnt_idx]);
            }

            if (reverse_flow_min_in_cycle_send_web[cnt_idx] == MEASURE_INPUT_FLOW_INVALID_VALUE 
                || reverse_flow_max_in_cycle_send_web[cnt_idx] == MEASURE_INPUT_FLOW_INVALID_VALUE)
            {
                reverse_flow_min_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_reverse_agv_cycle_wakeup;
                reverse_flow_max_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_reverse_agv_cycle_wakeup;
                //                            DEBUG_VERBOSE("Set MaxRsvFlow%u - %.2f\r\n", cnt_idx, .reverse_flow_max_in_cycle_send_web[cnt_idx]);
                //                            DEBUG_VERBOSE("Set MixRsvFlow%u - %.2f\r\n", cnt_idx, .reverse_flow_min_in_cycle_send_web[cnt_idx]);
            }

            // Compare old value and new value to determite new min-max value
            // 1.1 Forward direction min value
            if (fw_flow_min_in_cycle_send_web[cnt_idx] > pulse_ptr->flow_speed_fwd_agv_cycle_wakeup)
            {
                fw_flow_min_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_fwd_agv_cycle_wakeup;
            }
            //                        DEBUG_VERBOSE("[Temporaty] MinFlow%u - %.2f\r\n", cnt_idx, fw_flow_min_in_cycle_send_web[cnt_idx]);

            // 1.2 Forward direction max value
            if (fw_flow_max_in_cycle_send_web[cnt_idx] < pulse_ptr->flow_speed_fwd_agv_cycle_wakeup)
            {
                fw_flow_max_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_fwd_agv_cycle_wakeup;
            }
            //                        DEBUG_VERBOSE("[Temporaty] MaxFlow%u - %.2f\r\n", cnt_idx, fw_flow_max_in_cycle_send_web[cnt_idx]);

            // 2.1 reserve direction min value
            if (reverse_flow_min_in_cycle_send_web[cnt_idx] > pulse_ptr->flow_speed_reverse_agv_cycle_wakeup)
            {
                reverse_flow_min_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_reverse_agv_cycle_wakeup;
            }

            // 2.2 reserve direction max value
            if (reverse_flow_max_in_cycle_send_web[cnt_idx] < pulse_ptr->flow_speed_reverse_agv_cycle_wakeup)
            {
                reverse_flow_max_in_cycle_send_web[cnt_idx] = pulse_ptr->flow_speed_reverse_agv_cycle_wakeup;
            }

            allow_save_min_max_value = true;
        }
        else // invalid value, should never get there
        {
            DEBUG_ERROR("Shoud never get there\r\n");
            pulse_ptr->flow_speed_fwd_agv_cycle_wakeup = 0;
            pulse_ptr->flow_speed_reverse_agv_cycle_wakeup = 0;
        }
        
        // If we need to save min-max value in 1 cycle send web
        if (allow_save_min_max_value)
        {
            // If current counter >= wakeup counter =>> Send min max to server
            // Else we dont need to send min max data to server
            if (current_counter >= timestamp_send_data_to_internet)
            {
                float diff_cycle_send_web = (now_ms - m_last_time_measure_data_cycle_send_web);
                diff_cycle_send_web /= 1000; // convert ms to s
                if (diff_cycle_send_web == 0)
                {
                    diff_cycle_send_web = 1;
                }
                float k_cycle_send_web = (3600.0f / diff_cycle_send_web);
                // Pulse counter
                pulse_ptr->flow_avg_cycle_send_web.fw_flow_max = fw_flow_max_in_cycle_send_web[cnt_idx];
                pulse_ptr->flow_avg_cycle_send_web.fw_flow_min = fw_flow_min_in_cycle_send_web[cnt_idx];
                pulse_ptr->flow_avg_cycle_send_web.reverse_flow_max = reverse_flow_max_in_cycle_send_web[cnt_idx];
                pulse_ptr->flow_avg_cycle_send_web.reverse_flow_min = reverse_flow_min_in_cycle_send_web[cnt_idx];
                // Sum counter
                pulse_ptr->flow_avg_cycle_send_web.fw_flow_sum = pulse_ptr->total_forward
                                                                - pulse_ptr->flow_avg_cycle_send_web.fw_flow_first_counter;
                pulse_ptr->flow_avg_cycle_send_web.fw_flow_sum /= pulse_ptr->k;
                pulse_ptr->flow_avg_cycle_send_web.fw_flow_sum *= k_cycle_send_web;

                pulse_ptr->flow_avg_cycle_send_web.fw_flow_first_counter = pulse_ptr->total_forward;

                pulse_ptr->flow_avg_cycle_send_web.reverse_flow_sum = pulse_ptr->total_reverse 
                                                                    - pulse_ptr->flow_avg_cycle_send_web.reverse_flow_first_counter;
                pulse_ptr->flow_avg_cycle_send_web.reverse_flow_first_counter = pulse_ptr->total_reverse;

                pulse_ptr->flow_avg_cycle_send_web.reverse_flow_sum /= pulse_ptr->k;
                pulse_ptr->flow_avg_cycle_send_web.reverse_flow_sum *= k_cycle_send_web;
                pulse_ptr->flow_avg_cycle_send_web.valid = 1;

//                            DEBUG_VERBOSE("Cycle send web counter %u: Min max fw flow %.2f %.2f\r\n", cnt_idx,
//                                       pulse_ptr->flow_avg_cycle_send_web.fw_flow_max,
//                                       pulse_ptr->flow_avg_cycle_send_web.fw_flow_min);

//                            DEBUG_VERBOSE("Cycle send web counter %u: Min max resv flow %.2f %.2f\r\n", cnt_idx,
//                                       pulse_ptr->flow_avg_cycle_send_web.reverse_flow_max,
//                                       pulse_ptr->flow_avg_cycle_send_web.reverse_flow_min);

                // Clean min max value
                fw_flow_max_in_cycle_send_web[cnt_idx] = MEASURE_INPUT_FLOW_INVALID_VALUE;
                fw_flow_min_in_cycle_send_web[cnt_idx] = MEASURE_INPUT_FLOW_INVALID_VALUE;
                reverse_flow_max_in_cycle_send_web[cnt_idx] = MEASURE_INPUT_FLOW_INVALID_VALUE;
                reverse_flow_min_in_cycle_send_web[cnt_idx] = MEASURE_INPUT_FLOW_INVALID_VALUE;
            }
            else // Mark invalid, we dont need to send data to server
            {
                pulse_ptr->flow_avg_cycle_send_web.valid = 0;
            }
        }

        // Copy current value to prev value
        memcpy(&m_pre_pulse_counter_in_bkp[cnt_idx], &m_pulse_cnt_in_bkp[cnt_idx], sizeof(backup_pulse_data_t));

        // Copy current measure counter to message queue variable
        memcpy(&m_measure_data.counter[cnt_idx], &m_pulse_cnt_in_bkp[cnt_idx], sizeof(measure_input_cyber_counter_t));
        
         // counter ext interrupt maybe happen, disable for isr safe
        __disable_irq();
        pulse_ptr->fw_flow = 0;
        pre_pulse_ptr->fw_flow = 0;
        pulse_ptr->reverse_flow = 0;
        pre_pulse_ptr->reverse_flow = 0;

        // Neu ma can gui du lieu len web thi moi gui
        if (pulse_ptr->flow_avg_cycle_send_web.valid)
        {
            // Clean all data after 1 cycle send web, next time we will measure again
            pulse_ptr->flow_avg_cycle_send_web.fw_flow_max = MEASURE_INPUT_FLOW_INVALID_VALUE;
            pulse_ptr->flow_avg_cycle_send_web.fw_flow_min = MEASURE_INPUT_FLOW_INVALID_VALUE;
            pulse_ptr->flow_avg_cycle_send_web.reverse_flow_max = MEASURE_INPUT_FLOW_INVALID_VALUE;
            pulse_ptr->flow_avg_cycle_send_web.reverse_flow_min = MEASURE_INPUT_FLOW_INVALID_VALUE;
            pulse_ptr->flow_avg_cycle_send_web.fw_flow_sum = 0.0f;
            pulse_ptr->flow_avg_cycle_send_web.reverse_flow_sum = 0.0f;
        }
        __enable_irq();
        // Mark as invalid data
        pulse_ptr->flow_avg_cycle_send_web.valid = 0;

//                    DEBUG_VERBOSE("PWM%u %u\r\n", cnt_idx, m_measure_data.counter[cnt_idx].real_counter);
    }

    // If current counter >= wakeup counter =>> Send min max value to server
    // Else we dont need to send min max data to server
    if (current_counter >= timestamp_send_data_to_internet)
    {
        // Store 485 min - max value
        // 485
        for (uint32_t slave_index = 0; slave_index < HARDWARE_RS485_MAX_SLAVE_ON_BUS; slave_index++)
        {
            float diff_cycle_send_web = now_ms - m_last_time_measure_data_cycle_send_web;
            diff_cycle_send_web /= 1000; // convert ms to sec
            if (diff_cycle_send_web == 0.0f)       // avoid div by zero
            {
                diff_cycle_send_web = 1.0f;
            }
            
            // float k_hour = 3600.0f / diff_cycle_send_web;


            // Reset value to default
            //                        mb_rvs_flow_index[slave_index] = INPUT_485_INVALID_FLOAT_VALUE;
            //                        mb_fw_flow_index[slave_index] = INPUT_485_INVALID_FLOAT_VALUE;
            //                        mb_net_totalizer[slave_index] = INPUT_485_INVALID_FLOAT_VALUE;
        }

        m_last_time_measure_data_cycle_send_web = now_ms;
    }
    
    m_measure_data.temperature = adc_retval->temp;
    m_measure_data.state = MEASUREMENT_QUEUE_STATE_PENDING;
#if ALWAYS_SAVE_DATA_TO_FLASH == 0
        bool scan = true;
        while (scan)
        {
            // Scan for empty buffer
            bool queue_full = true;
            for (uint32_t i = 0; i < MEASUREMENT_MAX_MSQ_IN_RAM; i++)
            {
                if (m_sensor_msq[i].state == MEASUREMENT_QUEUE_STATE_IDLE)
                {
                    memcpy(&m_sensor_msq[i], &m_measure_data, sizeof(measure_input_peripheral_data_t));
                    queue_full = false;
                    DEBUG_INFO("Puts new msg to sensor queue\r\n");
                    scan = false;
                    break;
                }
            }

            if (queue_full)
            {
                DEBUG_ERROR("Message queue full\r\n");
                measure_input_save_all_data_to_flash();
            }
        }
#else
push_to_queue:
    // Scan available buffer for new data
    for (uint32_t i = 0; i < MEASUREMENT_MAX_MSQ_IN_RAM; i++)
    {
        if (m_sensor_msq[i].state == MEASUREMENT_QUEUE_STATE_IDLE)
        {
            DEBUG_VERBOSE("Store data to flash with reserve index %u %u, ts %u\r\n",
                          m_sensor_msq[i].counter[0].total_reverse_index,
                          m_sensor_msq[i].counter[1].total_reverse_index,
                          m_sensor_msq[i].measure_timestamp);
            memcpy(&m_sensor_msq[i], &m_measure_data, sizeof(measure_input_peripheral_data_t));
            // measure_input_save_all_data_to_flash();
            found_empty_buffer = true;
            break;
        }
    }
     // No empty queue found ->> save all data to flash and push data to queue again
    if (found_empty_buffer == false)
    {
        measure_input_save_all_data_to_flash();
        goto push_to_queue;
    }
#endif

end:
    m_last_time_measure_data = sys_get_ms();
    ctx->peripheral_running.name.adc = 0;
}

void measure_input_initialize(void)
{
    m_last_time_measure_data = sys_get_ms();
    app_eeprom_config_data_t *ee_cfg = app_eeprom_read_config_data();
    memset(m_pulse_cnt_in_bkp, 0, sizeof(m_pulse_cnt_in_bkp));
    
    for (int i = 0; i < APP_EEPROM_METER_MODE_MAX_ELEMENT; i++)
    {
        m_pulse_cnt_in_bkp[i].k = ee_cfg->cyber_config[i].k;
        m_pulse_cnt_in_bkp[i].indicator = ee_cfg->cyber_config[i].offset;
        m_measure_data.counter[i].k = ee_cfg->cyber_config[i].k;
        m_measure_data.counter[i].indicator = ee_cfg->cyber_config[i].offset;

        // Mark forward-reserve value is invalid
        fw_flow_min_in_cycle_send_web[i] = MEASURE_INPUT_FLOW_INVALID_VALUE;
        fw_flow_max_in_cycle_send_web[i] = MEASURE_INPUT_FLOW_INVALID_VALUE;
        reverse_flow_min_in_cycle_send_web[i] = MEASURE_INPUT_FLOW_INVALID_VALUE;
        reverse_flow_max_in_cycle_send_web[i] = MEASURE_INPUT_FLOW_INVALID_VALUE;
    }

    // Reset 485 register min - max
    for (uint32_t i = 0; i < HARDWARE_RS485_MAX_SLAVE_ON_BUS; i++)
    {
        mb_fw_flow_index[i] = INPUT_485_INVALID_FLOAT_VALUE;
        mb_rvs_flow_index[i] = INPUT_485_INVALID_FLOAT_VALUE;
        mb_net_totalizer_fw_flow_index[i] = INPUT_485_INVALID_FLOAT_VALUE;
        mb_net_totalizer_reverse_flow_index[i] = INPUT_485_INVALID_FLOAT_VALUE;
        mb_net_totalizer[i] = INPUT_485_INVALID_FLOAT_VALUE;
    }

    /* Doc gia tri do tu bo nho backup, neu gia tri tu BKP < flash -> lay theo gia tri flash
     * -> Case: Mat dien nguon -> mat du lieu trong RTC backup register
     */

    app_bkup_read_pulse_counter(&m_pulse_cnt_in_bkp[0]);

    for (uint32_t i = 0; i < MEASUREMENT_MAX_MSQ_IN_RAM; i++)
    {
        m_sensor_msq[i].state = MEASUREMENT_QUEUE_STATE_IDLE;
    }

    app_flash_log_data_t last_data;

    // Wakeup flash
    if (!sys_ctx()->peripheral_running.name.flash_running)
    {
        app_spi_flash_wakeup(sys_flash());
        spi_init();
        sys_ctx()->peripheral_running.name.flash_running = 1;
    }
        
    bool flash_has_data = app_flash_memory_log_data_is_available((app_flash_drv_t*)sys_flash());
    

    if (flash_has_data)
    {
        uint8_t *mem;
        uint16_t size = 0;
        uint32_t crc = 1234;        // dummy value
        
        app_flash_mem_error_t err = app_flash_log_the_get_last_message((app_flash_drv_t*)sys_flash(), &mem, &size);
        
        // Calculate CRC
        if (mem && size)
        {
            crc = utilities_calculate_crc32(mem, sizeof(app_flash_log_data_t) - HARDWARE_CRC32_SIZE);
        }
        else
        {
            size = 0;
        }
        
        if (size 
            && err == APP_SPI_FLASH_MEM_OK
            && crc == ((app_flash_log_data_t*)mem)->crc)
        {
            memcpy(&last_data, mem, sizeof(app_flash_log_data_t));
        }
        else
        {
            flash_has_data = false;
        }
    }

    if (flash_has_data)
    {
        bool save = false;
        if (last_data.counter[0].real_counter > m_pulse_cnt_in_bkp[0].real_counter)
        {
            m_pulse_cnt_in_bkp[0].real_counter = last_data.counter[0].real_counter;
            save = true;
        }

        if (last_data.counter[0].reverse_counter > m_pulse_cnt_in_bkp[0].reverse_counter)
        {
            m_pulse_cnt_in_bkp[0].reverse_counter = last_data.counter[0].reverse_counter;
            save = true;
        }

        //        if (save)
        //        {
        //            DEBUG_INFO("Restore value from flash\r\n");
        //        }

        m_pulse_cnt_in_bkp[0].total_forward = last_data.counter[0].total_forward;
        m_pulse_cnt_in_bkp[0].total_fwd_idx = last_data.counter[0].total_fwd_idx;
        m_pulse_cnt_in_bkp[0].total_reverse = last_data.counter[0].total_reverse;
        m_pulse_cnt_in_bkp[0].total_reverse_index = last_data.counter[0].total_reverse_index;
        m_pulse_cnt_in_bkp[0].flow_avg_cycle_send_web.fw_flow_first_counter = m_pulse_cnt_in_bkp[0].total_fwd_idx;
        m_pulse_cnt_in_bkp[0].flow_avg_cycle_send_web.reverse_flow_first_counter = m_pulse_cnt_in_bkp[0].total_reverse_index;

        if (last_data.counter[1].real_counter > m_pulse_cnt_in_bkp[1].real_counter)
        {
            m_pulse_cnt_in_bkp[1].real_counter = last_data.counter[1].real_counter;
            save = true;
        }

        if (last_data.counter[1].reverse_counter > m_pulse_cnt_in_bkp[1].reverse_counter)
        {
            m_pulse_cnt_in_bkp[1].reverse_counter = last_data.counter[1].reverse_counter;
            save = true;
        }

        if (save)
        {
            //            DEBUG_VERBOSE("Save new data from flash\r\n");
            app_bkup_write_pulse_counter(&m_pulse_cnt_in_bkp[0]);
        }

        m_pulse_cnt_in_bkp[1].total_forward = last_data.counter[1].total_forward;
        m_pulse_cnt_in_bkp[1].total_fwd_idx = last_data.counter[1].total_fwd_idx;
        m_pulse_cnt_in_bkp[1].total_reverse = last_data.counter[1].total_reverse;
        m_pulse_cnt_in_bkp[1].total_reverse_index = last_data.counter[1].total_reverse_index;
        m_pulse_cnt_in_bkp[1].flow_avg_cycle_send_web.fw_flow_first_counter = m_pulse_cnt_in_bkp[1].total_fwd_idx;
        m_pulse_cnt_in_bkp[1].flow_avg_cycle_send_web.reverse_flow_first_counter = m_pulse_cnt_in_bkp[1].total_reverse_index;

//        DEBUG_INFO("Pulse counter in BKP: %u-%u, %u-%u\r\n",
//                   m_pulse_cnt_in_bkp[0].real_counter, m_pulse_cnt_in_bkp[0].reverse_counter,
//                   m_pulse_cnt_in_bkp[1].real_counter, m_pulse_cnt_in_bkp[1].reverse_counter);
    }
    memcpy(m_pre_pulse_counter_in_bkp, m_pulse_cnt_in_bkp, sizeof(m_pulse_cnt_in_bkp));
    m_this_is_the_first_time = true;
    
    // Shutdown flash
    if (sys_ctx()->peripheral_running.name.flash_running)
    {
        app_spi_flash_shutdown(sys_flash());
        spi_deinit();
        sys_ctx()->peripheral_running.name.flash_running = 0;
    }
}

void measure_input_rs485_uart_handler(uint8_t data)
{
    modbus_memory_serial_rx(data);
}


// Calculate time in MS between 2 pulse counter
static inline uint32_t get_diff_ms_between_pulse(measure_input_timestamp_t *begin, 
                                                measure_input_timestamp_t *end, 
                                                uint8_t isr_type)
{
    uint32_t ms;
    uint32_t prescaler = LL_RTC_GetSynchPrescaler(RTC);
    if (isr_type == MEASURE_INPUT_NEW_DATA_TYPE_PWM_PIN)
    {
        ms = (end->counter_pwm - begin->counter_pwm) * ((uint32_t)1000);

        end->subsecond_pwm = 1000 * (prescaler - end->subsecond_pwm) / (prescaler + 1);
        begin->subsecond_pwm = 1000 * (prescaler - begin->subsecond_pwm) / (prescaler + 1);

        ms += end->subsecond_pwm;
        ms -= begin->subsecond_pwm;
    }
    else
    {
        ms = (end->counter_dir - begin->counter_dir) * ((uint32_t)1000);

        end->subsecond_dir = 1000 * (prescaler - end->subsecond_dir) / (prescaler + 1);
        begin->subsecond_dir = 1000 * (prescaler - begin->subsecond_dir) / (prescaler + 1);

        ms += end->subsecond_dir;
        ms -= begin->subsecond_dir;
    }
    return ms;
}

// volatile uint32_t m_last_direction_is_forward = 1;
void measure_input_pulse_irq(measure_input_cyber_meter_t *input)
{
    if (m_allow_blink_indicator_when_pulse)
    {
        HARDWARE_LED1_CTRL(1);
        sys_keep_led_on(3);
    }
    __disable_irq();
    
    // If data come from PWM pin
    if (input->name.isr_type == MEASURE_INPUT_NEW_DATA_TYPE_PWM_PIN)
    {
        m_end_pulse_timestamp[input->name.port].counter_pwm = app_rtc_get_counter();
        m_end_pulse_timestamp[input->name.port].subsecond_pwm = app_rtc_get_subsecond_counter();
        
        // Calculate period of pulse counter in ms
        m_pull_diff[input->name.port] = get_diff_ms_between_pulse(&m_begin_pulse_timestamp[input->name.port],
                                                             &m_end_pulse_timestamp[input->name.port],
                                                             MEASURE_INPUT_NEW_DATA_TYPE_PWM_PIN);
        m_begin_pulse_timestamp[input->name.port].counter_dir = m_end_pulse_timestamp[input->name.port].counter_dir;
        m_begin_pulse_timestamp[input->name.port].subsecond_dir = m_end_pulse_timestamp[input->name.port].subsecond_dir;
        
        // Filter 
        if (m_pull_diff[input->name.port] >= PULSE_MINMUM_WITDH_MS)
        {
            // Increase total forward counter

            m_is_pulse_trigger = 1;
            if (ee_cfg->cyber_config[input->name.port].mode == APP_EEPROM_METER_MODE_PWM_PLUS_DIR_MIN)
            {
                // If direction current level = direction on forward level
                if (ee_cfg->io_enable.name.cyber_direction_level == input->name.dir_level)
                {
                    DEBUG_VERBOSE("[PWM%u]+++ \r\n", input->name.port);
                    m_pulse_cnt_in_bkp[input->name.port].total_forward++;
                    m_pulse_cnt_in_bkp[input->name.port].fw_flow++;
                    m_pulse_cnt_in_bkp[input->name.port].total_fwd_idx = m_pulse_cnt_in_bkp[input->name.port].total_forward / m_pulse_cnt_in_bkp[input->name.port].k 
                                                                            + m_pulse_cnt_in_bkp[input->name.port].indicator;
                    m_pulse_cnt_in_bkp[input->name.port].real_counter++;
                }
                else // Reverser counter
                {
                    DEBUG_VERBOSE("[PWM]---\r\n");
                    m_pulse_cnt_in_bkp[input->name.port].reverse_flow++;
                    m_pulse_cnt_in_bkp[input->name.port].real_counter--;
                    m_pulse_cnt_in_bkp[input->name.port].total_reverse++;
                    m_pulse_cnt_in_bkp[input->name.port].total_reverse_index = m_pulse_cnt_in_bkp[input->name.port].total_reverse 
                                                                                / m_pulse_cnt_in_bkp[input->name.port].k;
                }
                m_pulse_cnt_in_bkp[input->name.port].reverse_counter = 0;
            }
            else if (ee_cfg->cyber_config[input->name.port].mode == APP_EEPROM_METER_MODE_ONLY_PWM)
            {
                DEBUG_VERBOSE("[PWM]+++++++\r\n");
                m_pulse_cnt_in_bkp[input->name.port].total_forward++;
                m_pulse_cnt_in_bkp[input->name.port].fw_flow++;
                // Calculate total forward index
                m_pulse_cnt_in_bkp[input->name.port].total_fwd_idx = m_pulse_cnt_in_bkp[input->name.port].total_forward 
                                                                        / m_pulse_cnt_in_bkp[input->name.port].k + m_pulse_cnt_in_bkp[input->name.port].indicator;

                m_pulse_cnt_in_bkp[input->name.port].real_counter++;
                m_pulse_cnt_in_bkp[input->name.port].reverse_counter = 0;
            }
            else if (ee_cfg->cyber_config[input->name.port].mode == APP_EEPROM_METER_MODE_DISABLE)
            {
                m_pulse_cnt_in_bkp[input->name.port].real_counter = 0;
                m_pulse_cnt_in_bkp[input->name.port].reverse_counter = 0;
                m_pulse_cnt_in_bkp[input->name.port].total_fwd_idx = 0;
                m_pulse_cnt_in_bkp[input->name.port].total_forward = 0;
            }
            else if (ee_cfg->cyber_config[input->name.port].mode == APP_EEPROM_METER_MODE_PWM_F_PWM_R)
            {
                DEBUG_VERBOSE("[PWM] +++++++\r\n");
                m_pulse_cnt_in_bkp[input->name.port].total_forward++;
                m_pulse_cnt_in_bkp[input->name.port].fw_flow++;
                m_pulse_cnt_in_bkp[input->name.port].total_fwd_idx = m_pulse_cnt_in_bkp[input->name.port].total_forward 
                                                                    / m_pulse_cnt_in_bkp[input->name.port].k + m_pulse_cnt_in_bkp[input->name.port].indicator;
                m_pulse_cnt_in_bkp[input->name.port].real_counter++;
            }
        }
        else
        {
           DEBUG_WARN("PWM Noise\r\n");
        }
    }
    // Data come from direction pin
    else if (input->name.isr_type == MEASURE_INPUT_NEW_DATA_TYPE_DIR_PIN)
    {
        if (ee_cfg->cyber_config[input->name.port].mode == APP_EEPROM_METER_MODE_PWM_F_PWM_R)
        {
            m_is_pulse_trigger = 1;

            m_end_pulse_timestamp[input->name.port].counter_dir = app_rtc_get_counter();
            m_end_pulse_timestamp[input->name.port].subsecond_dir = app_rtc_get_subsecond_counter();

            m_pull_diff[input->name.port] = get_diff_ms_between_pulse(&m_begin_pulse_timestamp[input->name.port],
                                                                 &m_end_pulse_timestamp[input->name.port],
                                                                 MEASURE_INPUT_NEW_DATA_TYPE_DIR_PIN);

            m_begin_pulse_timestamp[input->name.port].counter_dir = m_end_pulse_timestamp[input->name.port].counter_dir;
            m_begin_pulse_timestamp[input->name.port].subsecond_dir = m_end_pulse_timestamp[input->name.port].subsecond_dir;
            if (m_pull_diff[input->name.port] > PULSE_MINMUM_WITDH_MS)
            {
                DEBUG_INFO("[DIR]++++\r\n");
//                if (ee_cfg->meter_mode[input->port] == APP_EEPROM_METER_MODE_DISABLE || ee_cfg->meter_mode[input->port] == APP_EEPROM_METER_MODE_ONLY_PWM)
//                {
//                    m_pulse_cnt_in_bkp[input->port].reverse_counter = 0;
//                    m_pulse_cnt_in_bkp[input->port].total_reverse_index = 0;
//                    m_pulse_cnt_in_bkp[input->port].total_reverse = 0;
//                }
//                else
                {
                    m_pulse_cnt_in_bkp[input->name.port].total_reverse++;
                    m_pulse_cnt_in_bkp[input->name.port].reverse_counter++;
                    m_pulse_cnt_in_bkp[input->name.port].total_reverse_index = m_pulse_cnt_in_bkp[input->name.port].total_reverse 
                                                                                / m_pulse_cnt_in_bkp[input->name.port].k;
                }
            }
            else
            {
                DEBUG_WARN("DIR Noise\r\n");
            }
        }
        else
        {
            DEBUG_VERBOSE("[DIR]---\r\n");
        }
    }
    __enable_irq();
}

void modbus_master_serial_write(uint8_t *buf, uint8_t length)
{
    volatile uint32_t i;
    if (!HARDWARE_RS485_GET_DIRECTION())
    {
        HARDWARE_RS485_DIR_TX(); // Set TX mode
        i = 32;         // clock = 16Mhz =>> 1us = 16, delay at least 1.3us
        while (i--);
    }
    
    // Send data
    for (i = 0; i < length; i++)
    {
        LL_LPUART_TransmitData8(LPUART1, buf[i]);
        while (0 == LL_LPUART_IsActiveFlag_TXE(LPUART1));
    }
    
    // Wait until the last byte transfer to port
    while (0 == LL_LPUART_IsActiveFlag_TC(LPUART1))
    {
    }
    HARDWARE_RS485_DIR_RX();
}

uint32_t modbus_master_get_milis(void)
{
    return sys_get_ms();
}

measure_input_peripheral_data_t *measure_input_current_data(void)
{
    return &m_measure_data;
}

void modbus_master_sleep(void)
{
    //    __WFI();
    sys_delay_ms(1);
}

uint32_t measure_input_sensor_data_available(void)
{
    uint32_t retval = 0;

    for (uint32_t i = 0; i < MEASUREMENT_MAX_MSQ_IN_RAM; i++)
    {
        if (m_sensor_msq[i].state == MEASUREMENT_QUEUE_STATE_PENDING)
        {
            retval++;
        }
    }
    return retval;
}

measure_input_peripheral_data_t *measure_input_get_data_in_queue(void)
{
    for (uint32_t i = 0; i < MEASUREMENT_MAX_MSQ_IN_RAM; i++)
    {
        if (m_sensor_msq[i].state == MEASUREMENT_QUEUE_STATE_PENDING)
        {
            m_sensor_msq[i].state = MEASUREMENT_QUEUE_STATE_PROCESSING;
            return &m_sensor_msq[i];
        }
    }
    return NULL;
}


uint32_t measure_input_get_next_time_wakeup(void)
{
    uint32_t current_sec = app_rtc_get_counter();
    return (estimate_measure_timestamp - current_sec);
}

float measure_input_convert_4_20ma_to_pressure(float current)
{
    // Pa = a*current + b
    // 0 = 4*a + b
    // 10 = 20*a + b
    // =>> Pa = 0.625*current - 2.5f
    if (current < 4.0f)
    {
        return 0.0f;
    }

    if (current > 20.0f)
    {
        return 10.0f; // 10Pressure
    }

    // x10 bar
    return (6.25f * current - 25.0f);
}

uint32_t measure_input_get_time_remaing_turn_on_output_4_20ma_power()
{
    return delay_turn_on_power_for_external_sensor;
}
