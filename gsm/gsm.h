#ifndef GSM_H
#define GSM_H

/**
 * \defgroup        gsm GSM
 * \brief           All gsm function process here
 * \{
 */

#include <stdbool.h>
#include "sys_ctx.h"

#define GSM_ON 1
#define GSM_OFF 2

#define GSMIMEI 0
#define SIMIMEI 1

#define MAX_GSMRESETSYSTEMCOUNT 600 //10 phut

#define MODEM_BUFFER_SIZE 2048

#define MODEM_IDLE 0
#define MODEM_ERROR 1
#define MODEM_READY 2
#define MODEM_LISTEN 3
#define MODEM_ONLINE 4
#define MODEM_DIAL 5
#define MODEM_HANGUP 6

#define TIMEZONE        0

#define __GSM_SLEEP_MODE__ 1
#define __USED_HTTP__ 0

#define GSM_SMS_ENABLE                                  1
#define GSM_READ_SMS_ENABLE                             0
#define GSM_MAX_SMS_CONTENT_LENGTH                      160
#define GSM_MAX_SMS_PHONE_LENGTH                        16
#define GSM_MIN_SMS_PHONE_LENGTH                        9
#define GSM_MAX_DISCONNECT_COUNTER_SECOND               80
#define GSM_CSQ_INVALID                                 99

#define GSM_HARDWARE_PIN_NOT_SUPPORT (0xFFFFFFFF)
#define GSM_ASSERT(x)    (void)x        // assert(x)
    
#define GSM_HARDWARE_DEFAULT_CONFIG()   \
                                        {   .delay = NULL,  \
                                            .io_set = NULL, \
                                            .io_get = NULL, \
                                            .uart_control = NULL,    \
                                            .gpio_initialize = NULL,    \
                                            .serial_tx = NULL,  \
                                            .serial_rx = NULL,  \
                                            .serial_rx_flush = NULL, \
                                            .gpio.power_en = GSM_HARDWARE_PIN_NOT_SUPPORT,  \
                                            .gpio.power_key = GSM_HARDWARE_PIN_NOT_SUPPORT,  \
                                            .gpio.status_pin = GSM_HARDWARE_PIN_NOT_SUPPORT,  \
                                            .gpio.ri_pin = GSM_HARDWARE_PIN_NOT_SUPPORT,  \
                                            .gpio.reset_pin = GSM_HARDWARE_PIN_NOT_SUPPORT,  \
                                            .sys_now_ms = NULL,    \
                                            .hw_polling_ms = 5, \
                                            .is_rx_idle = NULL, \
                                        }
    
typedef enum
{
    GSM_EVENT_OK = 0,       // GSM response dung
    GSM_EVENT_TIMEOUT,      // Het timeout ma chua co response
    GSM_EVENT_ERROR,        // GSM response ko dung,
    GSM_EVENT_CONTINUES,    // Goi truc tiep khong thong qua hardware callback
} gsm_response_event_t;

typedef enum
{
    GSM_STATE_OK,
    GSM_STATE_RESET,
    GSM_STATE_SEND_SMS,
    GSM_STATE_READ_SMS,
    GSM_STATE_POWER_ON,
    GSM_STATE_REOPEN_PPP,
    GSM_STATE_GET_BTS_INFO,
//    GSM_STATE_SEND_ATC,
    GSM_STATE_GOTO_SLEEP,
    GSM_STATE_WAKEUP,
    GSM_STATE_AT_MODE_IDLE,
    GSM_STATE_SLEEP,
    GSM_STATE_HTTP_GET,
    GSM_STATE_HTTP_POST,
    GSM_STATE_FILE_PROCESS,
    GSM_STATE_SNTP_UPDATE,
    GSM_STATE_MAX
} gsm_state_t;

typedef enum
{
    GSM_AT_MODE = 1,
    GSM_PPP_MODE
} gsm_at_mode_t;

typedef enum
{
    GSM_INTERNET_MODE_PPP_STACK,
    GSM_INTERNET_MODE_AT_STACK
} gsm_internet_mode_t;

typedef struct
{
    gsm_state_t state;
    uint8_t step;
    uint8_t gsm_ready;
    uint8_t timeout_after_reset;
    uint8_t access_tech;
} gsm_manager_t;

typedef struct
{
    char phone_number[GSM_MAX_SMS_PHONE_LENGTH];
    char message[GSM_MAX_SMS_CONTENT_LENGTH];
    uint8_t need_to_send;
    uint8_t retry_count;
} gsm_sms_msg_t;


typedef enum
{
    GSM_ANALYZER_OK,
    GSM_ANALYZER_ERROR,
    GSM_ANALYZER_QUERY_AGAIN,
    GSM_ANALYZER_PROCESS_SUB_SEQ
} gsm_analyzer_err_t;

typedef gsm_analyzer_err_t (*gsm_at_error_callback_t)(gsm_response_event_t event, void *response_buffer);
typedef gsm_analyzer_err_t (*gsm_at_analyzer_response_t)(void *response_buffer);
typedef void (*gsm_send_at_cb_t)(gsm_response_event_t event, void *response_buffer);


