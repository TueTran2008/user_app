/******************************************************************************
 * @file    	m_gsm_atcLayer.c
 * @author
 * @version 	V1.0.0
 * @date    	10/11/2014
 * @brief
 ******************************************************************************/

/******************************************************************************
                                   INCLUDES
 ******************************************************************************/
#include <string.h>
#include "gsm.h"
#include "app_debug.h"
#include "lwrb.h"
#include "main.h"

#define GSM_COMMUNICATION_TIMEOUT_SECOND    90


static gsm_hardware_t m_gsm_atc;
gsm_manager_t gsm_manager;

static volatile bool m_new_uart_data = false;
static char m_gsm_imei[16] = {0};
static char m_sim_imei[16];
static char m_nw_operator[32];
static char m_sim_ccid[21];
static gsm_hw_config_t m_cfg;
static uint8_t m_hard_reset_sequence = 0;
static uint32_t current_response_length;
static uint32_t expect_compare_length;
static uint8_t *p_compare_end_str;

gsm_hw_config_t *gsm_hardware_get_configuration(void)
{
    return &m_cfg;
}

void gsm_hardware_initialize(gsm_hw_config_t *config)
{
    // Require callback
    GSM_ASSERT(config);
    GSM_ASSERT(config->io_set);
    GSM_ASSERT(config->uart_control);
    GSM_ASSERT(config->sys_now_ms);
    GSM_ASSERT(config->serial_tx);
    
    memcpy(&m_cfg, config, sizeof(gsm_hw_config_t));
    // Init io
    if (m_cfg.gpio_initialize)
        m_cfg.gpio_initialize();
    
    // Turn off power module
    m_cfg.io_set(m_cfg.gpio.power_en, 0);
    m_cfg.io_set(m_cfg.gpio.reset_pin, 1);
    m_cfg.io_set(m_cfg.gpio.power_key, 0);
    
    // Init uart
    m_cfg.uart_control(true);

    gsm_data_layer_initialize();

    gsm_change_state(GSM_STATE_RESET);

    gsm_manager.timeout_after_reset = GSM_COMMUNICATION_TIMEOUT_SECOND;
}

//static volatile uint32_t m_dma_rx_expected_size = 0;
//void gsm_uart_rx_dma_update_rx_index(bool rx_status)
//{
//    if (rx_status)
//    {
//        m_gsm_atc.atc.recv_buff.index += m_dma_rx_expected_size;
//    }
//    else
//    {
//        m_dma_rx_expected_size = 0;
//        m_gsm_atc.atc.recv_buff.buffer[0] = 0;
//        DEBUG_ERROR("UART RX error, retry received\r\n");
//        NVIC_SystemReset();
//    }
//}

void gsm_uart_handler(void)
{
    m_new_uart_data = true;
}

void gsm_hw_uart_send_raw(uint8_t *raw, uint32_t length)
{
    m_cfg.serial_tx(raw, length);
}

