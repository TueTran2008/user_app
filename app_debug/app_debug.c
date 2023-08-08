#include "app_debug.h"
#include <stdarg.h>
#include "stdio.h"
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "usart.h"
//#include "gd32e10x.h"

//#define SEGGER_RTT_PRINTF_BUFFER_SIZE 256
static char m_debug_buffer[SEGGER_RTT_PRINTF_BUFFER_SIZE];
static uint8_t m_debug_level = DEBUG_LEVEL;
extern uint32_t sys_get_ms(void);

int app_debug_rtt_nothing(const char *fmt,...)
{
    return -1;
}

int app_debug_rtt(const char *fmt,...)
{
    int     n = 0;

    if (m_debug_level == DEBUG_LEVEL_NOTHING)
    {
        return n;
    }
    
    char *p = &m_debug_buffer[0];
    int size = SEGGER_RTT_PRINTF_BUFFER_SIZE;
    int timestamp_size;

    p += sprintf(m_debug_buffer, "<%u>: ", sys_get_ms());
    timestamp_size = (p-m_debug_buffer);
    size -= timestamp_size;
    va_list args;

    va_start (args, fmt);
    n = vsnprintf(p, size, fmt, args);
    if (n > (int)size) 
    {
        SEGGER_RTT_Write(0, m_debug_buffer, size + timestamp_size);
#if(LOG_485)
        usart_lpusart_485_send((uint8_t*)m_debug_buffer, size + timestamp_size);
#endif
    } 
    else if (n > 0) 
    {
        SEGGER_RTT_Write(0, m_debug_buffer, n + timestamp_size);
#if(LOG_485)
        usart_lpusart_485_send((uint8_t*)m_debug_buffer, n + timestamp_size);
#endif
    }
    va_end(args);
    
    return n;
}

int app_debug_rtt_raw(const char *fmt,...)
{
    int     n = 0;

    if (m_debug_level == DEBUG_LEVEL_NOTHING)
    {
        return n;
    }
    
    
    char *p = &m_debug_buffer[0];
    int size = SEGGER_RTT_PRINTF_BUFFER_SIZE;
    va_list args;

    va_start (args, fmt);
    n = vsnprintf(p, size, fmt, args);
    if (n > (int)size) 
    {
        SEGGER_RTT_Write(0, m_debug_buffer, size);
#if(LOG_485)
        usart_lpusart_485_send((uint8_t*)m_debug_buffer, size);
#endif
    } 
    else if (n > 0) 
    {
        SEGGER_RTT_Write(0, m_debug_buffer, n);
#if(LOG_485)
        usart_lpusart_485_send((uint8_t*)m_debug_buffer, n);
#endif
    }
    va_end(args);
    
    return n;
}

void app_debug_dump(const void* data, int len, const char* string, ...)
{
	uint8_t* p = (uint8_t*)data;
    uint8_t  buffer[16];
    int32_t i_len;
    int32_t i;
    
    if (m_debug_level == DEBUG_LEVEL_NOTHING)
    {
        return;
    }
    
//    DEBUG_RAW("%s %u bytes\n", string, len);
    while (len > 0)
    {
        i_len = (len > 16) ? 16 : len;
        memset(buffer, 0, 16);
        memcpy(buffer, p, i_len);
        for (i = 0; i < 16; i++)
        {
            if (i < i_len)
                DEBUG_RAW("%02X ", buffer[i]);
            else
                DEBUG_RAW("   ");
        }
        DEBUG_RAW("\t");
        for (i = 0; i < 16; i++)
        {
            if (i < i_len)
            {
                if (isprint(buffer[i]))
				{
                    DEBUG_RAW("%c", (char)buffer[i]);
				}
                else
				{
                    DEBUG_RAW(".");
				}
            }
            else
                DEBUG_RAW(" ");
        }
        DEBUG_RAW("\r\n");
        len -= i_len;
        p += i_len;
    }
}

void app_debug_disable_all(void)
{
    //m_debug_level = DEBUG_LEVEL_NOTHING;
}

bool app_debug_is_enable(void)
{
    return (m_debug_level == DEBUG_LEVEL_NOTHING);
}