typedef struct
{
    char *cmd;
    char *response;
    char *expected_at_end;
    uint32_t timeout_ms;
    uint32_t retries;
    gsm_send_at_cb_t callback;
    gsm_at_error_callback_t error_cb;
    gsm_at_analyzer_response_t analyzer_response;
} gsm_at_lut_t;

/**
 * @brief       GSM power on success callback
 */
typedef void (*gsm_power_on_cb_t)(void);

/**
 * @brief       GSM IO control callback
 * @param[in]   pin Pin number
 * @param[in]   on_off Pin level
 */
typedef void (*gsm_hw_io_set_cb_t)(uint32_t pin, bool on_off);

/**
 * @brief       GSM IO read callback
 * @param[in]   pin Pin number
 * @retval      Pin level
 */
typedef bool (*gsm_hw_io_get_cb_t)(uint32_t pin);

/**
 * @brief       GSM delay callback
 */
typedef void (*gsm_hw_delay_cb_t)(uint32_t ms);

/**
 * @brief       GSM serial tx data callback
 * @param[in]   data Pointer to sending buffer
 * @param[in]   len Number of bytes send to serial port
 * @retval      Number of bytes written
 */
typedef uint32_t (*gsm_hw_uart_tx_cb_t)(uint8_t *data, uint32_t len);

/**
 * @brief       GSM serial rx data callback
 * @param[out]  data Pointer to read buffer
 * @param[in]   len Number of bytes want to from serial port
 * @retval      Number of bytes read
 */
typedef uint16_t (*gsm_hw_uart_rx_cb_t)(uint8_t *data, uint16_t len);

/**
 * @brief       GSM flush all data in serial port
 */
typedef void (*gsm_hw_uart_rx_flush_t)(void);

/**
 * @brief       GSM serial initialize callback
 */
typedef void (*gsm_hw_uart_initialize_cb_t)(bool enable);

/**
 * @brief       GSM GPIO initialize callback
 */
typedef void (*gsm_hw_gpio_initialize_t)(void);

/**
 * @brief       GSM get current system tick in milisecond
 * @retval      System tick in miliseconds
 */
typedef uint32_t (*gsm_sys_now_cb_t)(void);

/**
 * @brief       GSM idle line detect
 * @retval      TRUE GSM RX idle
 *              FALSE GSM RX is on going
 */
typedef bool (*gsm_hw_is_idle_line_detect_cb_t)(void);


/**
 * @brief       Hardware power on complete callback
 */
typedef void (*gsm_hardreset_complete_callback_t)(void);

typedef struct
{
    uint32_t power_en;      // by trans or mosfet
    uint32_t power_key;
    uint32_t status_pin;
    uint32_t ri_pin;
    uint32_t reset_pin;
} gsm_gpio_t;

typedef struct
{
    gsm_hw_delay_cb_t delay;
    gsm_hw_io_set_cb_t io_set;
    gsm_hw_io_get_cb_t io_get;
    gsm_hw_uart_initialize_cb_t uart_control;
    gsm_hw_gpio_initialize_t gpio_initialize;
    gsm_hw_uart_tx_cb_t serial_tx;
    gsm_hw_uart_rx_cb_t  serial_rx;
    gsm_hw_uart_rx_flush_t serial_rx_flush;
    gsm_gpio_t gpio;
    gsm_sys_now_cb_t sys_now_ms;
    uint32_t hw_polling_ms;
    gsm_hw_is_idle_line_detect_cb_t is_rx_idle;
} gsm_hw_config_t;


typedef struct
{
    char *cmd;
    char *expect_resp_from_atc;
    char *expected_response_at_the_end;
    uint32_t timeout_atc_ms;
    uint32_t current_timeout_atc_ms;
    uint8_t retry_count_atc;
    sys_ctx_small_buffer_t recv_buff;
    gsm_send_at_cb_t send_at_callback;
} gsm_at_cmd_t;

typedef struct
{
    gsm_at_cmd_t atc;
} gsm_hardware_t;


void gsm_hw_send_at_cmd(char *cmd, char *expect_resp, char *expect_resp_at_the_end,
                        uint32_t timeout,uint8_t retry_count, gsm_send_at_cb_t callback);

/**
 * @brief       Init gsm hardware
 * @param[in]   config GSM config
 */
void gsm_hardware_initialize(gsm_hw_config_t *config);

/**
 * @brief       Get gsm hardware configuration
 * @retval      Pointer to gsm configuration
 */
gsm_hw_config_t *gsm_hardware_get_configuration(void);

/**
 * @brief       Reset default gsm reset counter
 */
void gsm_hard_reset_default_sequence(void);

/**
 * @brief       Gsm hardware reset sequence
 */
void gsm_hard_reset_sequence(gsm_hardreset_complete_callback_t callback);
    
/**
 * @brief       GSM UART RX callback
 * @param[in]   data serial data
 * @param[in]   length Data len
 */
void gsm_hw_layer_on_rx_callback(uint8_t *data, uint32_t length);
    