void gsm_hw_layer_run(void)
{
    static uint32_t m_last_poll = 0;
    uint32_t now = m_cfg.sys_now_ms();
    
    bool is_idle_rx = true;
#ifndef IN_BOOTLODAER
    if (m_cfg.is_rx_idle == NULL)
    {
        is_idle_rx = true;
    }
    else 
    {
        is_idle_rx = m_cfg.is_rx_idle();
    }
#endif  
    
    
    // Only poll if GMS idle or timeout is over
    if ((now - m_last_poll) < m_cfg.hw_polling_ms
        || is_idle_rx == false)
    {
        return;
    }

    m_last_poll = now;
    bool has_uart_data = true;

    if (m_gsm_atc.atc.retry_count_atc)
    {
        // Todo : disable only UART IRQ instead of global irq
        __disable_irq();
        if (m_new_uart_data)
        {
            m_new_uart_data = false;
            has_uart_data = false;
        }
        __enable_irq();
        
        // if uart data is available && found string at the end of buffer
        if (has_uart_data == false 
            && (strstr((char *)(m_gsm_atc.atc.recv_buff.buffer), m_gsm_atc.atc.expect_resp_from_atc)))
        {
            bool do_cb = true;
            
            // If AT command need end pattern
            if (m_gsm_atc.atc.expected_response_at_the_end 
                && strlen(m_gsm_atc.atc.expected_response_at_the_end))
            {
                // DEBUG_PRINTF("Expected end %s\r\n", m_gsm_atc.atc.expected_response_at_the_end);
                current_response_length = m_gsm_atc.atc.recv_buff.index;
                expect_compare_length = strlen(m_gsm_atc.atc.expected_response_at_the_end);
                
                // If we received number of rx bytes > length_of (end pattern)
                if (expect_compare_length < current_response_length)
                {
                    /**
                        GSM RX buffer
                                 Byte 1| ................................................      |current_rx_index|.......................End
                    -> End ptr = Byte 1|     ........|current_rx_index -  expect_compare_length|                   
                    */
                    p_compare_end_str = &m_gsm_atc.atc.recv_buff.buffer[current_response_length - expect_compare_length];
                    
                    if ((memcmp(p_compare_end_str, 
                                m_gsm_atc.atc.expected_response_at_the_end, 
                                expect_compare_length) 
                                == 0))
                    {
                        do_cb = true;
                    }
                    else
                    {
                        do_cb = false;
                    }
                }
                else
                {
                    do_cb = false;
                }
            }
            
            // Call callback
            if (do_cb)
            {
                m_gsm_atc.atc.retry_count_atc = 0;
                m_gsm_atc.atc.timeout_atc_ms = 0;
                m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index] = 0;
                
                if (m_gsm_atc.atc.send_at_callback)
                {
                    m_gsm_atc.atc.send_at_callback(GSM_EVENT_OK, 
                                                    m_gsm_atc.atc.recv_buff.buffer);
                }
                
                // Clear buffer data
                m_gsm_atc.atc.recv_buff.index = 0;
                memset(m_gsm_atc.atc.recv_buff.buffer, 
                        0, 
                        sizeof(((sys_ctx_small_buffer_t *)0)->buffer));
            }
        }
        else if (has_uart_data == false)      // Debug only
        {
            #if 0
            char *p = strstr((char *)(m_gsm_atc.atc.recv_buff.buffer), "CME ERROR: ");
            if (p && strstr(p, "\r\n"))
            {
//                DEBUG_VERBOSE("%s", p);
//                if (m_gsm_atc.atc.send_at_callback)
//                {
//                    m_gsm_atc.atc.send_at_callback(GSM_EVENT_ERROR, m_gsm_atc.atc.recv_buff.buffer);
//                }
//                m_gsm_atc.atc.timeout_atc_ms = 0;
            }
            #endif
        }
    }
    
    
    uint32_t diff = m_cfg.sys_now_ms() - m_gsm_atc.atc.current_timeout_atc_ms;
    
    // Check if at command timeout
    if (m_gsm_atc.atc.timeout_atc_ms &&
        diff >= m_gsm_atc.atc.timeout_atc_ms)
    {
        // Decrease reties time
        if (m_gsm_atc.atc.retry_count_atc)
        {
            m_gsm_atc.atc.retry_count_atc--;
        }
        
        // If no more retris
        if (m_gsm_atc.atc.retry_count_atc == 0)
        {
            m_gsm_atc.atc.timeout_atc_ms = 0;

            if (m_gsm_atc.atc.send_at_callback != NULL)
            {
                m_gsm_atc.atc.send_at_callback(GSM_EVENT_TIMEOUT, NULL);
            }
            m_gsm_atc.atc.recv_buff.index = 0;
            m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index] = 0;
        }
        else        // resend at command
        {
            DEBUG_VERBOSE("Diff %u, Resend ATC: %sExpect %s\r\n", 
                        diff, m_gsm_atc.atc.cmd, m_gsm_atc.atc.expect_resp_from_atc);
            
            m_gsm_atc.atc.current_timeout_atc_ms = m_cfg.sys_now_ms();
            m_cfg.serial_tx((uint8_t *)m_gsm_atc.atc.cmd, 
                                strlen(m_gsm_atc.atc.cmd));
        }
    }
    else
    {

//    //    if (m_gsm_atc.atc.recv_buff.index > 32
//    //        && strstr((char*)m_gsm_atc.atc.recv_buff.buffer+10, "CUSD:"))
//    //        {
//    //            DEBUG_VERBOSE("CUSD %s\r\n", m_gsm_atc.atc.recv_buff.buffer);
//    //        }
    }
    
    // If device not in at command mode, but received new message with CRLF 
    // =>> Print out to terminal
    if (m_gsm_atc.atc.retry_count_atc == 0
        && m_gsm_atc.atc.recv_buff.index > 2)
    {
        if (strstr((char *)m_gsm_atc.atc.recv_buff.buffer, "\r\n"))
        {
            DEBUG_WARN("ATC : unhandled %s\r\n", m_gsm_atc.atc.recv_buff.buffer);
            m_gsm_atc.atc.recv_buff.index = 0;
            m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index] = 0;
        }
    }
    
    // Update task timer
    m_last_poll = m_cfg.sys_now_ms();
}

