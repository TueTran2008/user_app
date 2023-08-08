#include "spi.h"
#include "app_debug.h"
#include "gd32e23x.h"
#include "board_hw.h"

void HAL_SPI_CS(bool level)
{
	if (level)
		gpio_bit_set(BOARD_HW_EXT_FLASH_CS_PORT, BOARD_HW_EXT_FLASH_CS_PIN); 
	else
		gpio_bit_reset(BOARD_HW_EXT_FLASH_CS_PORT, BOARD_HW_EXT_FLASH_CS_PIN); 
}

void HAL_SPI_Initialize(void)
{
	    // SPI bus
    DEBUG_INFO("Initialize spi bus\r\n");
    rcu_periph_clock_enable(RCU_SPI0);
    rcu_periph_clock_enable(RCU_GPIOA);

	gpio_mode_set(BOARD_HW_EXT_FLASH_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BOARD_HW_EXT_FLASH_CS_PIN);
    gpio_output_options_set(BOARD_HW_EXT_FLASH_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BOARD_HW_EXT_FLASH_CS_PIN);
    gpio_bit_set(BOARD_HW_EXT_FLASH_CS_PORT, BOARD_HW_EXT_FLASH_CS_PIN);   
	
    /* SPI0 GPIO config: SCK/PA5, MISO/PA6, MOSI/PA7 */
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    spi_parameter_struct spi_init_struct;

    /* deinitilize SPI and the parameters */
    spi_i2s_deinit(SPI0);
    spi_struct_para_init(&spi_init_struct);
    
    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_16;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
    
    spi_enable(SPI0);
}

void HAL_SPI_Transmit(void *instance, uint8_t *tx_data, uint32_t len, uint32_t timeout)
{    
    bool invalid = false;
	uint8_t rx_dummy_data[1];
    for (uint32_t i = 0; i < len; i++)
    {
        timeout = 0x000FFFFF;
        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) && timeout--);
        if (timeout == 0)
        {
            invalid = true;
        }
        spi_i2s_data_transmit(SPI0, tx_data[i]);
        
        timeout = 0x000FFFFF;
        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE) && timeout--);
        if (timeout == 0)
        {
            invalid = true;
        }
        rx_dummy_data[0] = spi_i2s_data_receive(SPI0);
    }

    if (invalid)
    {
        DEBUG_INFO("SPI error\r\n");
    }
}


void HAL_SPI_Receive(void *instance, uint8_t *rx_data, uint32_t len, uint32_t timeout)
{
    bool invalid = false;
    for (uint32_t i = 0; i < len; i++)
    {
        timeout = 0x000FFFFF;
        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) && timeout--);
        if (timeout == 0)
        {
            invalid = true;
        }
        spi_i2s_data_transmit(SPI0, 0xFF);
        
        timeout = 0x000FFFFF;
        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE) && timeout--);
        if (timeout == 0)
        {
            invalid = true;
        }
        rx_data[i] = spi_i2s_data_receive(SPI0);
    }

    if (invalid)
    {
        DEBUG_INFO("SPI error\r\n");
    }
}


void HAL_SPI_TransmitReceive(void *instance, uint8_t *tx_data, uint8_t *rx_data, uint32_t len, uint32_t timeout)
{    
    bool invalid = false;
    for (uint32_t i = 0; i < len; i++)
    {
        timeout = 0x000FFFFF;
        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) && timeout--);
        if (timeout == 0)
        {
            invalid = true;
        }
        spi_i2s_data_transmit(SPI0, tx_data[i]);
        
        timeout = 0x000FFFFF;
        while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE) && timeout--);
        if (timeout == 0)
        {
            invalid = true;
        }
        rx_data[i] = spi_i2s_data_receive(SPI0);
    }

    if (invalid)
    {
        DEBUG_INFO("SPI error\r\n");
    }
}

