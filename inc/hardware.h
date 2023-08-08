#ifndef __HARDWARE_H__
#define __HARDWARE_H__


#if 1
#include "main.h"

#define HARDWARE_LED1_CTRL(x)             	{	if (x) \
                                                    LL_GPIO_ResetOutputPin(LED1_GPIO_Port, LED1_Pin);	\
                                                else	\
                                                    LL_GPIO_SetOutputPin(LED1_GPIO_Port, LED1_Pin);	\
                                            }		

#define HARDWARE_LED2_CTRL(x)             	{	if (x) \
                                                    LL_GPIO_SetOutputPin(LED2_GPIO_Port, LED2_Pin);	\
                                                else	\
                                                    LL_GPIO_ResetOutputPin(LED2_GPIO_Port, LED2_Pin);	\
                                            }	
 
#define HARDWARE_LED1_TOGGLE()			    LL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)
#define HARDWARE_LED2_TOGGLE()			    LL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin)		
				                                
							
#define HARDWARE_PRESSURE_0_OUTPUT_POWER(x)      	{	if (x) \
                                                        LL_GPIO_ResetOutputPin(TRANS_OUT1_PRESSURE_GPIO_Port, TRANS_OUT1_PRESSURE_Pin);	\
                                                    else	\
                                                        LL_GPIO_SetOutputPin(TRANS_OUT1_PRESSURE_GPIO_Port, TRANS_OUT1_PRESSURE_Pin);	\
                                                    }	

#define HARDWARE_PRESSURE_1_OUTPUT_POWER(x)      	{	if (x) \
                                                            LL_GPIO_ResetOutputPin(TRANS_OUT2_PRESSURE_GPIO_Port, TRANS_OUT2_PRESSURE_Pin);	\
                                                        else	\
                                                            LL_GPIO_SetOutputPin(TRANS_OUT2_PRESSURE_GPIO_Port, TRANS_OUT2_PRESSURE_Pin);	\
                                                    }								
#define HARDWARE_PRESSURE_0_OUTPUT_DISABLE()      LL_GPIO_IsOutputPinSet(TRANS_OUT1_PRESSURE_GPIO_Port, TRANS_OUT1_PRESSURE_Pin)
#define HARDWARE_PRESSURE_1_OUTPUT_DISABLE()      LL_GPIO_IsOutputPinSet(TRANS_OUT2_PRESSURE_GPIO_Port, TRANS_OUT2_PRESSURE_Pin)
                                                    
#define HARDWARE_TRANS_1_TOGGLE()      	        {   \
                                                    LL_GPIO_TogglePin(TRANS_OUT1_PRESSURE_GPIO_Port, TRANS_OUT1_PRESSURE_Pin);	\
                                                }	

#define HARDWARE_TRANS_2_TOGGLE()      	        {   \
                                                    LL_GPIO_TogglePin(TRANS_OUT2_PRESSURE_GPIO_Port, TRANS_OUT2_PRESSURE_Pin);	\
                                                }	
                                

								
#define HARDWARE_NTC_POWER_ON(x)                {	if (x) \
                                                        LL_GPIO_SetOutputPin(VNTC_GPIO_Port, VNTC_Pin);	\
                                                    else	\
                                                        LL_GPIO_ResetOutputPin(VNTC_GPIO_Port, VNTC_Pin);	\
                                                }		

#define HARDWARE_NTC_IS_POWER_ON()		LL_GPIO_IsOutputPinSet(VNTC_GPIO_Port, VNTC_Pin)						
								
								
#define HARDWARE_ENABLE_INPUT_4_20MA_POWER(x)	{	if (x) \
                                                        {   \
                                                            LL_GPIO_ResetOutputPin(EN_4_20MA_3V3_IN_GPIO_Port, EN_4_20MA_3V3_IN_Pin);	\
                                                            HARDWARE_EXT_4_20MA_SENSOR_PWR_ON(); \
                                                        }   \
                                                    else	\
                                                        {   \
                                                            LL_GPIO_SetOutputPin(EN_4_20MA_3V3_IN_GPIO_Port, EN_4_20MA_3V3_IN_Pin);	\
                                                            HARDWARE_EXT_4_20MA_SENSOR_PWR_OFF(); \
                                                        }   \
                                                }

#define HARDWARE_INPUT_POWER_4_20_MA_IS_ENABLE()       (LL_GPIO_IsOutputPinSet(EN_4_20MA_3V3_IN_GPIO_Port, EN_4_20MA_3V3_IN_Pin) ? 0 : 1)
                 

                                        
// 4.2V provice power for GSM and 4-20mA ouput
#define HARDWARE_ENABLE_OUTPUT_4_20MA_POWER(x)	{	if (x) \
                                                        LL_GPIO_SetOutputPin(EN_4_20MA_VREF_OUT_POWER_GPIO_Port, EN_4_20MA_VREF_OUT_POWER_Pin);	\
                                                    else	\
                                                        LL_GPIO_ResetOutputPin(EN_4_20MA_VREF_OUT_POWER_GPIO_Port, EN_4_20MA_VREF_OUT_POWER_Pin);	\
                                                }
    
#define HARDWARE_ENABLE_SYS_5V(x)				{	if (x) \
                                                        LL_GPIO_SetOutputPin(SYS_5V_EN_GPIO_Port, SYS_5V_EN_Pin);	\
                                                    else	\
                                                        LL_GPIO_ResetOutputPin(SYS_5V_EN_GPIO_Port, SYS_5V_EN_Pin);	\
                                                }	
										
