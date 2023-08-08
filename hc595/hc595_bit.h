/*
 * hc595_bit.h
 *
 *  Created on: Jul 27, 2021
 *      Author: huybk213
 */

#ifndef HC595_BIT_H_
#define HC595_BIT_H_

#include <stdint.h>
#include <stdbool.h>

typedef union
{
	struct
	{
		uint8_t power_b : 1;
		uint8_t power_r : 1;
		uint8_t net_r : 1;
		uint8_t net_b : 1;
		uint8_t led1 : 1;   // led_test_blue
		uint8_t led2 : 1;   // led_test_red
		uint8_t led3 : 1;
		uint8_t led4 : 1;
	} __attribute__((packed)) name;
	uint8_t value;
} __attribute__((packed)) hc595_bit_t;


/**
 * @brief		Get hc595 current value
 * @retval		Pointer to hc595 value
 */
hc595_bit_t *hc595_bit(void);
/**
 * @brief		Initialize hc595 bit mutex
 */
void hc595_bit_initialize(void);

/**
 * @brief		Lock mutex hc595
 * @param[in]	max_wait_time_ms Max timeout in ms
 * @retval		TRUE : Operation success
 * 				FALSE : Operation failed
 */
bool hc595_bit_lock(uint32_t max_wait_time_ms);

/**
 * @brief		Unlock mutex hc595
 */
void hc595_bit_unlock(void);

#endif /* HC595_BIT_H_ */
