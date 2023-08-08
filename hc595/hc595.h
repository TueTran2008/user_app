#ifndef HC595_HC595_H
#define HC595_HC595_H

#include <stdint.h>
#include <stdbool.h>

#define HC595_USE_LOCK 0 // RTOS mutex for multi task safe
#define HC595_DELAY_US 5
typedef enum
{
    HC595_PIN_SCK,
    HC595_PIN_DATA,
    HC595_PIN_LATCH,
    HC595_PIN_EN
} hc595_pin_t;

typedef void (*hc595_pin_write_callback_t)(hc595_pin_t pin, uint32_t value);
typedef void (*hc595_delay_callback_t)(uint32_t delay_us);

#if HC595_USE_LOCK
/**
 * @brief       Lock function, useful when using RTOS
 * @param[in]   true : Lock
 *              false : Unlock
 * @param[in]   timeout_ms : Max wait time in ms
 */
typedef bool (*hc595_lock_callback_t)(bool lock);
#endif

typedef struct
{
    hc595_pin_write_callback_t write_cb;
    hc595_delay_callback_t  delay_us;
#if HC595_USE_LOCK
    hc595_lock_callback_t lock;
    uint32_t max_wait_time_ms; // max lock wait time
#endif
} hc595_cfg_t;

/**
 * @brief       Config 74hc595 driver
 * @param[in]   cfg hc595 driver config
 */
void hc595_initialize(hc595_cfg_t *cfg);

/**
 * @brief       Write data to 74hc595 port with thread safe
 * @param[in]   cfg HC595 driver
 * @param[in]   data	Data write to port
 * @param[in]   length	Data length
 */
void hc595_write(hc595_cfg_t *cfg, uint8_t *data, uint32_t length);

#endif /* HC595_HC595_H */
