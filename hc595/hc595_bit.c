/*
 * hc595_bit.c
 *
 *  Created on: Jul 27, 2021
 *      Author: huybk213
 */
#include "hc595_bit.h"
#include <string.h>

#if HC595_USE_LOCK
#include "FreeRTOS.h"
#include "semphr.h"
static SemaphoreHandle_t m_lock;
#endif

static hc595_bit_t m_hc595_bit;

hc595_bit_t *hc595_bit(void)
{
    return &m_hc595_bit;
}

void hc595_bit_initialize(void)
{
#if HC595_USE_LOCK
    if (m_lock == NULL)
    {
        m_lock = xSemaphoreCreateMutex();
        xSemaphoreGive(m_lock);
    }
#endif
    memset(&m_hc595_bit, 0x00, sizeof(hc595_bit_t));
}

bool hc595_bit_lock(uint32_t max_wait_time_ms)
{
#if HC595_USE_LOCK
    return xSemaphoreTake(m_lock, max_wait_time_ms);
#else
    return true;
#endif
}

void hc595_bit_unlock(void)
{
#if HC595_USE_LOCK
    xSemaphoreGive(m_lock);
#endif
}
