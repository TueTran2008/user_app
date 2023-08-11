#include "flash_if.h"
#include "main.h"
#include "app_debug.h"
#include "ota_update.h"
#include "gd32f10x_fmc.h"
#include "app_wdt.h"
#include "string.h"

#define ABS_RETURN(x, y) (((x) < (y)) ? (y) : (x))

#define FLASH_IF_DISABLE_IRQ()          __disable_irq()
#define FLASH_IF_ENABLE_IRQ()           __enable_irq()

//#define _USE_EXTERNAL_FLASH_        1

#if(_USE_EXTERNAL_FLASH_)
#include "Flash.h"
#endif




bool flash_if_init(void)
{
    return true;
}
/*****************************************************************************/
/**
 * @brief	:   
 * @param	:  
 * @retval	:   
 * @author	:	TueTD
 * @created	:	30/05/2021
 * @version	:
 * @reviewer:	
 */
flash_if_error_t flash_if_erase(uint32_t addr, uint32_t size)
{
    uint32_t retval = 0;
#if(_USE_EXTERNAL_FLASH_)
    
    uint32_t begin_addr = addr;
    
    FLASH_IF_DISABLE_IRQ();
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    fmc_flag_clear(FMC_FLAG_BANK1_END);
    fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
    
    for (uint32_t i = 0; i < (size+FLASH_IF_PAGE_SIZE-1)/FLASH_IF_PAGE_SIZE; i++)
    {
        //DEBUG_INFO("Erase flash at addr 0x%08X\r\n", addr);
        // Erase the flash pages
        retval += fmc_page_erase(addr);
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        fmc_flag_clear(FMC_FLAG_BANK1_END);
        fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
        addr += FLASH_IF_PAGE_SIZE;
        app_wdt_feed();
    }
    fmc_lock();
    
    FLASH_IF_ENABLE_IRQ();
    for (uint32_t i = 0; i < FLASH_IF_PAGE_SIZE/4; i++)
    {
        if (*((volatile uint32_t*)begin_addr) != 0xFFFFFFFF)
        {
            //DEBUG_WARN("ERASE Failed\r\n");
            retval++;
            break;
        }
    }
    DEBUG_INFO("Erase done\r\n");
    if (retval)
    {
        return FLASH_IF_ERASE_KO;
    }
    return FLASH_IF_OK;
#else
    //FLASH_ERASE(FlashEraseSector4K);
    //FLASH_IF_DISABLE_IRQ();
    for (uint32_t index = 0; index < (size/ EXTERNAL_FLASH_BLOCK_SIZE); index++)
    {
        uint8_t read_back_buffer; /*1024 is the read back buffer data*/
        FlashEraseBlock64K((addr/EXTERNAL_FLASH_BLOCK_SIZE) + index);
        /*for (uint32_t read_back_index = 0; EXTERNAL_FLASH_BLOCK_SIZE/ 1024; read_back_index++)
        {*/
            //Delayms(100);
            FlashReadBytes(addr + EXTERNAL_FLASH_BLOCK_SIZE * index, &read_back_buffer , 1);
            //for (uint8_t i = 0; i < 10; i++)
            //{
                if(read_back_buffer != 0xFF)
                {
                    DEBUG_ERROR("Erase External Flash Fail- readback value: 0x%02x\r\n",read_back_buffer);
                    retval++;
                }
                else
                {
                    DEBUG_RAW("Erase Flash OK\r\n");
                }
            //}
        //}
    }
    //FLASH_IF_ENABLE_IRQ();
#endif
//    return retval;
}

flash_if_error_t flash_if_copy(uint32_t destination, uint32_t *source, uint32_t nb_of_word)
{
    uint32_t retval = 0;
#if(_USE_EXTERNAL_FLASH_)
    uint32_t readback = 0;
   // DEBUG_INFO("Write at Address: 0x%08x\r\n", destination);
    FLASH_IF_DISABLE_IRQ();
    fmc_unlock();
    for (uint32_t i = 0; i < nb_of_word; i++)
    {
        retval += fmc_word_program(destination, source[i]);
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        fmc_flag_clear(FMC_FLAG_BANK1_END);
        fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
        readback = *((uint32_t*)destination);
        if (retval || readback != source[i])
        {
            DEBUG_ERROR("Write Failed At: 0x%08x- Expected:0x%08x - Readback:0x%08x\r\n", destination, source[i], readback);
 
            retval = FLASH_IF_WRITING_ERROR;
            break;
        }
        destination += 4;
    }
    fmc_lock();
    FLASH_IF_ENABLE_IRQ();
    if (retval)
    {
        retval = FLASH_IF_WRITING_ERROR;
    }
    // FLASH_IF_ENABLE_IRQ();
    //app_wdt_feed();
#else
    static uint32_t des = 0;
    
    uint8_t temp_buff[512] = {0};
    uint8_t read_back = 0xFF;
    uint32_t nb_of_byte_left = nb_of_word * 4;
    des  = destination;
    for (uint8_t i = 0; i <= ((nb_of_word * 4)/ 512); i++)
    {
        if(nb_of_byte_left >= (uint32_t)512)
        {
            memcpy(temp_buff, source , 512);
            //Delayms(2);
            FlashWriteBytes(des, temp_buff, 512);
           // Delayms(2);
            FlashReadBytes(des, &read_back, 1);
            des = des + 512;
            /*Kieu cua source la uint32_t * => phai chia cho 4*/
            source = source + (512/4);
            nb_of_byte_left = nb_of_byte_left - 512;
        }
        else if(nb_of_byte_left > 0 )
        {
            memcpy(temp_buff, source , nb_of_byte_left);
            //Delayms(2);
            FlashWriteBytes(des, temp_buff, nb_of_byte_left);
           // Delayms(2);
            FlashReadBytes(des, &read_back, 1);
            des = des + nb_of_byte_left;
            source = source + (512/4);
            nb_of_byte_left = 0;
        }
        DEBUG_INFO("Destination:0x%08x:\r\n", des);
         //FlashReadBytes(des, &read_back, 1);
         if(read_back != temp_buff[0])
         {
            retval++;
            DEBUG_ERROR("Write Failed\r\n");
            DEBUG_RAW("Read back Value: 0x%02x- expected: 0x%02x\r\n", read_back, temp_buff[0]);
         }
         else
         {
             DEBUG_RAW("Read back Value: 0x%02x- expected: 0x%02x\r\n", read_back, temp_buff[0]);
         }
         //memset(temp_buff, 0, sizeof(temp_buff));
    }
#endif
    return (flash_if_error_t)retval;
}