void gsm_change_hw_polling_interval(uint32_t ms)
{
    m_cfg.hw_polling_ms = ms;
}

void gsm_hw_layer_reset_rx_buffer(void)
{
    memset(m_gsm_atc.atc.recv_buff.buffer, 
            0, 
            sizeof(((sys_ctx_small_buffer_t *)0)->buffer));
    m_gsm_atc.atc.recv_buff.index = 0;
    m_gsm_atc.atc.recv_buff.state = SYS_CTX_BUFFER_STATE_IDLE;
    m_gsm_atc.atc.retry_count_atc = 0;
    m_gsm_atc.atc.timeout_atc_ms = 0;
    
    if (m_cfg.serial_rx_flush)
    {
        m_cfg.serial_rx_flush();
    }
}

void gsm_hw_send_at_cmd(char *cmd, char *expect_resp,
                        char *expected_response_at_the_end_of_response, 
                        uint32_t timeout,
                        uint8_t retry_count, gsm_send_at_cb_t callback)
{
    if (timeout == 0 || callback == NULL)
    {
        m_cfg.serial_tx((uint8_t *)cmd, strlen(m_gsm_atc.atc.cmd));
        return;
    }

    if (strlen(cmd) < 64)
    {
        DEBUG_VERBOSE("ATC: %s", cmd);
    }

    m_gsm_atc.atc.cmd = cmd;
    
    m_gsm_atc.atc.expect_resp_from_atc = expect_resp;
    m_gsm_atc.atc.expected_response_at_the_end = expected_response_at_the_end_of_response;
    m_gsm_atc.atc.retry_count_atc = retry_count;
    m_gsm_atc.atc.send_at_callback = callback;
    m_gsm_atc.atc.timeout_atc_ms = timeout;
    m_gsm_atc.atc.current_timeout_atc_ms = m_cfg.sys_now_ms();
    memset(m_gsm_atc.atc.recv_buff.buffer, 0, sizeof(((sys_ctx_small_buffer_t *)0)->buffer));
    m_gsm_atc.atc.recv_buff.index = 0;
    m_gsm_atc.atc.recv_buff.state = SYS_CTX_BUFFER_STATE_IDLE;

    m_cfg.serial_tx((uint8_t *)cmd, strlen(cmd));
}


uint32_t prev_index = 0;
void gsm_hw_layer_on_rx_callback(uint8_t *data, uint32_t length)
{
    if (length)
    {
        m_new_uart_data = true;
        prev_index = m_gsm_atc.atc.recv_buff.index;
        for (uint32_t i = 0; i < length; i++)
        {
            m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index++] = data[i];
            
            // Check if data overflow
            if (m_gsm_atc.atc.recv_buff.index >= sizeof(((sys_ctx_small_buffer_t *)0)->buffer))
            {
                DEBUG_ERROR("GSM RX overflow\r\n");
                m_gsm_atc.atc.recv_buff.index = 0;
                m_gsm_atc.atc.recv_buff.buffer[0] = 0;
                return;
            }
        }
    }
}

void gsm_hard_reset_default_sequence(void)
{
    m_hard_reset_sequence = 0;
}