/**
 * @brief       Init gsm data layer
 */
void gsm_data_layer_initialize(void);

/**
 * @brief       GSM data layer state machine
 */
void gsm_manager_tick(void);

/**
 * @brief       GSM hardware polling
 */
void gsm_hardware_tick(void);

/**
 * @brief       Query sms state machine
 */
void gsm_query_sms(void);

/*!
 * @brief           Send sms to phone number
 * @param[in]       phone_number : Des phone number
 * @param[in]       message : A message send to phone number
 */
bool gsm_send_sms(char *phone_number, char *message);


/*!
 * @brief           Get SMS buffer size
 * @retval          Pointer to sms buffer size
 */
uint32_t gsm_get_max_sms_memory_buffer(void);

/*!
 * @brief           Get SMS context
 * @retval          Pointer to sms memory buffer
 */
gsm_sms_msg_t *gsm_get_sms_memory_buffer(void);


/*!
 * @brief           Process new sms message
 * @param[in]       buffer : Received buffer from serial port
 */
void gsm_sms_layer_process_cmd(char * buffer);

/*!
 * @brief           Change GSM state 
 * @param[in]       new_state New gsm state
 */
void gsm_change_state(gsm_state_t new_state);

/*!
 * @brief           Check gsm is sleeping or not
 * @retval          TRUE : GSM is sleeping
 *                  FALSE : GSM is not sleeping
 */
bool gsm_data_layer_is_module_sleeping(void);


/*!
 * @brief       Send data directly to serial port
 * @param[in]   raw Raw data send to serial port
 * @param[in]   len Data length
 */
void gsm_hw_uart_send_raw(uint8_t *raw, uint32_t length);


/*!
 * @brief       Get internet mode
 * @retval      Internet mode
 */
gsm_internet_mode_t *gsm_get_internet_mode(void);

/*!
 * @brief       GSM hardware uart polling
 */
void gsm_hw_layer_run(void);

/*!
 * @brief       Change GSM hardware uart polling interval is ms
 * @param[in]   Polling interval in ms
 */
void gsm_change_hw_polling_interval(uint32_t ms);

/*!
 * @brief       Set flag to prepare enter read sms state
 */
void gsm_set_flag_prepare_enter_read_sms_mode(void);


/*!
 * @brief       Enter read sms state
 */
void gsm_enter_read_sms(void);

/*!
 * @brief       Get current tick in ms
 * @retval      Current tick in ms
 */
uint32_t gsm_hardware_get_current_tick(void);

/*!
 * @brief       Update current index of gsm dma rx buffer
 * @param[in]   rx_status RX status code
 *				TRUE : data is valid, FALSE data is invalid
 */
void gsm_uart_rx_dma_update_rx_index(bool rx_status);

/*!
 * @brief		Get SIM IMEI
 * @retval		SIM IMEI
 */
char* gsm_get_sim_imei(void);

/*!
 * @brief		Get SIM CCID
 * @retval		SIM CCID pointer
 */
char *gsm_get_sim_ccid(void);

/*!
 * @brief		Get GSM IMEI
 * @retval		GSM IMEI
 */
char* gsm_get_module_imei(void);


/*!
 * @brief		Set SIM IMEI
 * @param[in]	SIM IMEI
 */
void gsm_set_sim_imei(char *imei);

/*!
 * @brief		Set GSM IMEI
 * @param[in]	GSM IMEI
 */
void gsm_set_module_imei(char *imei);

/*!
 * @brief		Set network operator
 * @param[in]	Network operator
 */
void gsm_set_network_operator(char *nw_operator);

/*!
 * @brief		Get network operator
 * @retval		Network operator
 */
char *gsm_get_network_operator(void);

/*!
 * @brief		Set GSM CSQ
 * @param[in]	CSQ GSM CSQ
 */
void gsm_set_csq(uint8_t csq);

/*!
 * @brief		Get GSM CSQ
 * @retval	 	GSM CSQ
 */
uint8_t gsm_get_csq(void);

/*!
 * @brief		Get GSM CSQ in percent
 * @retval	 	GSM CSQ in percent
 */
uint8_t gsm_get_csq_in_percent(void);

/*!
 * @brief		Wakeup gsm module
 */
void gsm_wakeup_now(void);

/*!
 * @brief		Reset gsm rx buffer
 */
void gsm_hw_layer_reset_rx_buffer(void);

/*!
 * @brief		GSM mnr task
 */
void gsm_mnr_task(void *arg);

/**
 * @brief       Get next wakeup timestamp send data to server
 * @retval      Next wakeup timestamp
 */
uint32_t gsm_data_layer_estimate_wakeup_timestamp(void);

/**
 * @brief       Check available sms need to send in buffer
 * @retval      TRUE Need to enter sms mode
 *              FALSE No message need to send
 */
bool gsm_sms_has_pending_message(void);

/**
 * @brief       Get gsm error code
 * @retval      Error code
 */
uint32_t gsm_get_error_code(void);

/**
 * \}
 */

#endif // GSM_H
