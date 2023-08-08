#ifndef APP_FLASH_LOG_H
#define APP_FLASH_LOG_H

#include "app_spi_flash.h"
#include "measure_input.h"

typedef union
{
    struct
    {
        uint8_t input_on_off_0 : 1;
        uint8_t input_on_off_1 : 1;
        uint8_t reserver : 6;
    } __attribute__((packed)) name;
    uint8_t value;
} __attribute__((packed)) app_spi_flash_on_off_data_t;

typedef struct
{
    uint32_t valid_flag;
    measure_input_cyber_counter_t counter[APP_FLASH_NB_OF_METER_INPUT];		// so nuoc
    measure_input_modbus_register_t rs485[HARDWARE_RS485_MAX_SLAVE_ON_BUS];			// 485
    uint32_t input_pressure_analog[HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE];
    uint8_t temp;	// nhiet do
    uint8_t csq_percent;
    uint16_t internal_battery_voltage;		// dien ap pin
    uint8_t vin_in_percent;
    uint8_t output_4_20ma_enable;
    app_spi_flash_on_off_data_t on_off;
    
    float input_4_20mA[APP_FLASH_NB_OFF_4_20MA_INPUT];		// 4-20mA
    float output_4_20mA[HARDWARE_NUMBER_OF_OUTPUT_4_20MA];
    uint16_t analog_input[HARDWARE_NUMBER_OF_ANALOG_INPUT];
    
    uint32_t timestamp;
    uint32_t resend_to_server_flag;
    uint32_t crc;
} __attribute__((packed)) app_flash_log_data_t;

#pragma pack(1)
typedef struct
{
    uint16_t valid_flag;
    uint32_t previous_write_index;
    uint32_t wr_index;
    uint32_t rd_index;
} app_flash_ringbuffer_pointer_t;

#pragma pack(1)
typedef struct
{
    uint16_t header;
    uint16_t size;
} app_flash_log_data_header_t;

/*!
 * @brief       Find current read and write pointer in flash
 * @param[in]   driver Flash driver
 */
void app_flash_log_find_read_write_pointer(app_flash_drv_t *driver);

/*!
 * @brief       Write data to flash
 * @param[in]   driver Flash driver
 * @param[in]   wr_data New data
 */
void app_flash_log_measurement_data(app_flash_drv_t *driver, app_flash_log_data_t *wr_data);

/*!
 * @brief       Dump all valid data to rs485 port
 * @param[in]   driver Flash driver
 * @retval      Number of valid packet
 */
uint32_t app_flash_log_send_to_host(app_flash_drv_t *driver);


/**
 * @brief       Get log data status in flash
 * @retval      TRUE Data is available
 *              TRUE Data is not available
 */
bool app_flash_memory_log_data_is_available(app_flash_drv_t *driver);

/**
 * @brief       Get current data available to send to server
 * @param[in]   message Pointer to data
 * @param[in]   size Size of data
 * @retval      app_flash_mem_error_t
 */
app_flash_mem_error_t api_flash_log_read_current_message(app_flash_drv_t *driver, uint8_t** message, uint16_t* size);

/**
 * @brief       Get last data available to send to server
 * @param[in]   message Pointer to data
 * @param[in]   size Size of data
 * @retval      app_flash_mem_error_t
 */
app_flash_mem_error_t app_flash_log_the_get_last_message(app_flash_drv_t *driver, uint8_t** message, uint16_t* size);


/*!
 * @brief       Get the last data
 * @param[in]   last_data Pointer will hold last write data
 * @retval      TRUE Operation success
 *              FALSE Operation failed
 */
bool app_flash_log_get_last_data(app_flash_log_data_t *last_data);

#endif /* APP_FLASH_LOG_H */
