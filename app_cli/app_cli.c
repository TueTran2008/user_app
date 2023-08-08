#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#if 1

#include "app_debug.h"
#include "board_hw.h"
//#include "app_flash.h"

#define PRINTF_OVER_RTT             1
#define PRINTF_OVER_UART            (!PRINTF_OVER_RTT)


#include "app_cli.h"
#include "app_shell.h"
#if PRINTF_OVER_RTT
#include "SEGGER_RTT.h"
#endif
#include "host_data_layer.h"
#include "main.h"

#if PRINTF_OVER_RTT
int rtt_custom_printf(const char *format, ...)
{
    int r;
    va_list ParamList;

    va_start(ParamList, format);
    r = SEGGER_RTT_vprintf(0, format, &ParamList);
    va_end(ParamList);
    
    return r;
}
#else
#define rtt_custom_printf           DEBUG_RAW
#endif

static shell_context_struct m_user_context;
static int32_t cli_reset_system(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_dump_ip(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_get_enter_test_mode(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_test_ui(p_shell_context_t context, int32_t argc, char **argv);
//static int32_t cli_flash_test(p_shell_context_t context, int32_t argc, char **argv);

static const shell_command_context_t cli_command_table[] = 
{
    {"reset",           "\treset: reset system\r\n",                            cli_reset_system,                           0},   
    {"ip",              "\tip : Dump gateway ip\r\n",                           cli_dump_ip,                                0},
	{"test",          	"\ttest : enter/exit test mode\r\n",                    cli_get_enter_test_mode,                    1},
    {"ui",              "\tui: test ui\r\n",                                    cli_test_ui,                                1},
};

void app_cli_puts(uint8_t *buf, uint32_t len)
{
#if PRINTF_OVER_RTT
    SEGGER_RTT_Write(0, buf, len);
#else
    extern void app_debug_uart_print(uint8_t *data, uint32_t len);
    app_debug_uart_print(buf, len);
#endif
}

void app_cli_poll()
{
    app_shell_task();
}


extern uint8_t get_debug_rx_data(void);
void app_cli_gets(uint8_t *buf, uint32_t len)
{
#if PRINTF_OVER_RTT
    for (uint32_t i = 0; i < len; i++)
    {
        buf[i] = 0xFF;
    }
        
    if (!SEGGER_RTT_HASDATA(0))
    {
        return;
    }
    
    int read = SEGGER_RTT_Read(0, buf, len);
    if (read > 0 && read < len)
    {
        for (uint32_t i = read; i < len; i++)
        {
            buf[i] = 0xFF;
        }
    }
#else
	for (uint32_t i = 0; i < len; i++)
	{
		buf[i] = get_debug_rx_data();
	}
#endif
}


void app_cli_start()
{
    app_shell_set_context(&m_user_context);
    app_shell_init(&m_user_context,
                   app_cli_puts,
                   app_cli_gets,
                   rtt_custom_printf,
                   ">",
                   true);

    /* Register CLI commands */
    for (int i = 0; i < sizeof(cli_command_table) / sizeof(shell_command_context_t); i++)
    {
        app_shell_register_cmd(&cli_command_table[i]);
    }

    /* Run CLI task */
    app_shell_task();
}

/* Reset System */
static int32_t cli_reset_system(p_shell_context_t context, int32_t argc, char **argv)
{
//    DEBUG_INFO("System reset\r\n");
    board_hw_reset();
    return 0;
}

static int32_t cli_dump_ip(p_shell_context_t context, int32_t argc, char **argv)
{
    host_data_layer_request_ip();
    return 0;
}

extern uint32_t reset_reason;
static int32_t cli_get_enter_test_mode(p_shell_context_t context, int32_t argc, char **argv)
{
    if (strstr(argv[1], "enter"))
    {
        DEBUG_INFO("Enter test mode\r\n");
        host_data_layer_set_test_mode(true);
    }
    else if (strstr(argv[1], "quit"))
    {
        DEBUG_INFO("Exit test mode\r\n");
        host_data_layer_set_test_mode(false);
    }
    return 0;
}

extern void process_ui_msg(char *msg);
static int32_t cli_test_ui(p_shell_context_t context, int32_t argc, char **argv)
{
    process_ui_msg(argv[1]);
    return 0;
}

#endif /* APP_CLI_ENABLE */
