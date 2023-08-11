#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include <stdint.h>
//#include <stdio.h>

#define DEBUG_LEVEL_ALL         0
#define DEBUG_LEVEL_VERBOSE     1
#define DEBUG_LEVEL_INFO        2
#define DEBUG_LEVEL_WARN        3
#define DEBUG_LEVEL_ERROR       4

#define DEBUG_LEVEL_NO			5
#define DEBUG_LEVEL             DEBUG_LEVEL_INFO
#define DEBUG_BY_UART			1
//#define DEBUG_RTT
#define APP_DEBUG_PRINTF_BUFFER_SIZE	1024

#if 0
#define KNRM  "\x1B[0m"
#define KRED  RTT_CTRL_TEXT_RED
#define KGRN  RTT_CTRL_TEXT_GREEN
#define KYEL  RTT_CTRL_TEXT_YELLOW
#define KBLU  RTT_CTRL_TEXT_BLUE
#define KMAG  RTT_CTRL_TEXT_MAGENTA
#define KCYN  RTT_CTRL_TEXT_CYAN
#define KWHT  RTT_CTRL_TEXT_WHITE
#else
#define KNRM  "$,2,\x1B[0m"
#define KRED  "$,2,\x1B[31m"
#define KGRN  "$,2,\x1B[32m"
#define KYEL  "$,2,\x1B[33m"
#define KBLU  "$,2,\x1B[34m"
#define KMAG  "$,2,\x1B[35m"
#define KCYN  "$,2,\x1B[36m"
#define KWHT  "$,2,\x1B[37m"
#endif

//#define DEBUG_RTT               1
#define DEBUG_RAW               				app_debug_print_raw
#define DEBUG_DUMP                           	app_debug_dump

unsigned int log_get_sys_ms(void);

void log_uart_put(void *data, unsigned int size);
#if (DEBUG_LEVEL_VERBOSE >= DEBUG_LEVEL)
#ifdef DEBUG_BY_UART
#define DEBUG_VERBOSE(s, args...)               app_debug_print_raw(KMAG "<%u> [I] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#else
#define DEBUG_VERBOSE(s, args...)               app_debug_print_raw(KMAG "<%u> [I] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#endif
#else
#define DEBUG_VERBOSE(s, args...)               app_debug_print_nothing(s, ##args)
#endif

#if (DEBUG_LEVEL_INFO >= DEBUG_LEVEL)
#ifdef DEBUG_BY_UART
#define DEBUG_INFO(s, args...)                  app_debug_print_raw(KGRN "<%u> [I] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#else
#define DEBUG_INFO                 				app_debug_print_raw(KGRN "<%u> [I] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#endif
#else
#define DEBUG_INFO(s, args...)                  app_debug_print_nothing(s, ##args)
#endif

#if (DEBUG_LEVEL_ERROR >= DEBUG_LEVEL)
#ifdef DEBUG_BY_UART
#define DEBUG_ERROR(s, args...)                 app_debug_print_raw(KRED "<%u> [I] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#else
#define DEBUG_ERROR(s, args...) 				app_debug_print_raw(KRED "<%u> [I] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#endif
#else
#define DEBUG_ERROR(s, args...)                 app_debug_print_nothing(s, ##args)
#endif

#if (DEBUG_LEVEL_WARN >= DEBUG_LEVEL)
#ifdef DEBUG_BY_UART
#define DEBUG_WARN(s, args...)                  app_debug_print_raw(KYEL "<%u> [W] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#else
#define DEBUG_WARN(s, args...)                  app_debug_print_raw(KYEL "<%u> [W] %s : " s KNRM,  log_get_sys_ms(), "", ##args)
#endif
#else
#define DEBUG_WARN(s, args...)                  app_debug_print_nothing(s, ##args)
#endif


#ifdef DEBUG_BY_UART
#define DEBUG_COLOR(color, s, args...)       	app_debug_print_raw(color s KNRM, ##args)
#else
#define DEBUG_COLOR(color, s, args...)       	app_debug_print_raw(color s KNRM, ##args)
#endif

#define DEBUG_PRINTF            app_debug_print
#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF(String...)	SEGGER_RTT_printf(0, String)
#endif

#ifndef DEBUG_FLUSH
#define DEBUG_FLUSH()      while(0)
#endif

typedef unsigned int (*app_debug_get_ms_cb_t)(void);
typedef void (*app_debug_puts_cb_t)(void *data, unsigned int size);

typedef struct
{
	app_debug_get_ms_cb_t get_ms;
	app_debug_puts_cb_t put_cb;
} app_debug_callback_t;

void app_debug_set_callback(app_debug_callback_t debug_callback);

int32_t app_debug_print_nothing(const char *fmt,...);

int32_t app_debug_print(const char *fmt,...);

int32_t app_debug_print_raw(const char *fmt,...);

void app_debug_dump(const void* data, int len, const char* string, ...);

#endif /* APP_DEBUG_H */