void gsm_hard_reset_sequence(gsm_hardreset_complete_callback_t callback)
{
    static uint32_t hard_reset_interval = 0;
    if (m_cfg.sys_now_ms() - hard_reset_interval >= (uint32_t)1000)
    {
        hard_reset_interval = sys_get_ms();
    }
    else
    {
        return;
    }
    
    DEBUG_INFO("GSM hard reset step %d\r\n", m_hard_reset_sequence);
    switch (m_hard_reset_sequence)
    {
    case 0:
    {
        // Power off // add a lot of delay in power off state, due to reserve time for measurement sensor data
        // If vbat is low, sensor data will be incorrect
        m_hard_reset_sequence++;    
        // Deinit uart
        if (m_cfg.uart_control)
        {
            m_cfg.uart_control(false);
        }
        
        if (m_cfg.io_set)
        {
            m_cfg.io_set(m_cfg.gpio.power_en, 0);
            m_cfg.io_set(m_cfg.gpio.power_key, 0);
            m_cfg.io_set(m_cfg.gpio.reset_pin, 1);
        }
    }
        break;

    case 1:
        DEBUG_INFO("Gsm power on\r\n");
        
        // Turn on mosfet power 
        if (m_cfg.io_set)
        {
            m_cfg.io_set(m_cfg.gpio.reset_pin, 0);
            m_cfg.io_set(m_cfg.gpio.power_en, 1);
        }
        
        m_hard_reset_sequence++;
        break;

    case 2:
        m_hard_reset_sequence++;
        break;

    case 3:
        DEBUG_INFO("Pulse power key\r\n");
        /* Generate pulse 1-0-1 (|_|) to Power on module, min 1s  */
        if (m_cfg.io_set)
        {
            m_cfg.io_set(m_cfg.gpio.power_key, 1);
        }
        m_hard_reset_sequence++;
        break;

    case 4:
        if (m_cfg.io_set)
        {
            m_cfg.io_set(m_cfg.gpio.power_key, 0);
            m_cfg.io_set(m_cfg.gpio.reset_pin, 0);
        }
        gsm_manager.timeout_after_reset = GSM_MAX_DISCONNECT_COUNTER_SECOND;
        m_hard_reset_sequence++;
        break;

    case 5:
    case 6:
        m_hard_reset_sequence++;
        break;

    case 7:
        m_hard_reset_sequence = 0;
        // Power on module
        if (m_cfg.uart_control)
        {
            m_cfg.uart_control(true);
        }
        if (callback)
        {
            callback();
        }
        break;

    default:
        break;
    }
}

char *gsm_get_sim_imei(void)
{
    return m_sim_imei;
}

char *gsm_get_sim_ccid(void)
{
    return m_sim_ccid;
}

char *gsm_get_module_imei(void)
{
    //    #warning "Hardcode imei 123456789012345"
    //    return  "123456789012345";

    return m_gsm_imei;
}

void gsm_set_sim_imei(char *imei)
{
    memcpy(m_sim_imei, imei, 15);
    m_sim_imei[15] = 0;
}

void gsm_set_module_imei(char *imei)
{
    memcpy(m_gsm_imei, imei, 15);
    m_gsm_imei[15] = 0;
}

void gsm_set_network_operator(char *nw_operator)
{
    snprintf(m_nw_operator, sizeof(m_nw_operator) - 1, "%s", nw_operator);
    m_nw_operator[sizeof(m_nw_operator) - 1] = 0;
}

char *gsm_get_network_operator(void)
{
    return m_nw_operator;
}

static uint8_t m_csq, m_csq_percent;

void gsm_set_csq(uint8_t csq)
{
    m_csq = csq;
    if (m_csq == 99) // 99 is invalid CSQ
    {
        m_csq = 0;
    }

    if (m_csq > 31)
    {
        m_csq = 31;
    }

    if (m_csq < 10)
    {
        m_csq = 10;
    }

    m_csq_percent = ((m_csq - 10) * 100) / (31 - 10);
}

uint8_t gsm_get_csq_in_percent()
{
    return m_csq_percent;
}
