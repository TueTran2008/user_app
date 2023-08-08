#ifndef MEASURE_INTPUT_H
#define MEASURE_INTPUT_H

#include "hardware.h"
#include "app_spi_flash.h"


#include <stdint.h>
#include <stdbool.h>

#define MEASURE_INTPUT_WAIT_4_20MA_SENSOR_POWER_ON_MS 3000
//#define MEASURE_INPUT_NEW_DATA_TYPE_PWM_PIN         0
//#define MEASURE_INPUT_NEW_DATA_TYPE_DIR_PIN         1
#define MEASUREMENT_MAX_MSQ_IN_RAM                              6

#define MEASUREMENT_QUEUE_STATE_IDLE                            0
#define MEASUREMENT_QUEUE_STATE_PENDING                         1       // Dang cho de doc
#define MEASUREMENT_QUEUE_STATE_PROCESSING                      2       // Da doc nhung dang xu li, chua release thanh free

#define MEASURE_INPUT_RS485_DATA_TYPE_INT16						0
#define MEASURE_INPUT_RS485_DATA_TYPE_INT32						1
#define MEASURE_INPUT_RS485_DATA_TYPE_FLOAT						2

#define MEASURE_INPUT_FLOW_INVALID_VALUE                  (-1.0f)

typedef enum
{
    MEASURE_INPUT_NEW_DATA_TYPE_PWM_PIN = 0,
    MEASURE_INPUT_NEW_DATA_TYPE_DIR_PIN = 1
} measure_input_cyber_pulse_counter_isr_type_t;

typedef union
{
    struct 
    {
        uint8_t port                : 1;
        uint8_t dir_level           : 1;
        uint8_t pwm_level           : 1;
        uint8_t line_break_detect   : 1;
        uint8_t isr_type            : 1;
        uint8_t reserved            : 3;    
    } __attribute__((packed)) name;
    uint8_t value;
} __attribute__((packed)) measure_input_cyber_meter_t;


typedef struct
{
    float fw_flow_min;
    float fw_flow_max;
    float reverse_flow_min;
    float reverse_flow_max;
    uint32_t fw_flow_first_counter;
    uint32_t reverse_flow_first_counter;
    float fw_flow_sum;
    float reverse_flow_sum;
    uint8_t valid;
} __attribute__((packed)) measure_input_flow_hour_t;


typedef struct
{
    uint32_t current_ma_mil_10;		// 4ma =>> 400
    int32_t adc_mv;					    // adc voltage
} measure_input_4_20ma_lookup_t;

typedef struct
{
    uint32_t round_per_sec_max;
    uint32_t round_per_sec_min;
} __attribute__((packed)) measure_input_cyber_speed_limit_t;

typedef struct
{
    uint32_t tick;
    uint8_t isr_type;
} measure_input_cyber_pulse_irq_t;


typedef struct
{
    int32_t real_counter;           // gia tri do khi da tinh ca xung am duong
    int32_t reverse_counter;
    int32_t total_forward;
    int32_t total_reverse;

    uint32_t fw_flow;               // Toc do quay thuan
    uint32_t reverse_flow;          // Toc do quay nguoc

    float flow_speed_fwd_agv_cycle_wakeup;             // Trung binh toc do giua 2 lan do 
    float flow_speed_reverse_agv_cycle_wakeup;             // Trung binh toc do giua 2 lan do 

    uint32_t total_fwd_idx;       // So nuoc thuan da tinh khi chia cho k + offset
    uint32_t total_reverse_index;       // so nuoc nghich da tinh sau khi chia cho k (ko co offset)

    measure_input_flow_hour_t flow_avg_cycle_send_web;        // Trung binh toc do giua 2 lan thuc day gui data len web

    uint32_t indicator;
    uint16_t k;
    uint8_t mode;
    uint8_t cir_break;
} __attribute__((packed)) measure_input_cyber_counter_t;



typedef union
{
    struct
    {
        uint8_t valid : 1;
        uint8_t type : 7;
    } __attribute__((packed)) name;
    uint8_t raw;
} __attribute__((packed)) measure_input_rs485_data_type_t;

typedef union
{
    float float_val;
    int32_t int32_val;
    uint8_t raw[4];
} __attribute__((packed)) measure_input_rs485_float_uint32_t;

typedef struct
{
	uint16_t reg_addr;
	measure_input_rs485_float_uint32_t value;
	measure_input_rs485_data_type_t data_type;
	uint8_t unit[HARDWARE_RS485_MAX_UNIT_NAME_LENGTH];
	int8_t read_ok;
} __attribute__((packed)) measure_input_rs485_sub_register_t;


