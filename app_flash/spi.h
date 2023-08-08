#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief		Control CS
 * @param[in]	level TRUE CS high, otherwise cs low
 */
void HAL_SPI_CS(bool level);

/**
 * @brief		Initialize SPI service
 */
void HAL_SPI_Initialize(void);

/**
 * @brief		Transmit data
 * @param[in]	instance		Not used
 * @param[in]	data			Data write to spi port
 * @param[in]	length			Payload length
 * @param[in]	ms				Timeout in ms
 */
void HAL_SPI_Transmit(void *instance, uint8_t *data, uint32_t length, uint32_t timeout_ms);


/**
 * @brief		Received data
 * @param[in]	instance		Not used
 * @param[in]	data			Data read from spi port
 * @param[in]	length			Payload length
 * @param[in]	ms				Timeout in ms
 */
void HAL_SPI_Receive(void *instance, uint8_t *data, uint32_t length, uint32_t timeout_ms);


/**
 * @brief		Transmit and received data
 * @param[in]	instance		Not used
 * @param[in]	tx_data			Data write to spi port
 * @param[in]	rx_data			Data read from spi port
 * @param[in]	length			Payload length
 * @param[in]	ms				Timeout in ms
 */
void HAL_SPI_TransmitReceive(void *instance, uint8_t *tx_data, uint8_t *rx_data, uint32_t length, uint32_t timeout_ms);

#endif /* SPI_H */


