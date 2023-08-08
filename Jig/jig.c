#include "jig.h"
#include "main.h"
#include "usart.h"
#include "hardware.h"
#include "measure_input.h"
#include "string.h"
#include "adc.h"
#include <stdarg.h>
#include "stdio.h"
#include <string.h>
#include <ctype.h>
#include "umm_malloc.h"
#include "app_bkup.h"
#include "sys_ctx.h"
#include "app_eeprom.h"
#include "app_debug.h"

#define JIG_DEFAULT_TIMEOUT_MS 1000
#define JIG_RS485_RX485_TX_BUFFER_SIZE	128
#define JIG_RS485_RX485_RX_BUFFER_SIZE	128

static jig_rs485_buffer_t m_jig_buffer;
volatile uint32_t jig_timeout_ms = JIG_DEFAULT_TIMEOUT_MS;
static bool m_jig_in_test_mode = false;

static char m_jig_buffer_tx[JIG_RS485_RX485_TX_BUFFER_SIZE];
static char m_jig_buffer_rx[JIG_RS485_RX485_RX_BUFFER_SIZE];

void jig_print(const char *fmt,...)
{
	int     n;

    char *p = (char*)m_jig_buffer_tx;
    int size = 64;
    va_list args;

    va_start (args, fmt);
    n = vsnprintf(p, size, fmt, args);
    if (n > (int)size) 
    {
        DEBUG_INFO("%.*s\r\n", size, m_jig_buffer_tx);
        usart_lpusart_485_send((uint8_t*)m_jig_buffer_tx, size);
    } 
    else if (n > 0) 
    {
        DEBUG_INFO("%.*s", n, m_jig_buffer_tx);
        usart_lpusart_485_send((uint8_t*)m_jig_buffer_tx, n);
    }
    va_end(args);
}

bool jig_is_in_test_mode(void)
{
	return m_jig_in_test_mode;
}

void jig_set_test_mode(bool set)
{
    m_jig_in_test_mode = set;
}

void jig_start(void)
{
	m_jig_buffer.rx_idx = 0;
	m_jig_buffer.tx_ptr = m_jig_buffer_tx;
	m_jig_buffer.rx_ptr = m_jig_buffer_rx;

	HARDWARE_RS485_POWER_EN(1);
	usart_rs485_control(1);
	sys_delay_ms(200);
    jig_timeout_ms = JIG_DEFAULT_TIMEOUT_MS;
    sys_ctx()->status.timeout_wait_rs485_config_message = 0;
	while (jig_timeout_ms)
	{
		// Reload watchdog
		WRITE_REG(IWDG->KR, LL_IWDG_KEY_RELOAD);
		if (m_jig_buffer.rx_idx && strstr(m_jig_buffer.rx_ptr, "{\"test\":1}"))
		{
            m_jig_buffer.rx_idx = 0;
			m_jig_in_test_mode = true;
		}
		else		// Exit test mode
		{
            break;
		}
	}
    
#if ENTER_JIG_TEST == 1
    m_jig_in_test_mode = true;
#endif
    
    if (m_jig_in_test_mode == false)
    {
        m_jig_buffer.rx_ptr = NULL;
        m_jig_buffer.tx_ptr = NULL;
        HARDWARE_RS485_POWER_EN(0);
    }
    else
    {
        jig_print("Enter test mode\r\n");
        HARDWARE_RS485_POWER_EN(1);
    }
}

void jig_uart_insert(uint8_t data)
{
	if (m_jig_buffer.rx_ptr && data)
	{
		m_jig_buffer.rx_ptr[m_jig_buffer.rx_idx++] = data;
		if (m_jig_buffer.rx_idx >= JIG_RS485_RX485_RX_BUFFER_SIZE)
		{
			memset(&m_jig_buffer, 0, sizeof(m_jig_buffer));
		}
	}
}

