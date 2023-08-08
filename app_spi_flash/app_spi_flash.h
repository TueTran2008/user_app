#ifndef APP_SPI_FLASH_H
#define APP_SPI_FLASH_H

#include <stdint.h>
#include <stdbool.h>

#ifndef APP_SPI_FLASH_LOW_POWER_MODE
#define APP_SPI_FLASH_LOW_POWER_MODE 1
#endif

#ifndef APP_SPI_FLASH_VERIFY_AFTER_WRITE
#define APP_SPI_FLASH_VERIFY_AFTER_WRITE 0
#endif

typedef enum
{
    APP_SPI_FLASH_MEM_OK = 0,
    APP_SPI_FLASH_MEM_ERROR_RING_BUFFER_EMPTY,
    APP_SPI_FLASH_MEM_ERROR_DATA_CORRUPT,
    APP_SPI_FLASH_MEM_PHYSIC_ERROR
} app_flash_mem_error_t;


typedef union
{
    struct
    {
        uint8_t manu_id;
        uint8_t mem_type;
        uint8_t capacity;
    } __attribute__((packed)) name;
    uint8_t raw[3];
} __attribute__((packed)) app_spi_flash_device_id_t;

typedef enum
{
    APP_SPI_FLASH_DEVICE_INVALID,
    APP_SPI_FLASH_FL164K,
    APP_SPI_FLASH_FL127S,
    APP_SPI_FLASH_FL256S,
    APP_SPI_FLASH_GD256,
    APP_SPI_FLASH_W25Q256JV,
    APP_SPI_FLASH_W25Q80D,
    APP_SPI_FLASH_W25Q128,
    APP_SPI_FLASH_W25Q32,
    APP_SPI_FLASH_W25Q64,
    APP_SPI_FRAM_MB85RS16,
    APP_SPI_FLASH_AT25SF128,
    APP_SPI_FRAM_FM25V02A,
    APP_SPI_FRAM_MB85RS64V,
    APP_SPI_FRAM_FM25V01,
    APP_SPI_FRAM_FM25V10,
    APP_SPI_FRAM_MB85RS128TYPNF,
    APP_SPI_FLASH_IS25LP032D,
    APP_SPI_FLASH_GD25Q64,
    APP_SPI_FLASH_MAX
} app_flash_device_t;

typedef enum
{
    APP_SPI_DEVICE_ERROR,
    APP_SPI_FLASH,
    APP_SPI_FRAM
} app_flash_type_t;

typedef union
{
    struct
    {
        uint8_t WIP : 1;
        uint8_t WEL : 1;
        uint8_t BP0 : 1;
        uint8_t BP1 : 1;
        uint8_t BP2 : 1;
        uint8_t BP3 : 1;
        uint8_t QE : 1;
        uint8_t SRWD : 1;
    } __attribute__((packed)) name;
    uint8_t val;
} app_spi_flash_status_register_t;

typedef struct
{
    app_flash_device_t device;
    uint32_t size; // bytes
    app_flash_type_t type;
    uint16_t page_size;
    uint16_t sector_size;
} app_flash_info_t;

typedef struct
{
    void (*spi_tx_buffer)(void *spi, uint8_t *tx_data, uint32_t length);
    void (*spi_rx_buffer)(void *spi, uint8_t *rx_data, uint32_t length);
    void (*spi_tx_rx)(void *spi, uint8_t *tx_data, uint8_t *rx_data, uint32_t length);
    uint8_t (*spi_tx_byte)(void *spi, uint8_t data);
    void (*spi_cs)(void *spi, bool level);
    void (*delay_ms)(void *spi, uint32_t ms);
} app_spi_flash_cb_t;

typedef struct
{
    void *spi;
    app_spi_flash_cb_t callback;
    app_flash_info_t info;
    uint32_t ringbuffer_log_size;
    bool error;
    uint8_t *working_buffer;        // at least 1 sector size
} app_flash_drv_t;

/**
 * @brief       Read status register
 * @param[in]   driver Pointer to flash driver
 * @retval      Register value
 */
 
app_spi_flash_status_register_t app_spi_flash_read_status(app_flash_drv_t *driver);

/*!
 * @brief       Initialize spi flash
 */
void app_spi_flash_initialize(app_flash_drv_t *driver);

/*!
 * @brief       Estimate write address
 * @param[in]   flash_full : Flash full or not
 */
uint32_t app_flash_estimate_next_write_addr(bool *flash_full, bool erase_next_page);

/*!
 * @brief       Dump all valid data
 */
uint32_t app_spi_flash_dump_all_data(void);

/**
 * @brief       Erase all data in flash
 * @param[in]   driver Pointer to flash driver
 * @param[in]   timeout_ms Timeout in ms
 * @param[in]   wait_until_complete TRUE Wait until flash complete or timeout is over
 *                                  FALSE Dont wait
 * @retval      TRUE Erase flash success
 *              FALSE Erase flash failed
 */
bool app_spi_flash_erase_all(app_flash_drv_t *driver, uint32_t timeout_ms, bool wait_until_complete);

/*!
 * @brief       Check flash ok status
 */
bool app_spi_flash_is_ok(app_flash_drv_t *driver);

/*!
 * @brief       Wakeup the flash
 * @param[in]   driver Flash driver
 */
void app_spi_flash_wakeup(app_flash_drv_t *driver);

/*!
 * @brief       Power down then flash
 * @param[in]   driver Flash driver
 */
void app_spi_flash_shutdown(app_flash_drv_t *driver);


/*!
 * @brief       Check sector is empty or not
 * @param[in]   sector Sector count
 */
bool app_spi_flash_is_sector_empty(app_flash_drv_t *driver, uint32_t sector_count);
    
/*!
 * @brief       Flash stress write test
 * @param[in]   nb_of_write_times Number of write times
 */
void app_spi_flash_stress_test(uint32_t nb_of_write_times);

/*!
 * @brief       Flash read all retransmission pending data
 * @param[in]   nb_of_write_times Number of write times
 */
void app_spi_flash_retransmission_data_test(void);

/*!
 * @brief       Test write behavior if flash full
 */
void app_spi_flash_skip_to_end_flash_test(void);

/*!
 * @brief       Erase 1 sector
 * @param[in]   sector_count Sector number
 */
bool app_spi_flash_erase_sector_4k(app_flash_drv_t *driver, uint32_t sector_count);


/*!
 * @brief       Write data to flash
 * @param[in]   addr Address in flash
 * @param[in]   buffer Data to write
 * @param[in]   length Size of data in bytes
 */
bool app_spi_flash_write(app_flash_drv_t *driver, uint32_t addr, uint8_t *buffer, uint32_t length);
    
/*!
 * @brief       Read data at specific address in flash
 * @param[in]   driver Driver flash
 * @param[in]   addr Begin address
 * @param[in]   buffer Pointer to buffer contain data
 * @param[in]   length Number of bytes to read
 */
void app_spi_flash_read_bytes(app_flash_drv_t *driver, uint32_t addr, uint8_t *buffer, uint16_t length);

#endif /* APP_SPI_FLASH_H */