#define HARDWARE_RS485_POWER_EN(x)				    {	if (x) \
                                                            LL_GPIO_ResetOutputPin(RS485_EN_GPIO_Port, RS485_EN_Pin);	\
                                                        else	\
                                                            LL_GPIO_ResetOutputPin(RS485_EN_GPIO_Port, RS485_EN_Pin);	\
                                                    }	
#define HARDWARE_RS485_DIR_TX()                     {   \
                                                        LL_GPIO_SetOutputPin(RS485_DIR_GPIO_Port, RS485_DIR_Pin);     \
                                                    }
   
#define HARDWARE_RS485_DIR_RX()                     {   \
                                                        LL_GPIO_ResetOutputPin(RS485_DIR_GPIO_Port, RS485_DIR_Pin);     \
                                                    }                                        
#define HARDWARE_RS485_GET_DIRECTION()               LL_GPIO_IsOutputPinSet(RS485_DIR_GPIO_Port, RS485_DIR_Pin)
#endif
                                        
#define HARDWARE_EXT_4_20MA_SENSOR_PWR_ON()         {   \
                                                        LL_GPIO_SetOutputPin(EXTERNAL_EN_4_20MA_POWER_PORT, EXTERNAL_EN_4_20MA_POWER_PIN);     \
                                                    }                                        
#define HARDWARE_EXT_4_20MA_SENSOR_PWR_OFF()        {   \
                                                        LL_GPIO_ResetOutputPin(EXTERNAL_EN_4_20MA_POWER_PORT, EXTERNAL_EN_4_20MA_POWER_PIN);     \
                                                    }   


#define HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE    2
#define HARDWARE_NUMBER_OF_INPUT_4_20MA			    2
#define MEASURE_NUMBER_OF_WATER_METER_INPUT			2
#define HARDWARE_PRESSURE_0_INDEX_IN_4_20MA_ARRAY   2
#define HARDWARE_PRESSURE_1_INDEX_IN_4_20MA_ARRAY   3

#define HARDWARE_NUMBER_OF_OUTPUT_4_20MA                     1

#define HARDWARE_MEASURE_INPUT_PORT_1						    0
#define HARDWARE_MEASURE_INPUT_PORT_2		                    1
#define	HARDWARE_NUMBER_OF_INPUT_ON_OFF						    2
#define HARDWARE_NUMBER_OF_ANALOG_INPUT                         2

// Neu so thiet bi HARDWARE_RS485_MAX_SLAVE_ON_BUS >= 2 thi bi stack over flow khi tinh toan thanh ghi modbus float
// Ban chua co thoi gian nen toi uu sau
#define HARDWARE_RS485_MAX_SLAVE_ON_BUS						    1
#define HARDWARE_RS485_MAX_REGISTER_SUPPORT					    4
#define HARDWARE_RS485_MAX_SUB_REGISTER						    HARDWARE_RS485_MAX_REGISTER_SUPPORT



#define APP_EEPROM_METER_MODE_DISABLE				0
#define APP_EEPROM_METER_MODE_PWM_PLUS_DIR_MIN		1 // Meter mode 0 : PWM++, DIR--
#define APP_EEPROM_METER_MODE_ONLY_PWM				2 // Meter mode 1 : PWM++
#define APP_EEPROM_METER_MODE_PWM_F_PWM_R			3 // Meter mode 2 : PWM_F & PWM_R
#define APP_EEPROM_METER_MODE_MAX_ELEMENT           2
#define HARDWARE_RS485_MAX_UNIT_NAME_LENGTH				6

#define APP_FLASH_VALID_DATA_KEY                    0x12345678               
#define APP_FLASH_NB_OFF_4_20MA_INPUT               HARDWARE_NUMBER_OF_INPUT_4_20MA
#define APP_FLASH_NB_OF_METER_INPUT                 APP_EEPROM_METER_MODE_MAX_ELEMENT
#define APP_FLASH_RS485_MAX_SIZE                    (HARDWARE_RS485_MAX_REGISTER_SUPPORT)

#define APP_FLASH_DONT_NEED_TO_SEND_TO_SERVER_FLAG  0xA5A5A5A5
//#define APP_FLASH_DATA_HEADER_KEY                   0x9813567A

#define HARDWARE_CRC32_SIZE		4


#define HARDWARE_GET_INPUT_ON_OFF_0()			                (LL_GPIO_IsInputPinSet(OPTOIN1_GPIO_Port, OPTOIN1_Pin) ? 1 : 0)
#define HARDWARE_GET_INPUT_ON_OFF_1()			                (LL_GPIO_IsInputPinSet(OPTOIN2_GPIO_Port, OPTOIN2_Pin) ? 1 : 0)



//static const measure_input_4_20ma_lookup_t lookup_table_4_20ma_input[] =
//{
//    {  0,        400    },
//    {  400,      490    },
//    {  500,      591    },  
//    {  600,      688    },   
//    {  710,      800    },     
//    {  800,      890    },    
//    {  900,      992    },    
//    {  1100,     1098   },  
//    {  1290,     1385   },    
//    {  1525,     1617   },     
//    {  1823,     1915   },     
//    {  2000,     2100   },
//};
                                  

                                  
#endif /* __HARDWARE_H__ */
