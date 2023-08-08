#ifndef APP_EEPROM_H
#define APP_EEPROM_H

#include <stdint.h>
#include "measure_input.h"

#define APP_EEPROM_DEFAULE_SERVER_ADDR                  "http://171.244.142.95:8084"
#define APP_EEPROM_BAUD_9600                            9600
#define APP_EEPROM_BAUD_19200                           19200
#define APP_EEPROM_BAUD_115200                          115200
#define APP_EEPROM_DEFAULT_BAUD                         APP_EEPROM_BAUD_9600
#define APP_EEPROM_BAUD_VALID                           0x58
#define APP_EEPROM_PULSE_VALID                          0x61
#define APP_EEPROM_MODBUS_MSB_FIRST                     0
#define APP_EEPROM_MODBUS_LSB_FIRST                     1
#define APP_EEPROM_RECHECK_PULSE_TIMEOUT_MS             5

#define APP_EEPROM_MEASURE_INTERVAL_MS                  (30*60*1000)
#define APP_EEPROM_SEND_TO_SERVER_INTERVAL_S            (60*60*1000)

#define APP_EEPROM_VALID_FLAG						    0x15334519
#define APP_EEPROM_SIZE								    (6*1024)

#define APP_EEPROM_MAX_PHONE_LENGTH                     16
#define APP_EEPROM_MAX_SERVER_ADDR_LENGTH               64
#define APP_EEPROM_MAX_NUMBER_OF_SERVER				    2
#define APP_EEPROM_MAIN_SERVER_ADDR_INDEX			    0
#define APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX	    (APP_EEPROM_MAX_NUMBER_OF_SERVER-1)
#define APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME         -1
#define APP_EEPROM_MAX_FIXED_SEND_WEB_TIME              6

/**
 * \defgroup        App eeprom
 * \brief           Store device config into internal eeprom
 * \{
 */

typedef union
{
    struct
    {
        uint32_t input0                             : 1;
        uint32_t input1                             : 1;
        uint32_t rs485_en                           : 1;
        uint32_t warning                            : 1;
        uint32_t sos                                : 1;
        uint32_t input_4_20ma_0_enable              : 1;
        uint32_t input_4_20ma_1_enable              : 1;
        uint32_t input_pressure_sensor_0            : 1;
        uint32_t input_pressure_sensor_1            : 1;
        uint32_t output_4_20ma_enable               : 1;
        uint32_t alarm_sensor_value_high            : 1;
        uint32_t alarm_sensor_value_low             : 1;
        uint32_t register_sim_status                : 1;
        uint32_t calculate_flow_speed               : 1;
        uint32_t notify_imei                        : 1;
        uint32_t cyber_direction_level              : 1;

        uint32_t output_4_20ma_timeout_100ms        : 8;  


        uint32_t rs485_byte_order                   : 1;
        uint32_t modbus_cal_method                  : 2;
        uint32_t esim_active                        : 1;
        uint32_t reserve                            : 4;

    } __attribute__((packed)) name;
    uint32_t value;
} __attribute__((packed)) app_eeprom_io_enable_t;

typedef struct
{
    // Input offset and factor
    uint32_t k;
    uint32_t offset;
    uint8_t mode;	
} __attribute__((packed)) cyber_config_t;

typedef struct
{
    uint32_t measure_interval_ms;
    app_eeprom_io_enable_t io_enable;       // Viet the nay cho tiet kiem bo nho
                                            // Neu chia ra thi se de hieu hon
    uint32_t valid_flag;

    // Input offset and factor
    cyber_config_t cyber_config[APP_EEPROM_METER_MODE_MAX_ELEMENT];
    uint32_t cyber_pulse_ms;

    // Server
    // Vi du cu 15p gui 1 lan, thi thoi gian thuc day la : 15p + send_to_server_delay_s
    uint32_t send_to_server_delay_s;        
    // Dia chi server gui tin
    uint8_t http_server_address[APP_EEPROM_MAX_NUMBER_OF_SERVER][APP_EEPROM_MAX_SERVER_ADDR_LENGTH];
    uint32_t send_to_server_interval_ms;
    uint8_t phone[APP_EEPROM_MAX_PHONE_LENGTH];

    // Modbus
    measure_input_modbus_register_t rs485[HARDWARE_RS485_MAX_SLAVE_ON_BUS];
    uint32_t rs485_baudrate;

    float output_4_20ma;
    uint32_t poll_config_interval_hour;		// Poll configuration from default link in second
    uint8_t battery_low_percent;
    uint8_t sync_clock_in_hour;            // Time to synchronize NTP time
    uint8_t max_sms_1_day;
    int16_t sync_web_in_min[APP_EEPROM_MAX_FIXED_SEND_WEB_TIME];
    uint32_t debug_timeout_second;
    uint32_t crc;
} __attribute__((packed)) app_eeprom_config_data_t;

typedef struct
{
    // Factory reset server address
    uint8_t server[APP_EEPROM_MAX_SERVER_ADDR_LENGTH];
    uint8_t reserve[28];
    uint32_t crc;
} __attribute__((packed)) app_eeprom_factory_data_t;


/*!
 * @brief       Init eeprom data 
 */
void app_eeprom_init(void);

/*!
 * @brief       Save data to eeprom
 */
void app_eeprom_save_config(void);

/*!
 * @brief       Read data from eeprom
 * @retval      Pointer to data, NULL on error
 */
app_eeprom_config_data_t *app_eeprom_read_config_data(void);

/*!
 * @brief       Erase eeprom data
 */
void app_eeprom_erase(void);

/*!
 * @brief       Save default factory data
 */
void app_eeprom_save_factory_data(app_eeprom_factory_data_t *factory_data);

/*!
 * @brief       Read factory reset data
 * @retval      Pointer to data
 */
app_eeprom_factory_data_t *app_eeprom_read_factory_data(void);

/*!
 * @brief       Get cyber pulse ms timeout to filter noise
 */
uint32_t app_eeprom_get_cyber_pulse_ms(void);

/*!
 * @brief       Get fix time setting to wakeup and send data to internet
 * @retval      Pointer to data
 */
int16_t *app_eeprom_get_fixed_time_wakeup_setting(void);

/*!
 * @brief       Get fix time setting to wakeup and send data to internet
 * @param[in]   Pointer to data
 */
void app_eeprom_set_fixed_time_wakeup_setting(int16_t *data);

/*!
 * @brief       Check if device working in fixed wakeup mode
 * @retval      TRUE device has fixed time
 * @retval      FALSe device not has fixed time
 */
bool app_eeprom_has_fixed_time_wakeup(void);


/**
 * \}
 */

#endif /* APP_EEPROM_H */