typedef struct
{
	measure_input_rs485_sub_register_t sub_reg[HARDWARE_RS485_MAX_SUB_REGISTER];
    uint16_t fw_flow_reg;
    uint16_t reverse_flow_reg;
    uint16_t net_totalizer_fw_reg;
    uint16_t net_totalizer_reverse_reg;
    uint16_t net_totalizer_reg;
    uint8_t slave_addr;
} __attribute__((packed)) measure_input_modbus_register_t;


typedef struct
{
    uint32_t measure_timestamp;

    // Input 4-20mA
    float input_4_20mA[HARDWARE_NUMBER_OF_INPUT_4_20MA];

    // Input pressure
    uint32_t input_pressure_analog[HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE];

    // Input on/off
    uint8_t input_on_off[HARDWARE_NUMBER_OF_INPUT_ON_OFF];

    uint8_t output_4_20ma_enable;
    uint16_t analog_input[HARDWARE_NUMBER_OF_ANALOG_INPUT];

    // Output 	
    float output_4_20mA[HARDWARE_NUMBER_OF_OUTPUT_4_20MA];

    // Battery
    uint8_t vin_in_percent;
    uint16_t internal_battery_voltage;
    float vin_mv;

//	measure_input_cyber_meter_t water_pulse_counter[MEASURE_NUMBER_OF_WATER_METER_INPUT];
    measure_input_cyber_counter_t counter[MEASURE_NUMBER_OF_WATER_METER_INPUT];

    // Temperature
    int8_t temperature;
    int8_t temperature_error;

    // RS485
    measure_input_modbus_register_t rs485[HARDWARE_RS485_MAX_SLAVE_ON_BUS];

    // CSQ
    uint8_t csq_percent;

    // memory queue state
    uint8_t state;
} measure_input_peripheral_data_t;

extern uint8_t measure_input_485_error_code;

/*!
 * @brief       Init measurement module 
 */
void measure_input_initialize(void);

/*!
 * @brief       Poll input
 */
void measure_input_task(void);


/*!
 * @brief       RS485 new uart data
 * @param[in]	data New uart data
 */
void measure_input_rs485_uart_handler(uint8_t data);

/*!
 * @brief       RS485 IDLE detect
 */
void measure_input_rs485_idle_detect(void);


/*!
 * @brief       Read current measurement input data
 * @param[in]   data New uart data
 */
measure_input_peripheral_data_t *measure_input_current_data(void);

/*!
 * @brief       Reset counter in backup domain
 * @param[in]   index Pulse meter index
 */
void measure_input_reset_counter(uint8_t index);

/*!
 * @brief       Reset indicator offset
 * @param[in]   index Pulse meter index
 * @param[in]   new_indicator Pulse indicator
 */
void measure_input_reset_indicator(uint8_t index, uint32_t new_indicator);

/*!
 * @brief       Reset k offset
 * @param[in]   index k meter
 * @param[in]   new_k K divider
 */
void measure_input_reset_k(uint8_t index, uint32_t new_k);
	
/*!
 * @brief       Measure input callback
 * @param[in]   input New input data
 */
void measure_input_pulse_irq(measure_input_cyber_meter_t *input);

/**
 *  @brief      Wakeup to start measure data
 */
void measure_input_measure_wakeup_to_get_data(void);

/*!
 * @brief       Check device has new sensor data
 * @retval      Number of queue message
 */
uint32_t measure_input_sensor_data_available(void);

/*!
 * @brief       Get data in sensor message queue
 * @retval      Pointer to queue, NULL on no data available
 */
measure_input_peripheral_data_t *measure_input_get_data_in_queue(void);

/*!
 * @brief       Save all data in sensor message queue to flash 
 */
void measure_input_save_all_data_to_flash(void);

/*!
 * @brief       Set delay time to enable input 4_20ma
 * @param[in]   ms Delay time in ms
 */
void measure_input_delay_delay_measure_input_4_20ma(uint32_t ms);


/*!
 * @brief       Get next diff time wakeup and measure data
 * @retval      Next time wakeup
 */
uint32_t measure_input_get_next_time_wakeup(void);

/*!
 * @brief       Convert 4-20mA current to pressure in PA, range 0-10PA
 * @retval      Pressure value
 */
float measure_input_convert_4_20ma_to_pressure(float current);

/*!
 * @brief       Get time remaining turn on 4-20mA sensor in ms
 */
uint32_t measure_input_get_time_remaing_turn_on_output_4_20ma_power(void);

#endif /* MEASURE_INTPUT_H */