bool jig_found_cmd_sync_data_to_host(void)
{
	if (m_jig_buffer.rx_ptr == NULL)
	{
		m_jig_buffer.rx_ptr = m_jig_buffer_rx;
        memset(m_jig_buffer_rx, 0, sizeof(m_jig_buffer_rx));
		m_jig_buffer.rx_ptr[0] = 0;
	}
	
	if (strstr((char*)m_jig_buffer.rx_ptr, "Hawaco.Datalogger.PingMessage"))
	{
        m_jig_buffer.rx_ptr = NULL;
		memset(&m_jig_buffer, 0, sizeof(m_jig_buffer));
		return true;
	}
	return false;
}

bool jig_found_cmd_change_server(char **ptr, uint32_t *size)
{
    // Format Server:http://iot.wilab.vn\r\n
    *size = 0;
    char *server = strstr((char*)m_jig_buffer.rx_ptr, "Server:http://");
	if (server && strstr(server, "\r\n"))
	{
        *ptr = server + 7;      // 7 = strlen("Server:")
        *size = strstr(server, "\r\n") - *ptr;
        if (*size >= (APP_EEPROM_MAX_SERVER_ADDR_LENGTH-1))
        {
            return false;
        }
        return true;
	}
    
	server = strstr((char*)m_jig_buffer.rx_ptr, "Server:https://");
    if (server && strstr(server, "\r\n"))
	{
        *ptr = server + 7;      // 7 = strlen("Server:")
        *size = strstr(server, "\r\n") - *ptr;
        if (*size >= (APP_EEPROM_MAX_SERVER_ADDR_LENGTH-1))
        {
            return false;
        }
        return true;
	}
	return false;
}

bool jig_found_cmd_set_default_server_server(char **ptr, uint32_t *size)
{
    // Format Default:http://iot.wilab.vn\r\n
    *size = 0;
    char *server = strstr((char*)m_jig_buffer.rx_ptr, "Default:http://");
	if (server && strstr(server, "\r\n"))
	{
        *ptr = server + 7;      // 7 = strlen("Server:")
        *size = strstr(server, "\r\n") - *ptr;
        if (*size >= (APP_EEPROM_MAX_SERVER_ADDR_LENGTH-1))
        {
            return false;
        }
        return true;
	}
    server = strstr((char*)m_jig_buffer.rx_ptr, "Default:https://");
    if (server && strstr(server, "\r\n"))
	{
        *ptr = server + 7;      // 7 = strlen("Server:")
        *size = strstr(server, "\r\n") - *ptr;
        if (*size >= (APP_EEPROM_MAX_SERVER_ADDR_LENGTH-1))
        {
            return false;
        }
        return true;
	}
	return false;
}

bool jig_found_cmd_get_config(void)
{
    // Format GetConfig\r\n
    char *config = strstr((char*)m_jig_buffer.rx_ptr, "GetConfig\r\n");
	if (config)
    {
        return true;
    }
	return false;
}

bool jig_found_cmd_shutdown(void)
{
    return strstr((char*)m_jig_buffer.rx_ptr, "Shutdown\r\n") ? true : false;
}


bool jig_found_cmd_test(void)
{
    // Format GetConfig\r\n
    char *config = strstr((char*)m_jig_buffer.rx_ptr, "{\"test\":1}");
	if (config)
    {
        DEBUG_INFO("Enter test mode\r\n");
        memset(m_jig_buffer_rx, 0, sizeof(m_jig_buffer_rx));
        m_jig_buffer.tx_ptr = m_jig_buffer_tx;
        m_jig_in_test_mode = 1;
        m_jig_buffer.rx_idx = 0;
        return true;
    }
	return false;
}

bool jig_found_cmd_boot(void)
{
    return strstr((char*)m_jig_buffer.rx_ptr, "reboot\r\n") ? true : false;
}


void jig_release_memory(void)
{
	if (m_jig_buffer.rx_ptr)
	{
        m_jig_buffer.rx_ptr = NULL;
        memset(m_jig_buffer_rx, 0, sizeof(m_jig_buffer_rx));
        memset(m_jig_buffer_tx, 0, sizeof(m_jig_buffer_tx));
		memset(&m_jig_buffer, 0, sizeof(m_jig_buffer));
	}
}



