#include "hc595.h"

void hc595_initialize(hc595_cfg_t *cfg)
{
    if (!cfg || !cfg->write_cb || !cfg->delay_us)
    {
        return;
    }

#if HC595_USE_LOCK
    if (!cfg->lock)
    {
        return;
    }

    if (!cfg->lock(true))
    {
        return;
    }
#endif

    cfg->write_cb(HC595_PIN_LATCH, 0);
    cfg->write_cb(HC595_PIN_EN, 0);

#if HC595_USE_LOCK
    cfg->lock(false);
#endif
}

void hc595_write(hc595_cfg_t *cfg, uint8_t *data, uint32_t length)
{
    if (!cfg || !cfg->write_cb)
    {
        return;
    }

#if HC595_USE_LOCK
    if (!cfg->lock)
    {
        return;
    }

    if (!cfg->lock(true))
    {
        return;
    }
#endif

    uint16_t i;
    uint8_t byte;

    data += length - 1; // Equivalent to data[length-1]

    while (length--)
    {
        byte = *data--;                  // Take the array data[length-1] and assign byte, and then --data[length-1].
        cfg->write_cb(HC595_PIN_SCK, 0); // clk = 0 keep the buffer state
        for (i = 0; i < 8; i++)
        {
            if (byte & 0x80) // Start sending from the high position
            {
                cfg->write_cb(HC595_PIN_DATA, 1); // data=1
            }
            else
            {
                cfg->write_cb(HC595_PIN_DATA, 0); // data=0
            }
            byte <<= 1;
            cfg->write_cb(HC595_PIN_SCK, 1); // clk = 1 Store data to the buffer
            cfg->delay_us(HC595_DELAY_US);        
            cfg->write_cb(HC595_PIN_SCK, 0); // clk = 0 keep the buffer state
            cfg->delay_us(HC595_DELAY_US);        
        }
    }
    cfg->write_cb(HC595_PIN_LATCH, 0);
    cfg->delay_us(HC595_DELAY_US);          
    cfg->write_cb(HC595_PIN_LATCH, 1); // Send buffer content

#if HC595_USE_LOCK
    cfg->lock(false);
#endif
}
