#include <stdio.h>
#include <string.h>
#include "gsm.h"
#include "app_debug.h"
#include "gsm_utilities.h"
#include "hardware.h"
#include "hardware_manager.h"
#include "gsm_http.h"
#include "server_msg.h"
// #include "app_queue.h"
#include "utilities.h"
#include "app_bkup.h"
#include "app_eeprom.h"
#include "measure_input.h"
#include "version_control.h"
#include "sys_ctx.h"
#include "ota_update.h"
#include "app_rtc.h"
#include "app_spi_flash.h"
#include "spi.h"
#include "app_flash_log.h"
#include "umm_malloc_cfg.h"
#include "usart.h"
#include "app_rtc.h"
#include "flash_if.h"
#include "gsm_ntp.h"
#include "gsm_application.h"
#include "measure_input.h"
#include "main.h"

#define UNLOCK_BAND                                                 1
#define CUSD_ENABLE                                                 0
#define MAX_TIMEOUT_TO_SLEEP_S                                      60
#define GSM_NEED_ENTER_HTTP_GET()                                   (m_enter_http_get)
#define GSM_DONT_NEED_HTTP_GET()                                    (m_enter_http_get = false)
#define GSM_NEED_ENTER_HTTP_POST()                                  (m_enter_http_post)
#define GSM_DONT_NEED_HTTP_POST()                                   (m_enter_http_post = false)
#define GSM_ENTER_HTTP_POST()                                       (m_enter_http_post = true)
#define SAVE_DATA_TO_FLASH_WHEN_POST_SUCCESS                        1
#define SAVE_DATA_TO_FLASH_WHEN_POST_FAILED                         1

#define POST_URL                                                    "%s/api/v1/%s/telemetry"
#define GET_URL                                                     "%s/api/v1/%s/attributes"
#define POLL_CONFIG_URL                                             "%s/api/v1/default_imei/attributes"

typedef union
{
    struct
    {
        uint32_t creg_err : 1;
        uint32_t sim_err : 1;
        uint32_t gsm_imei : 1;
        uint32_t reserve : 29;
    } __attribute__((packed)) name;
    uint32_t error_no;
} __attribute__((packed)) gsm_error_t;
    
extern gsm_manager_t gsm_manager;
static char m_at_cmd_buffer[128];

static const char *gsm_state_descriptor[24] = 
{
    "OK", "RESET", "SEND SMS", 
    "READ SMS", "POWER ON", "REOPEN PPP", 
    "GET BTS INFO", "GOTO SLEEP", "WAKEUP", 
    "AT", "SLEEP", "HTTP_GET", 
    "HTTP_POST", "HTTP FILE PROCESS", 
    "SNTP"
};

void gsm_at_cb_power_on_gsm(gsm_response_event_t event, void *resp_buffer);
void gsm_at_cb_read_sms(gsm_response_event_t event, void *resp_buffer);
void gsm_at_cb_send_sms(gsm_response_event_t event, void *resp_buffer);
void gsm_data_layer_switch_mode_at_cmd(gsm_response_event_t event, void *resp_buffer);
void gsm_at_cb_exit_sleep(gsm_response_event_t event, void *resp_buffer);

// NTP time query callback
static void on_ntp_complete(gsm_ntp_time_t *timestamp, bool is_error);

uint8_t convert_csq_to_percent(uint8_t csq);
uint8_t gsm_check_ready_status(void);
static void gsm_http_event_cb(gsm_http_event_t event, void *data);

static bool m_enter_http_get = false;
static bool m_enter_http_post = false;
static uint32_t m_malloc_count = 0;
static app_flash_log_data_t m_flash_rd_data;

static char *m_last_http_msg = NULL;
static app_flash_log_data_t *m_retransmission_data_in_flash;
uint32_t m_wake_time = 0;

static bool m_http_post_failed = false;
extern timestamp m_last_timedata;
bool m_poll_new_gsm_state = false;
static bool m_sntp_error = true;
static int8_t m_lut_cmd_seq = 0;
static uint32_t last_time_update_sntp = 0;
static uint32_t m_wakeup_timestamp_send_data_to_server = 0;
static uint32_t m_wakeup_time_in_minute_send_data_to_server = 0;
static uint32_t m_wakeup_time_in_day_send_data_to_server = 0;
static gsm_error_t m_gsm_error = 
{
    .error_no = 0,
};

uint32_t gsm_data_layer_estimate_wakeup_timestamp(void)
{
    return m_wakeup_timestamp_send_data_to_server;
}

static bool gsm_need_wakeup(void)
{   
    bool retval = false;    
    // Get current timestamp and internet interval
    uint32_t current_sec = app_rtc_get_counter();        
    if (app_eeprom_has_fixed_time_wakeup())
    {
        // mode fixed time
        // tim khoang thoi gian gan nhat can send data len server
        rtc_date_time_t current_time;
        uint32_t min_from_00h = 0;
        int16_t *time_setting = app_eeprom_get_fixed_time_wakeup_setting();
        app_rtc_get_time(&current_time);
        min_from_00h = current_time.hour * 60 + current_time.minute;
        int next_wakeup_time = 0;
        int min_wakeup_time = 0;
        bool new_day = false;
        bool timer_expire = false;
        
        DEBUG_INFO("Current time %02u:%02u, last send time %02u:%02u\r\n", 
                    current_time.hour, current_time.minute, 
                    m_wakeup_time_in_minute_send_data_to_server/60,
                    m_wakeup_time_in_minute_send_data_to_server%60);
        
        for (uint32_t i = 0; 
            (i < APP_EEPROM_MAX_FIXED_SEND_WEB_TIME) && retval == false; 
            i++)
        {
            int tmp = time_setting[i];
            if (time_setting[i] == APP_EEPROM_FIXED_SEND_TIME_INVALID_TIME)
            {
                // Skip invalid data
                continue;
            }
            
            if (tmp > 0)        // 14h00 send web -> 13h59 wakeup
            {
                tmp--;
            }
            
            DEBUG_VERBOSE("[%u] Setting : %02u:%02u\r\n", i, tmp/60, tmp%60);
            
            if (min_wakeup_time == 0)
            {
                min_wakeup_time = tmp;
            }
            if (next_wakeup_time == 0
                 && (tmp > m_wakeup_time_in_minute_send_data_to_server))
            {
                next_wakeup_time = tmp;
            }
            
            // Check if new day -> reset counter
            new_day = m_wakeup_time_in_day_send_data_to_server != current_time.day
                        && min_from_00h >= min_wakeup_time;
            
            timer_expire = min_from_00h >= m_wakeup_time_in_minute_send_data_to_server      // Thoi gian hien tai >= thoi gian send web lan truoc                      
                            && tmp > m_wakeup_time_in_minute_send_data_to_server
                            && min_from_00h > tmp;
            
            timer_expire |= new_day;
            
            if (timer_expire)
            {
                DEBUG_WARN("Cycle send web expire\r\n");
                next_wakeup_time = 0;
                m_wakeup_time_in_day_send_data_to_server = current_time.day;        // New day
                m_wakeup_time_in_minute_send_data_to_server = min_from_00h;
                // If module is sleep ->> wakeup immediately
                if (gsm_data_layer_is_module_sleeping())
                {
                    retval = true;
                }
            }
        }
        
        if (next_wakeup_time == 0)      // Next day
        {
            next_wakeup_time = min_wakeup_time;
        }
            
        if (!retval /*&& next_wakeup_time*/)
        {
            DEBUG_WARN("Next time wakeup %02u:%02u\r\n", 
                        next_wakeup_time/60, next_wakeup_time%60);
        }
    }
    else
    {
        // If first time reset -> estimate next time wakeup
        if (m_wakeup_timestamp_send_data_to_server == 0)
        {
            m_wakeup_timestamp_send_data_to_server = gsm_application_estimate_wakeup_time();
        }
    
        // Normal mode
        // Wakeup early, 5 secs for module power on
        if (current_sec >= m_wakeup_timestamp_send_data_to_server-5)
        {
            // If module is sleep ->> wakeup immediately
            if (gsm_data_layer_is_module_sleeping())
            {
                retval = true;
            }
        }
    }
    
    return retval;
}

void gsm_wakeup_now(void)
{
    m_wakeup_timestamp_send_data_to_server = 1;
    m_wakeup_time_in_day_send_data_to_server = 0;
    m_wakeup_time_in_minute_send_data_to_server = 0;
    
    gsm_change_state(GSM_STATE_WAKEUP);
    
    // Reset http error code
    m_http_post_failed = false;
}

void gsm_hardware_power_on_complete(void)
{
    gsm_change_state(GSM_STATE_POWER_ON);
}


static char *get_server_url(void)
{
    char *url = (char *)app_eeprom_read_config_data()->http_server_address[APP_EEPROM_MAIN_SERVER_ADDR_INDEX];
    // If main server addr is not valid =>> switch to new server addr
    // TODO check new server
    // But device has limited memory size
    if (strlen(url) < 6)    // 6 = len http://
    {
        url = (char *)app_eeprom_read_config_data()->http_server_address[APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX];
    }
    return url;
}

volatile uint32_t m_delay_wait_for_measurement_again_s = 0;
void gsm_manager_tick(void)
{
    sys_ctx_t *ctx = sys_ctx();
    // app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();

    /* GSM state machine */
    switch (gsm_manager.state)
    {
    case GSM_STATE_POWER_ON:
        if (gsm_manager.step == 0)
        {
            m_http_post_failed = false;
            gsm_data_layer_initialize();
            gsm_manager.step = 1;
            m_lut_cmd_seq = -1;
            gsm_at_cb_power_on_gsm(GSM_EVENT_CONTINUES, NULL);
        }
        break;

    case GSM_STATE_SNTP_UPDATE:
    {
        if (gsm_manager.step == 0)
        {
            gsm_manager.step = 1;
            uint32_t current_counter = app_rtc_get_counter();       
            uint32_t diff_hour = sys_get_ms() - last_time_update_sntp;   
            diff_hour /= 3600000;
          
            /* 
                Only synchronize time when
                1/ The first time reset
                2/ Time duration is over
                3/ System time is invalid
            */
            if (last_time_update_sntp == 0
                || (diff_hour >= app_eeprom_read_config_data()->sync_clock_in_hour)
                || current_counter < (uint32_t)1666147125)      // 1666147125 = 2022/10/19
            {
                gsm_ntp_start(on_ntp_complete);
            }
            else
            {
                DEBUG_INFO("Next time update sntp is %u hour\r\n", 
                            app_eeprom_read_config_data()->sync_clock_in_hour - diff_hour);
                gsm_change_state(GSM_STATE_OK);
            }
        }

    }
        break;
    
    case GSM_STATE_OK:
    {
        bool need_to_send_sms = gsm_application_send_sms_to_master_phone();
        if (app_eeprom_read_config_data()->io_enable.name.esim_active == 0)
        {
            app_eeprom_read_config_data()->io_enable.name.esim_active = 1;      // ko can commit vi lan dau tien ket noi sv -> new config ->auto save
        }
		
        gsm_need_wakeup();
        
        // Check if we need to send sms message
        if (need_to_send_sms
            && gsm_sms_has_pending_message())
        {
            gsm_change_state(GSM_STATE_SEND_SMS);
            return;
        }
        
        if (gsm_manager.state == GSM_STATE_OK) // gsm state maybe changed in gsm_sms_has_pending_message task
        {
            bool ready_to_sleep = true;
            bool enter_post = false;

            // If sensor data avaible =>> Enter http post
            uint32_t pending_msq = measure_input_sensor_data_available();
            if ((pending_msq || m_retransmission_data_in_flash) 
                && !ctx->status.enter_ota_update 
                && m_http_post_failed == false)
            {
                if (pending_msq)
                {   
                    DEBUG_INFO("Sensor queue available %u\r\n", pending_msq);
                }
                
                if (m_retransmission_data_in_flash)
                {
                    DEBUG_INFO("Retransmission data not empty\r\n", pending_msq);
                }
                enter_post = true;
            }
            
            // If no post data is active -> Scan flash to find data
            if (enter_post == false 
                && m_http_post_failed == false 
                && m_retransmission_data_in_flash == NULL)
            {
                // Wakeup flash if needed
                if (!ctx->peripheral_running.name.flash_running)
                {
                    //            DEBUG_VERBOSE("Wakup flash\r\n");
                    spi_init();
                    app_spi_flash_wakeup((app_flash_drv_t*)sys_flash());
                    ctx->peripheral_running.name.flash_running = 1;
                }
                
                LL_IWDG_ReloadCounter(IWDG);
                uint32_t do_scan = 100;
                app_flash_drv_t *flash_drv = (app_flash_drv_t*)sys_flash();
                while (do_scan)
                {
                    bool flash_has_data = app_flash_memory_log_data_is_available(flash_drv);
     
                    if (flash_has_data)
                    {
                        uint8_t *mem = NULL;
                        uint16_t size = 0;
                        uint32_t crc = 1234;        // dummy value
                        
                        app_flash_mem_error_t err = api_flash_log_read_current_message(flash_drv, 
                                                                                        &mem, 
                                                                                        &size);
                        if (mem && size == sizeof(app_flash_log_data_t))
                        {
                            // Calculate CRC
                            crc  = utilities_calculate_crc32(mem, 
                                                            sizeof(app_flash_log_data_t) - HARDWARE_CRC32_SIZE);
                        }
                        else
                        {
                            size = 0;
                        }
                        
                        if (err == APP_SPI_FLASH_MEM_OK
                            && size
                            && crc == ((app_flash_log_data_t*)mem)->crc)
                        {
                            if (((app_flash_log_data_t*)mem)->resend_to_server_flag 
                                    != APP_FLASH_DONT_NEED_TO_SEND_TO_SERVER_FLAG)
                            {
                                memcpy(&m_flash_rd_data, mem, sizeof(app_flash_log_data_t));
                                m_retransmission_data_in_flash = &m_flash_rd_data;
                                enter_post = true;
                                do_scan = 0;
                                DEBUG_VERBOSE("Found 1\r\n");
                            }
                            else
                            {
                                DEBUG_VERBOSE("Scan again\r\n");
                                do_scan--;
                            }
                        }
                        else if (err == APP_SPI_FLASH_MEM_ERROR_DATA_CORRUPT)
                        {
                            do_scan = 0;
                            DEBUG_ERROR("Flash data corrupt\r\n");
                            m_retransmission_data_in_flash = NULL;
                            enter_post = false;
                        }
                        else
                        {
                            do_scan = 0;
                        }
                    }
                    else
                    {
                        DEBUG_INFO("No more data in flash\r\n");
                        do_scan = 0;
                    }
                }
                
                // Shutdown flash
                if (sys_ctx()->peripheral_running.name.flash_running)
                {
                    app_spi_flash_shutdown(flash_drv);
                    spi_deinit();
                    sys_ctx()->peripheral_running.name.flash_running = 0;
                }
            }

            if (enter_post)
            {
                DEBUG_INFO("Enter http post\r\n");
                GSM_ENTER_HTTP_POST();
                ready_to_sleep = false;
                gsm_change_state(GSM_STATE_HTTP_POST);
            }
            else // Enter http get
            {
                DEBUG_VERBOSE("Queue empty\r\n");
                if (gsm_manager.state == GSM_STATE_OK)
                {
                    // If device need to get data to server, 
                    // or need to update firmware, or need to try to new server =>> Enter http get
                    if (GSM_NEED_ENTER_HTTP_GET() 
                        || ctx->status.enter_ota_update 
                        || (ctx->status.new_server && ctx->status.try_new_server))
                    {
                        ctx->status.delay_ota_update = 0;
                        gsm_change_state(GSM_STATE_HTTP_GET);
                        ready_to_sleep = false;
                        m_enter_http_get = true;
                    }
                }
            }
            
            // When device received new config from http get -> query sensor data again after XX delay second
            if (m_delay_wait_for_measurement_again_s > 0)
            {
                m_delay_wait_for_measurement_again_s--;
                if (measure_input_sensor_data_available())
                {
                    m_delay_wait_for_measurement_again_s = 0;
                }
                ready_to_sleep = false;
            }
            
            
            // No more http post/get ->> try to sleep
            if (ready_to_sleep)
            {
                uint32_t next_timestamp_poll_broadcast = gsm_appication_next_time_poll_boardcast_command();
                if (next_timestamp_poll_broadcast)
                {
                    // Estimate next time polling server config
                    ctx->status.next_time_get_data_from_server = next_timestamp_poll_broadcast;

                    ctx->status.poll_broadcast_msg_from_server = 1;

                    gsm_change_state(GSM_STATE_HTTP_GET);
                    m_enter_http_get = 1;
                    
                    DEBUG_WARN("Next time poll server config %u\r\n", 
                                next_timestamp_poll_broadcast);
                }
                else
                {
                    ctx->status.poll_broadcast_msg_from_server = 0;
                    gsm_hw_layer_reset_rx_buffer();
                    gsm_change_state(GSM_STATE_SLEEP);
                }
            }
        }
    }
    break;

    case GSM_STATE_RESET: /* Hard Reset */
        gsm_manager.gsm_ready = 0;
        gsm_hard_reset_sequence(gsm_hardware_power_on_complete);
        break;

#if GSM_READ_SMS_ENABLE
    case GSM_STATE_READ_SMS: /* Read SMS */
        if (gsm_manager.step == 0)
        {
            gsm_enter_read_sms();
        }
        break;
#endif

    case GSM_STATE_SEND_SMS: /* Send SMS */
    {
        if (!gsm_manager.gsm_ready)
            break;

        if (gsm_manager.step == 0)
        {
            gsm_manager.step = 1;
            gsm_hw_send_at_cmd("ATV1\r\n", "OK\r\n", "", 100, 1, gsm_at_cb_send_sms);
        }
    }
    break;

    case GSM_STATE_HTTP_POST:
    {
        if (GSM_NEED_ENTER_HTTP_POST())
        {
            GSM_DONT_NEED_HTTP_POST();
            static gsm_http_config_t cfg;
            char *http_server_address = get_server_url();

            snprintf(cfg.url, GSM_HTTP_MAX_URL_SIZE, POST_URL,
                     http_server_address,
                     gsm_get_module_imei());

            // sprintf(cfg.url, "%s", "https://iot.wilad.vn");
            cfg.on_event_cb = gsm_http_event_cb;
            cfg.action = GSM_HTTP_ACTION_POST;
            cfg.port = 443;     // unuse port
            gsm_http_start(&cfg);
            m_enter_http_get = true;
        }
    }
    break;

    case GSM_STATE_HTTP_GET:
    {
        // app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
        if (GSM_NEED_ENTER_HTTP_GET())
        {
            static gsm_http_config_t cfg;
            char *http_server_address = get_server_url();
            
            // Periodic update config data from server
            if (!sys_ctx()->status.enter_ota_update 
                && !ctx->status.try_new_server)
            {
                snprintf(cfg.url, GSM_HTTP_MAX_URL_SIZE,
                         GET_URL,
                         http_server_address,
                         gsm_get_module_imei());
                cfg.on_event_cb = gsm_http_event_cb;
                cfg.action = GSM_HTTP_ACTION_GET;
                cfg.big_file_for_ota = 0;
                gsm_http_start(&cfg);
                GSM_DONT_NEED_HTTP_GET();
            }
            else if (sys_ctx()->status.poll_broadcast_msg_from_server)
            {
                // Dinh ki quet lai 1 URL master 
                // URL nay dung de cau hinh tat ca cac thiet bi Logger
                snprintf(cfg.url, GSM_HTTP_MAX_URL_SIZE,
                         POLL_CONFIG_URL,
                         http_server_address);
                DEBUG_INFO("Poll default addr %s", cfg.url);
                sys_ctx()->status.poll_broadcast_msg_from_server = 0;
                cfg.on_event_cb = gsm_http_event_cb;
                cfg.action = GSM_HTTP_ACTION_GET;
                cfg.big_file_for_ota = 0;
                gsm_http_start(&cfg);
                GSM_DONT_NEED_HTTP_GET();
            }
            // If device not need to ota update =>> Enter mode get data from server
            else if (ctx->status.new_server && ctx->status.try_new_server) // Try new server address
            {
                DEBUG_INFO("Try new server\r\n");
                snprintf(cfg.url, GSM_HTTP_MAX_URL_SIZE,
                         GET_URL,
                         ctx->status.new_server,
                         gsm_get_module_imei());
                cfg.on_event_cb = gsm_http_event_cb;
                cfg.action = GSM_HTTP_ACTION_GET;
                cfg.big_file_for_ota = 0;
                gsm_http_start(&cfg);
                GSM_DONT_NEED_HTTP_GET();
            }
            // Device is in ota update =>> Build OTA url and enter http get
            else if (sys_ctx()->status.enter_ota_update)
            {
                snprintf(cfg.url, GSM_HTTP_MAX_URL_SIZE, "%s", ctx->status.ota_url);
                cfg.on_event_cb = gsm_http_event_cb;
                cfg.action = GSM_HTTP_ACTION_GET;
//                cfg.port = 443;
                cfg.big_file_for_ota = 1;
                gsm_http_start(&cfg);
                GSM_DONT_NEED_HTTP_GET();
            }
            else        // No more HTTP get -> enter gsm state ok
            {
                gsm_change_state(GSM_STATE_OK);
            }
        }
    }
    break;

    case GSM_STATE_WAKEUP: /* Exit sleep mode */
    {
        gsm_change_state(GSM_STATE_RESET);
    }
    break;

    case GSM_STATE_SLEEP: /* In sleep  mode */
    {
        // Must be called before estimate wakeup time
        if (gsm_need_wakeup())
        {
            gsm_change_state(GSM_STATE_WAKEUP);
        }
        else
        {
            gsm_hw_config_t *gsm_hardware = gsm_hardware_get_configuration();
            
            // Deinit uart
            if (gsm_hardware->uart_control)
            {
                gsm_hardware->uart_control(false);
            }
            
            // Shutdown all power port
            if (gsm_hardware->io_set)
            {
                gsm_hardware->io_set(gsm_hardware->gpio.power_en, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.power_key, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.reset_pin, 0);
            }
        }
    }
    break;

    default:
        DEBUG_ERROR("Unhandled case %u\r\n", gsm_manager.state);
        break;
    }
}


void gsm_data_layer_initialize(void)
{
    gsm_http_cleanup();
}

bool gsm_data_layer_is_module_sleeping(void)
{
    if (gsm_manager.state == GSM_STATE_SLEEP)
    {
        return 1;
    }
    return 0;
}

void gsm_change_state(gsm_state_t new_state)
{
    if (new_state >= GSM_STATE_MAX)
    {
        DEBUG_ERROR("Invalid gsm state %u\r\n", new_state);
        GSM_ASSERT(0);
        return;
    }
    DEBUG_INFO("Change GSM state to: %s\r\n", gsm_state_descriptor[new_state]);
    
    m_poll_new_gsm_state = true;
    if (new_state == GSM_STATE_OK) // Command state -> Data state trong PPP mode
    {
        gsm_manager.gsm_ready = 1;
    }
    switch ((uint8_t)new_state)
    {
        case GSM_STATE_RESET:
        {
            gsm_hard_reset_default_sequence();      // Reset power sequence
            gsm_hw_layer_reset_rx_buffer();
            gsm_hw_config_t *gsm_hardware = gsm_hardware_get_configuration();
            
            // Control power
            if (gsm_hardware->io_set)
            {
                gsm_hardware->io_set(gsm_hardware->gpio.power_en, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.power_key, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.reset_pin, 1);
            }
        }
            break;

        case GSM_STATE_POWER_ON:
            gsm_hw_layer_reset_rx_buffer();
            m_wake_time++;
            break;
        
        case GSM_STATE_SLEEP:
        {
            m_http_post_failed = false;
            sys_ctx_t *ctx = sys_ctx();
            ctx->peripheral_running.name.gsm_running = 0;
            gsm_hw_layer_reset_rx_buffer();
   
            if (!app_eeprom_has_fixed_time_wakeup())
            {
                // Estimate next time send data to server
                m_wakeup_timestamp_send_data_to_server = gsm_application_estimate_wakeup_time();
            }
        }
        break;        
        
        default:
            break;
    }
    gsm_manager.state = new_state;
    gsm_manager.step = 0;
}

#if UNLOCK_BAND
/*
 * EC200S-EU has bug with ESIM when using networking operator Vinaphone, Mobifone
 -> Must enable unlock band seq 
 */
static int8_t m_unlock_band_step = 0;
static void unlock_band_step_by_step(gsm_response_event_t event, void *resp_buffer);

gsm_at_lut_t unlock_band_lut[] = 
{
    {"AT+QCFG=\"nwscanseq\",3,1\r\n",   "OK\r\n", "", 3000, 2, unlock_band_step_by_step, NULL, NULL},
    {"AT+QCFG=\"nwscanmode\",3,1\r\n",  "OK\r\n", "", 3000, 3, unlock_band_step_by_step, NULL, NULL},
    {"AT+QCFG=\"band\",00,45\r\n",      "OK\r\n", "", 3000, 3, unlock_band_step_by_step, NULL, NULL},
};

static gsm_analyzer_err_t do_unlock_band(void *resp_buffer)
{
    m_unlock_band_step = -1;
    unlock_band_step_by_step(GSM_EVENT_CONTINUES, NULL);
    return GSM_ANALYZER_PROCESS_SUB_SEQ;
}

static void unlock_band_step_by_step(gsm_response_event_t event, void *resp_buffer)
{
    gsm_at_lut_t *lut;
    DEBUG_VERBOSE("Unlock band step %d\r\n", m_unlock_band_step);
    m_unlock_band_step++;
    
    if (m_unlock_band_step == sizeof(unlock_band_lut)/sizeof(unlock_band_lut[0]))
    {
        m_unlock_band_step = 0;
        gsm_at_cb_power_on_gsm(GSM_EVENT_CONTINUES, NULL);
        // Unlock process complete
        return;
    };
    
    lut = &unlock_band_lut[m_unlock_band_step];
    gsm_hw_send_at_cmd(lut->cmd, 
                        lut->response,
                        lut->expected_at_end,
                        lut->timeout_ms,
                        lut->retries, 
                        lut->callback);
}
#endif /* UNLOCK_BAND */

// Convert RTC time to unix timestamp
uint32_t make_counter(rtc_date_time_t *time)
{
    uint32_t counter;
    counter = rtc_struct_to_counter(time);
    counter += (946681200 + 3600);
    return counter;
}

//// Check if we need to set new RTC time
//bool need_update_time(uint32_t new_counter, uint32_t current_counter)
//{
//    // Dieu kien update tiem la 12h update 1 lan
//    DEBUG_VERBOSE("New counter %u, current counter %u\r\n", new_counter, current_counter);
//    static uint32_t m_last_time_update = 0;
//    if (m_last_time_update == 0 
//        || (current_counter - m_last_time_update >= (uint32_t)(3600 * 12)) 
//        || ((new_counter >= current_counter) && (new_counter - current_counter >= (uint32_t)60)) 
//        || ((new_counter < current_counter) && (current_counter - new_counter >= (uint32_t)60)))
//    {
//        DEBUG_VERBOSE("Update time\r\n");
//        m_last_time_update = new_counter;
//        return true;
//    }
//    return false;
//}


gsm_analyzer_err_t gsm_critical_error(gsm_response_event_t event, void *response_buffer)
{
    DEBUG_ERROR("GSM not response at cmd\r\n");
    gsm_change_state(GSM_STATE_RESET);
    m_lut_cmd_seq = 0;
    return GSM_ANALYZER_ERROR;
}

gsm_analyzer_err_t gsm_process_module_imei(void *resp_buffer)
{
    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;
    uint8_t *imei_buffer = (uint8_t *)gsm_get_module_imei();
    if (strlen((char *)imei_buffer) < 14)
    {
        gsm_utilities_get_imei(resp_buffer, (uint8_t *)imei_buffer, 16);
        DEBUG_WARN("Get GSM IMEI: %s\r\n", imei_buffer);
        imei_buffer = (uint8_t *)gsm_get_module_imei();
        if (strlen((char*)imei_buffer) < 15)
        {
            DEBUG_ERROR("IMEI's invalid!\r\n");
            m_gsm_error.name.sim_err = 1;
        }
        else
        {
            retval = GSM_ANALYZER_OK;
        }
    }
    else        // Da co imei tu trc roi, ko can parse
    {
        retval = GSM_ANALYZER_OK;
    }
    return retval;
}

gsm_analyzer_err_t gsm_process_sim_imsi(void *resp_buffer)
{
    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;
    uint8_t *imei_buffer = (uint8_t *)gsm_get_sim_imei();
    gsm_utilities_get_imei(resp_buffer, imei_buffer, 16);
    DEBUG_INFO("Get SIM IMSI: %s\r\n", gsm_get_sim_imei());
    static uint32_t retry = 0;
    if (strlen(gsm_get_sim_imei()) < 15)
    {
        DEBUG_ERROR("SIM's not inserted!\r\n");
        if (retry++ < 5)
        {
            retval = GSM_ANALYZER_QUERY_AGAIN;
        }
        else
        {
            retry = 0;
            retval = GSM_ANALYZER_ERROR;
        }
    }
    else
    {
        retry = 0;
        retval = GSM_ANALYZER_OK;
    }
    return retval;
}

gsm_analyzer_err_t gsm_process_sim_ccid(void *resp_buffer)
{
    // DEBUG_INFO("Get SIM IMEI: %s\r\n", (char *)resp_buffer);
    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;

    gsm_change_hw_polling_interval(5);

    uint8_t *ccid_buffer = (uint8_t *)gsm_get_sim_ccid();
    if (strlen((char *)ccid_buffer) < 10)
    {
        gsm_utilities_get_sim_ccid(resp_buffer, ccid_buffer, 20);
    }

    DEBUG_INFO("SIM CCID: %s\r\n", ccid_buffer);

    static uint32_t retry = 0;
    if (strlen((char *)ccid_buffer) < 10 && retry < 2)
    {
        retry++;
        retval = GSM_ANALYZER_QUERY_AGAIN;
    }
    else
    {
        retry = 0;
        retval = GSM_ANALYZER_OK;
    }
    return retval;
}

gsm_analyzer_err_t gsm_process_access_technology(void *resp_buffer)
{
    /** +CGREG: 2,1,"3279","487BD01",7 */

    DEBUG_INFO("Query network status : %s\r\n", 
                (char *)resp_buffer);
    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;
    static uint32_t retry = 0;
    // Parse network access technology
    if (!gsm_utilities_get_network_access_tech(resp_buffer, &gsm_manager.access_tech))
    {
		uint32_t max_retries = 60;
        if (app_eeprom_read_config_data()->io_enable.name.esim_active)
        {
            max_retries = 15;
        }
        if (retry++ < max_retries)
        {
            retval = GSM_ANALYZER_QUERY_AGAIN;
            gsm_change_hw_polling_interval(1000);
        }
        else
        {
            retry = 0;
            gsm_change_hw_polling_interval(5);
            m_gsm_error.name.creg_err = 1;
        }
    }
    else
    {
        retry = 0;
        retval = GSM_ANALYZER_OK;
    }
    return retval;
}

gsm_analyzer_err_t gsm_process_network_operator(void *resp_buffer)
{
    DEBUG_INFO("Query network operator: %s\r\n", 
                (char*)resp_buffer); /** +COPS: 0,0,"Viettel Viettel",7 */
    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;
    static uint32_t retry = 0;
    // Parse network operator
    gsm_utilities_get_network_operator(resp_buffer,
                                        gsm_get_network_operator(),
                                        32);
    if (strlen(gsm_get_network_operator()) < 5)
    {
        if (retry++ < 5)
        {
            retval = GSM_ANALYZER_QUERY_AGAIN;
        }
        else
        {
            retry = 0;
            retval = GSM_ANALYZER_OK;
        }
    }
    else
    {
        DEBUG_INFO("Network operator: %s\r\n", gsm_get_network_operator());
        gsm_change_hw_polling_interval(5);
        retval = GSM_ANALYZER_OK;
    }

    return retval;
}

gsm_analyzer_err_t gsm_process_cclk(void *resp_buffer)
{
    DEBUG_WARN("Query CCLK: %s\r\n",
                (char *)resp_buffer);

    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;
    rtc_date_time_t time;
    bool time_is_valid;
    static uint32_t retry = 0;
    
    memset(&time, 0, sizeof(rtc_date_time_t));
    
    // Prase timestamp buffer
    time_is_valid = gsm_utilities_parse_timestamp_buffer((char *)resp_buffer, &time)
                    && (time.year > 21)
                    && (time.year < 40) 
                    && (time.hour < 24);
    
    time.hour += TIMEZONE;

    DEBUG_VERBOSE("Network time is %s, NTP time is %s\r\n",
            time_is_valid ? "valid" : "invalid",
            m_sntp_error ? "valid" : "invalid");
    
    if (time_is_valid)
    {
        retry = 0;
        retval = GSM_ANALYZER_OK;
    }
    
    if (time_is_valid
        && m_sntp_error) // if 23h40 =>> time.hour += 7 = 30h =>> invalid
                        // Lazy solution : do not update time from 17h
    {        
        // Chi update thoi gian khi thiet bi khong dc cai thoi gian thu cong o web
        if (m_last_timedata.time.year == 0
            && m_last_timedata.time.month == 0
            && m_last_timedata.time.day == 0
            && m_last_timedata.time.hour == 0
            && m_last_timedata.time.minute == 0
            && m_last_timedata.time.second == 0
            && /*need_update_time(new_counter, current_counter)*/1)
        {
            app_rtc_set_counter(&time);
        }
    }
    else if (time_is_valid == false)
    {
        if (retry == 0 || retry > 3)
        {
            gsm_change_hw_polling_interval(5);
            retval = GSM_ANALYZER_OK;
        }
        else
        {
            DEBUG_WARN("Re-sync clock\r\n");
            retval = GSM_ANALYZER_QUERY_AGAIN;
            gsm_change_hw_polling_interval(1000);
        }
    }


    return retval;
}

gsm_analyzer_err_t gsm_process_csq(void *resp_buffer)
{
    uint8_t csq = GSM_CSQ_INVALID;
    gsm_analyzer_err_t retval = GSM_ANALYZER_ERROR;
    gsm_set_csq(0);
    
    // Parse CSQ
    gsm_utilities_get_signal_strength_from_buffer(resp_buffer, &csq);
    DEBUG_INFO("CSQ: %d\r\n", csq);

    if (csq == GSM_CSQ_INVALID)
    {
        retval = GSM_ANALYZER_QUERY_AGAIN;
        gsm_change_hw_polling_interval(500);
    }
    else
    {
        gsm_set_csq(csq);
        gsm_change_hw_polling_interval(5);
        retval = GSM_ANALYZER_OK;
    }
    return retval;
}

// GSM register network sequence
static gsm_at_lut_t gsm_power_on_lut[] = 
{
    {"AT\r\n",                  "OK\r\n",       "",             500,   15,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     NULL                            },
    {"ATE0\r\n",                "OK\r\n",       "",             1000,   5,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     NULL                            },
    {"AT+CMEE=2\r\n",           "OK\r\n",       "",             1000,   10,     gsm_at_cb_power_on_gsm,     gsm_critical_error,     NULL                            },

    {"ATI\r\n",                 "OK\r\n",       "",             1000,   10,     gsm_at_cb_power_on_gsm,     NULL,                   NULL                            },
    {"AT+QCFG="
    "\"urc/ri/smsincoming\""
    ",\"pulse\",2000\r\n",      "OK\r\n",       "",             1000,   10,     gsm_at_cb_power_on_gsm,     gsm_critical_error,     NULL                            },
    {"AT+CNMI=2,1,0,0,0\r\n",   "OK\r\n",       "",             1000,   10,     gsm_at_cb_power_on_gsm,     NULL,                   NULL                            },
    {"AT+CMGF=1\r\n",           "OK\r\n",       "",             1000,   3,      gsm_at_cb_power_on_gsm,     NULL,                   NULL                            },
    {"AT+CGSN\r\n",             "\r\nOK\r\n",   "",             1000,   5,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     gsm_process_module_imei         },
    {"AT+CIMI\r\n",             "OK\r\n",       "",             1000,   5,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     gsm_process_sim_imsi            },
    {"AT+QCCID\r\n",            "QCCID",        "OK\r\n",       1000,   3,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     gsm_process_sim_ccid            },
    {"AT+CPIN?\r\n",            "READY",        "",             1000,   3,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     NULL                            },
    {"AT+QIDEACT=1\r\n",        "OK\r\n",       "",             3000,   1,      gsm_at_cb_power_on_gsm,     NULL,                   NULL                            },
    {"AT\r\n",                  "OK\r\n",       "",             1000,   3,      gsm_at_cb_power_on_gsm,     NULL,                   do_unlock_band                  },
    {"AT+CGDCONT=1,\"IP\","
    "\"v-internet\"\r\n",       "OK\r\n",       "",             3000,   2,      gsm_at_cb_power_on_gsm,     NULL,                   NULL                            }, /** <cid> = 1-24 */
    {"AT+CGREG=2\r\n",          "OK\r\n",       "",             2000,   3,      gsm_at_cb_power_on_gsm,     NULL,                   NULL                            },
    {"AT+CGREG?\r\n",           "OK\r\n",       "",             3000,   5,      gsm_at_cb_power_on_gsm,     NULL,                   gsm_process_access_technology},
    {"AT+COPS?\r\n",            "OK\r\n",       "",             2000,   5,      gsm_at_cb_power_on_gsm,     NULL,                   gsm_process_network_operator    },
    {"AT+CCLK?\r\n",            "+CCLK:",       "OK\r\n",       1000,   5,      gsm_at_cb_power_on_gsm,     NULL,                   gsm_process_cclk                },
    {"AT+CSQ\r\n",              "OK\r\n",       "",             1000,   5,      gsm_at_cb_power_on_gsm,     gsm_critical_error,     gsm_process_csq                 },
    // Final : change to state sntp
};


void gsm_at_cb_power_on_gsm(gsm_response_event_t event, void *resp_buffer)
{
    gsm_at_lut_t *lut;
#if 0
    char *ptr = "null";
    if (resp_buffer)
    {
        ptr = resp_buffer;
    }
    DEBUG_INFO("Power on LUT result %u, response %s\r\n", event, ptr);
#endif
    if (event == GSM_EVENT_OK || event == GSM_EVENT_CONTINUES)
    {
        gsm_analyzer_err_t analyzer_error = GSM_ANALYZER_OK;
        if (event == GSM_EVENT_CONTINUES)
        {
            DEBUG_VERBOSE("Continues next lut\r\n");
            goto next_lut_cmd;
        }
        if (gsm_power_on_lut[m_lut_cmd_seq].analyzer_response)
        {
            analyzer_error = gsm_power_on_lut[m_lut_cmd_seq].analyzer_response(resp_buffer);
        }
        
        if (analyzer_error == GSM_ANALYZER_QUERY_AGAIN)
        {
            DEBUG_INFO("Query again\r\n");
            goto send_cmd;
        }
        
        if (analyzer_error == GSM_ANALYZER_PROCESS_SUB_SEQ)
        {
            return;
        }

        
        if (analyzer_error == GSM_ANALYZER_ERROR)
        {
            if (gsm_power_on_lut[m_lut_cmd_seq].error_cb)
            {
                gsm_power_on_lut[m_lut_cmd_seq].error_cb(event, resp_buffer);
            }
            else
            {
                gsm_change_state(GSM_STATE_SLEEP);
            }
            return;
        }
next_lut_cmd:
        // Success command
        m_lut_cmd_seq++;
        goto send_cmd;
    }
    else        // An error occurred
    {
        if (gsm_power_on_lut[m_lut_cmd_seq].error_cb)
        {
            gsm_power_on_lut[m_lut_cmd_seq].error_cb(event, resp_buffer);
        }
        return;
    }

send_cmd:
    // If we run to the last index of table -> quit
    if (m_lut_cmd_seq == sizeof(gsm_power_on_lut)/sizeof(gsm_power_on_lut[0]))
    {
        m_lut_cmd_seq = 0;
        gsm_change_state(GSM_STATE_SNTP_UPDATE);
        return;
    };
    // Else process command in LUT
    lut = &gsm_power_on_lut[m_lut_cmd_seq];
    gsm_hw_send_at_cmd(lut->cmd, 
                            lut->response,
                            lut->expected_at_end,
                            lut->timeout_ms,
                            lut->retries, 
                            lut->callback);
}


void gsm_at_cb_exit_sleep(gsm_response_event_t event, void *resp_buffer)
{
    switch (gsm_manager.step)
    {
    case 1:
        gsm_hw_send_at_cmd("ATV1\r\n", "OK\r\n", "", 1000, 5, gsm_at_cb_exit_sleep);
        break;
    case 2:
        gsm_hw_send_at_cmd("AT+QSCLK=1\r\n", "OK\r\n", "", 1000, 10, gsm_at_cb_exit_sleep);
        break;
    case 3:
        if (event == GSM_EVENT_OK)
        {
            //            DEBUG_INFO("Exit sleep!");
            gsm_change_state(GSM_STATE_OK);
        }
        else
        {
            //            DEBUG_INFO("Khong phan hoi lenh, reset module..");
            gsm_change_state(GSM_STATE_RESET);
        }
        break;

    default:
        break;
    }
    gsm_manager.step++;
}


void gsm_at_cb_send_sms(gsm_response_event_t event, void *resp_buffer)
{
    uint8_t count;
    static uint8_t retry_count = 0;
    gsm_sms_msg_t *sms = gsm_get_sms_memory_buffer();
    uint32_t max_sms = gsm_get_max_sms_memory_buffer();

    //    DEBUG_INFO("Debug SEND SMS : %u %u,%s\r\n", gsm_manager.step, event, resp_buffer);

    switch (gsm_manager.step)
    {
    case 1:
        for (count = 0; count < max_sms; count++)
        {
            if (sms[count].need_to_send == 2)
            {
                DEBUG_INFO("Sms to %s. Content : %s\r\n",
                           sms[count].phone_number, sms[count].message);

                sprintf(m_at_cmd_buffer, "AT+CMGS=\"%s\"\r\n", sms[count].phone_number);

                gsm_hw_send_at_cmd(m_at_cmd_buffer,
                                   ">", NULL,
                                   15000,
                                   1,
                                   gsm_at_cb_send_sms);
                break;
            }
        }
        break;

    case 2:
        if (event == GSM_EVENT_OK)
        {
            for (count = 0; count < max_sms; count++)
            {
                if (sms[count].need_to_send == 2)
                {
                    sms[count].message[strlen(sms[count].message)] = 26; // 26 = ctrl Z
                    gsm_hw_send_at_cmd(sms[count].message, "+CMGS",
                                       NULL,
                                       30000,
                                       1,
                                       gsm_at_cb_send_sms);
                    //                    DEBUG_INFO("Sending sms in buffer %u\r\n", count);
                    break;
                }
            }
        }
        else
        {
            retry_count++;
            if (retry_count < 3)
            {
                gsm_change_state(GSM_STATE_SEND_SMS);
                return;
            }
            else
            {
                goto SEND_SMS_FAIL;
            }
        }
        break;

    case 3:
        if (event == GSM_EVENT_OK)
        {
            DEBUG_INFO("SMS : Send sms success\r\n");

            for (count = 0; count < max_sms; count++)
            {
                if (sms[count].need_to_send == 2)
                {
                    sms[count].need_to_send = 0;
                }
            }
            retry_count = 0;
            gsm_change_state(GSM_STATE_OK);
        }
        else
        {
            DEBUG_ERROR("SMS : Send sms failed\r\n");
            retry_count++;
            if (retry_count < 3)
            {
                gsm_change_state(GSM_STATE_SEND_SMS);
                return;
            }
            else
            {
                DEBUG_ERROR("Send sms failed many times, cancle\r\n");
                goto SEND_SMS_FAIL;
            }
        }
        return;

    default:
        DEBUG_INFO("Unknown outgoing sms step %d\r\n", gsm_manager.step);
        gsm_change_state(GSM_STATE_OK);
        break;
    }

    gsm_manager.step++;

    return;

SEND_SMS_FAIL:
    for (count = 0; count < gsm_get_max_sms_memory_buffer(); count++)
    {
        if (sms[count].need_to_send == 2)
        {
            sms[count].need_to_send = 1;
            gsm_change_state(GSM_STATE_OK);
        }
    }

    retry_count = 0;
}

static measure_input_peripheral_data_t *m_sensor_msq;

static void gsm_http_event_cb(gsm_http_event_t event, void *data)
{
    sys_ctx_t *ctx = sys_ctx();
    sys_keep_led_on(2);
    
    switch (event)
    {
    case GSM_HTTP_EVENT_START:
        break;

    case GSM_HTTP_EVENT_CONNTECTED:
        HARDWARE_LED1_CTRL(1);
        DEBUG_INFO("HTTP connected, data size %u\r\n", *((uint32_t *)data));
        if (ctx->status.enter_ota_update)
        {
#if OTA_VERSION == 0
            if (!ota_update_start(*((uint32_t *)data)))
            {
                DEBUG_WARN("OTA update failed\r\n");
                sys_delay_ms(10);
                NVIC_SystemReset();
            }
#endif
        }
        break;

    case GSM_HTTP_GET_EVENT_DATA:
    {
        gsm_http_data_t *get_data = (gsm_http_data_t *)data;
        uint8_t new_config;
        DEBUG_RAW("%s\r\n", get_data->data);
        gsm_application_on_http_get_data(get_data->data, get_data->data_length, &new_config);
        
        // Do lai cam bien 1 lan nua roi gui len server
        // Boi vi co the server ra lenh thay doi tham so, thay doi server
        // Phai gui len gia tri do moi ngay lap tuc
        // Delay 10s de thoi gian bat/tat cam bien
        if (new_config)
        {  
            // Save data if we has new config
            measure_input_measure_wakeup_to_get_data();
            app_eeprom_save_config();
            m_delay_wait_for_measurement_again_s = 10;
        }
    }
    break;

    case GSM_HTTP_POST_EVENT_DATA:
    {
        bool build_msg = false;

        // If we have data need to send to server from ext flash
        // Else we scan measure input message queeu
        if (m_retransmission_data_in_flash)
        {
            DEBUG_INFO("Found retransmission data\r\n");
            // Copy old pulse counter from spi flash to temp variable
            static measure_input_peripheral_data_t tmp;
            memset(&tmp, 0, sizeof(measure_input_peripheral_data_t));
            for (uint32_t i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
            {
                memcpy(&tmp.counter[i],
                       &m_retransmission_data_in_flash->counter[i],
                       sizeof(measure_input_cyber_counter_t));
            }

            // Input 4-20mA
            for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_INPUT_4_20MA; i++)
            {
                tmp.input_4_20mA[i] = m_retransmission_data_in_flash->input_4_20mA[i];
            }
            
            // Input anlog pressure
            for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; i++)
            {
                tmp.input_pressure_analog[i] = m_retransmission_data_in_flash->input_pressure_analog[i];
            }
            

            // Output 4-20mA
            for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_OUTPUT_4_20MA; i++)
            {
                tmp.output_4_20mA[i] = m_retransmission_data_in_flash->output_4_20mA[i];
            }

            tmp.csq_percent = m_retransmission_data_in_flash->csq_percent;
            tmp.measure_timestamp = m_retransmission_data_in_flash->timestamp;
            tmp.temperature = m_retransmission_data_in_flash->temp;
            tmp.internal_battery_voltage = m_retransmission_data_in_flash->internal_battery_voltage;
            tmp.vin_in_percent = m_retransmission_data_in_flash->vin_in_percent;

            // Analog input
            tmp.analog_input[0] = m_retransmission_data_in_flash->analog_input[0];
            tmp.analog_input[1] = m_retransmission_data_in_flash->analog_input[1];
            // DEBUG_INFO("A0 %umv\r\n", m_retransmission_data_in_flash->analog_input[0]);
            // DEBUG_INFO("A1 %umv\r\n", m_retransmission_data_in_flash->analog_input[1]);

            // on/off
            tmp.input_on_off[0] = m_retransmission_data_in_flash->on_off.name.input_on_off_0;
            tmp.input_on_off[1] = m_retransmission_data_in_flash->on_off.name.input_on_off_1;

            // Modbus
            for (uint32_t i = 0; i < HARDWARE_RS485_MAX_SLAVE_ON_BUS; i++)
            {
                memcpy(&tmp.rs485[i], 
                        &m_retransmission_data_in_flash->rs485[i], 
                        sizeof(measure_input_modbus_register_t));
            }

            m_sensor_msq = &tmp;
            m_retransmission_data_in_flash = NULL;
            build_msg = true;
        }
        else // Get data from measure input queue
        {
            DEBUG_VERBOSE("Get http post data from queue\r\n");
            m_sensor_msq = measure_input_get_data_in_queue();
            if (!m_sensor_msq)
            {
                DEBUG_INFO("No data in RAM queue, scan message in flash\r\n");
                gsm_change_state(GSM_STATE_OK);
            }
            else
            {
                DEBUG_VERBOSE("Found message in queue\r\n");
                build_msg = true;
            }
        }

        if (build_msg)
        {
            if (m_last_http_msg)
            {
                m_last_http_msg = NULL;
                m_malloc_count--;
            }

            static char m_http_mem[1024 + 512+128];
            m_last_http_msg = m_http_mem;
            if (m_last_http_msg)
            {
                ++m_malloc_count;

                // Build sensor message
                m_sensor_msq->state = MEASUREMENT_QUEUE_STATE_PROCESSING;
                gsm_application_build_http_post_message(m_last_http_msg, m_sensor_msq);

                ((gsm_http_data_t *)data)->data = (uint8_t *)m_last_http_msg;
                ((gsm_http_data_t *)data)->data_length = strlen(m_last_http_msg);
                ((gsm_http_data_t *)data)->header = (uint8_t *)"";
                DEBUG_INFO("Data size %d\r\n", ((gsm_http_data_t *)data)->data_length);
            }
            else
            {
                DEBUG_ERROR("Malloc error\r\n");
                NVIC_SystemReset();
            }
        }
    }
    break;

    case GSM_HTTP_GET_EVENT_FINISH_SUCCESS:
    {
        ctx->status.disconnected_counter = 0;
        DEBUG_INFO("HTTP get : event success\r\n");
        app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
        ctx->status.disconnected_counter = 0;
        if (ctx->status.enter_ota_update // If ota update finish and all file downloaded =>> turn off gsm
            && ctx->status.delay_ota_update == 0)
        {
            // turn off module
            gsm_hw_config_t *gsm_hardware = gsm_hardware_get_configuration();
            if (gsm_hardware->io_set)
            {
                gsm_hardware->io_set(gsm_hardware->gpio.power_en, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.power_key, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.reset_pin, 0);
            }
#if OTA_VERSION == 0
            measure_input_save_all_data_to_flash();
            ota_update_finish(true);
#endif
        }

        // If device in mode test connection with new server =>> Store new server to flash
        // Step 1 : device trying to connect with new server, step jump from 2 to 1
        // Step 2 : step == 1 =>> try success
        if (ctx->status.try_new_server == 1 && ctx->status.new_server)
        {
            // Delete old server and copy new server addr
            memset(eeprom_cfg->http_server_address[APP_EEPROM_MAIN_SERVER_ADDR_INDEX], 
                    0, 
                    APP_EEPROM_MAX_SERVER_ADDR_LENGTH);
            sprintf((char *)eeprom_cfg->http_server_address[APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX], 
                    "%s", 
                    ctx->status.new_server);
            
            ctx->status.try_new_server = 0;

            //			umm_free(ctx->status.new_server);
            ctx->status.new_server = NULL;

            app_eeprom_save_config(); // Store current config into eeprom
            DEBUG_INFO("Set new server addr success\r\n");
        }
        if (ctx->status.try_new_server)
        {
            ctx->status.try_new_server--;
        }

        if (ctx->status.last_state_is_disconnect)
        {
            ctx->status.last_state_is_disconnect = 0;
            // Only send sms when device has valid phone && alarm enable && message was sent < max message in 1 day
            if (strlen((char *)eeprom_cfg->phone) > 9 
                && eeprom_cfg->io_enable.name.warning 
                && (ctx->status.total_sms_in_24_hour < eeprom_cfg->max_sms_1_day))
            {
                char msg[128];
                char *p = msg;
                rtc_date_time_t time;
                app_rtc_get_time(&time);

                p += sprintf(p, "%04u/%02u/%02u %02u:%02u: ",
                             time.year + 2000,
                             time.month,
                             time.day,
                             time.hour,
                             time.minute);
                p += sprintf(p, "Thiet bi %s, %s da ket noi tro lai", 
                            VERSION_CONTROL_DEVICE, 
                            gsm_get_module_imei());
                ctx->status.total_sms_in_24_hour++;
                gsm_send_sms((char *)eeprom_cfg->phone, msg);
            }
        }

        gsm_change_state(GSM_STATE_OK);
        HARDWARE_LED1_CTRL(0);
    }
    break;

    case GSM_HTTP_POST_EVENT_FINISH_SUCCESS:
    {
        DEBUG_INFO("HTTP post : event success\r\n");
        HARDWARE_LED1_CTRL(0);
        m_http_post_failed = false;
        ctx->status.disconnected_counter = 0;

        // Release old memory from send buffer
        if (m_last_http_msg)
        {
            m_malloc_count--;
            //            umm_free(m_last_http_msg);
            m_last_http_msg = NULL;
        }

#if SAVE_DATA_TO_FLASH_WHEN_POST_SUCCESS
        // Read data from ext flash
        app_flash_log_data_t wr_data;

        // Mark flag we dont need to send data to server
        wr_data.resend_to_server_flag = APP_FLASH_DONT_NEED_TO_SEND_TO_SERVER_FLAG;

        // Copy 4-20mA input
        for (uint32_t i = 0; i < APP_FLASH_NB_OFF_4_20MA_INPUT; i++)
        {
            wr_data.input_4_20mA[i] = m_sensor_msq->input_4_20mA[i];
        }
        
        // Copy pulse counter
        for (uint32_t i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
        {
            memcpy(&wr_data.counter[i], 
                    &m_sensor_msq->counter[i], 
                    sizeof(measure_input_cyber_counter_t));
        }
        
        // Input pressure
        for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; i++)
        {
            wr_data.input_pressure_analog[i] = m_sensor_msq->input_pressure_analog[i];
        }
        
        // on/off
        wr_data.on_off.name.input_on_off_0 = m_sensor_msq->input_on_off[0];
        wr_data.on_off.name.input_on_off_1 = m_sensor_msq->input_on_off[1];
        
        // 4-20mA output
        for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_OUTPUT_4_20MA; i++)
        {
            wr_data.output_4_20mA[i] = m_sensor_msq->output_4_20mA[i];
        }

        wr_data.csq_percent = m_sensor_msq->csq_percent;
        wr_data.timestamp = m_sensor_msq->measure_timestamp;
        wr_data.valid_flag = APP_FLASH_VALID_DATA_KEY;
        wr_data.internal_battery_voltage = m_sensor_msq->internal_battery_voltage;
        wr_data.vin_in_percent = m_sensor_msq->vin_in_percent;
        wr_data.temp = m_sensor_msq->temperature;

        // Copy 485 data
        for (uint32_t i = 0; i < HARDWARE_RS485_MAX_SLAVE_ON_BUS; i++)
        {
            memcpy(&wr_data.rs485[i], 
                    &m_sensor_msq->rs485[i], 
                    sizeof(measure_input_modbus_register_t));
        }

        // Wakeup flash if needed
        if (!ctx->peripheral_running.name.flash_running)
        {
//            DEBUG_VERBOSE("Wakup flash\r\n");
            spi_init();
            app_spi_flash_wakeup((app_flash_drv_t*)sys_flash());
            ctx->peripheral_running.name.flash_running = 1;
        }

        // Commit data and release memory
        app_flash_log_measurement_data((app_flash_drv_t*)sys_flash(), &wr_data);
        
        // Shutdown flash
        if (ctx->peripheral_running.name.flash_running)
        {
            app_spi_flash_shutdown(sys_flash());
            spi_deinit();
            ctx->peripheral_running.name.flash_running = 0;
        }
#endif /* SAVE_DATA_TO_FLASH_WHEN_POST_SUCCESS */
        
        m_sensor_msq->state = MEASUREMENT_QUEUE_STATE_IDLE;
        m_sensor_msq = NULL;

        // Scan for retransmission data
        bool re_send = false;
        
        // Wakeup flash
        if (!ctx->peripheral_running.name.flash_running)
        {
            spi_init();
            app_spi_flash_wakeup((app_flash_drv_t*)sys_flash());
            ctx->peripheral_running.name.flash_running = 1;
        }
        uint32_t cont_scan = 100;
        uint32_t log_size = sizeof(app_flash_log_data_t);
        while (cont_scan)
        {
            // Check ringbuffer has data
            bool flash_has_data = app_flash_memory_log_data_is_available((app_flash_drv_t*)sys_flash());
            if (flash_has_data)
            {
                if (cont_scan)
                {
                    cont_scan--;
                }
                uint8_t *mem = NULL;
                uint16_t size = 0;
                uint32_t crc = 1234;        // dummy value
                
                // Read current message in ringbuffer
                app_flash_mem_error_t err;
                err = api_flash_log_read_current_message((app_flash_drv_t*)sys_flash(), &mem, &size);
                if (mem && size >= log_size)
                {
                    // Calculate CRC
                    crc = utilities_calculate_crc32(mem, log_size - HARDWARE_CRC32_SIZE);
                }
                else
                {
                    size = 0;
                }
                        
                // If no error & valid size
                if (err == APP_SPI_FLASH_MEM_OK
                    && size)
                {
                    // Compare CRC
                    if (crc != ((app_flash_log_data_t*)mem)->crc)
                    {
                        DEBUG_ERROR("Invalid CRC\r\n");
                    }
                    // Check if flag == dont need to send ->> Skip this message and search memory again
                    else if (((app_flash_log_data_t*)mem)->resend_to_server_flag 
                                != APP_FLASH_DONT_NEED_TO_SEND_TO_SERVER_FLAG)
                    {
                        memcpy(&m_flash_rd_data, mem, log_size);
                        m_retransmission_data_in_flash = &m_flash_rd_data;
                        GSM_ENTER_HTTP_POST();
                        re_send = true;
                        cont_scan = 0;
                    }
                    else
                    {
                        DEBUG_VERBOSE("Dont need send to server\r\n");
                    }
                }
                else if (err == APP_SPI_FLASH_MEM_ERROR_DATA_CORRUPT)
                {
                    DEBUG_ERROR("Flash data corrupt\r\n");
                    m_retransmission_data_in_flash = NULL;
                    m_enter_http_post = false;
                }
                else
                {
                    DEBUG_ERROR("Flash error code %u\r\n", err);
                    cont_scan = 0;
                }
            }
            else
            {
                DEBUG_INFO("Flash queue empty\r\n");
                cont_scan = 0;
            }
        }
        uint32_t pending_msq = measure_input_sensor_data_available();
        
        if (re_send)
        {
            DEBUG_INFO("Need re-transission data\r\n");
        } // no need retransmisson
        else if (pending_msq == 0)
        {
            // Ready to enter http get
            if (sys_ctx()->status.poll_broadcast_msg_from_server)
            {
                sys_ctx()->status.poll_broadcast_msg_from_server = 0;
            }
            else
            {

            }
            DEBUG_INFO("No more data need to re-send to server\r\n");
            m_retransmission_data_in_flash = NULL;
        }
        else
        {
            DEBUG_INFO("Pending msq %u\r\n", pending_msq);
        }
        
        // Shutdown flash
        if (sys_ctx()->peripheral_running.name.flash_running)
        {
            app_spi_flash_shutdown(sys_flash());
            spi_deinit();
            sys_ctx()->peripheral_running.name.flash_running = 0;
        }
                
        gsm_change_state(GSM_STATE_OK);
    }
    break;

    case GSM_HTTP_POST_EVENT_FINISH_FAILED:
    {
        DEBUG_WARN("Http post event failed\r\n");
        m_http_post_failed = true;
        if (m_last_http_msg)
        {
            m_malloc_count--;
            //            umm_free(m_last_http_msg);
            m_last_http_msg = NULL;
        }
        
#if SAVE_DATA_TO_FLASH_WHEN_POST_FAILED
        static app_flash_log_data_t wr_data;;

        wr_data.resend_to_server_flag = 0;

        // Input 4-20mA
        for (uint32_t i = 0; i < APP_FLASH_NB_OFF_4_20MA_INPUT; i++)
        {
            wr_data.input_4_20mA[i] = m_sensor_msq->input_4_20mA[i];
        }
        for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_ANALOG_INPUT_PRESSURE; i++)
        {
            wr_data.input_pressure_analog[i] = m_sensor_msq->input_pressure_analog[i];
        }
        
        for (uint32_t i = 0; i < MEASURE_NUMBER_OF_WATER_METER_INPUT; i++)
        {
            memcpy(&wr_data.counter[i], 
                    &m_sensor_msq->counter[i], 
                    sizeof(measure_input_cyber_counter_t));
        }

        wr_data.timestamp = m_sensor_msq->measure_timestamp;
        wr_data.valid_flag = APP_FLASH_VALID_DATA_KEY;
        wr_data.internal_battery_voltage = m_sensor_msq->internal_battery_voltage;
        wr_data.vin_in_percent = m_sensor_msq->vin_in_percent;
        wr_data.temp = m_sensor_msq->temperature;
        wr_data.csq_percent = m_sensor_msq->csq_percent;

        // on/off
        wr_data.on_off.name.input_on_off_0 = m_sensor_msq->input_on_off[0];
        wr_data.on_off.name.input_on_off_1 = m_sensor_msq->input_on_off[1];

        // Output 4-20mA
        for (uint32_t i = 0; i < HARDWARE_NUMBER_OF_OUTPUT_4_20MA; i++)
        {
            wr_data.output_4_20mA[i] = m_sensor_msq->output_4_20mA[i];
        }

        // 485
        for (uint32_t i = 0; i < HARDWARE_RS485_MAX_SLAVE_ON_BUS; i++)
        {
            memcpy(&wr_data.rs485[i], 
                    &m_sensor_msq->rs485[i], 
                    sizeof(measure_input_modbus_register_t));
        }
        
        // Wakeup flash
        if (!ctx->peripheral_running.name.flash_running)
        {
            spi_init();
            app_spi_flash_wakeup((app_flash_drv_t*)sys_flash());
            ctx->peripheral_running.name.flash_running = 1;
        }
        if (m_sensor_msq->state != MEASUREMENT_QUEUE_STATE_IDLE)
        {
            app_flash_log_measurement_data((app_flash_drv_t*)sys_flash(), &wr_data);
            m_sensor_msq->state = MEASUREMENT_QUEUE_STATE_IDLE;
        }
        
        // Shutdown flash
        if (ctx->peripheral_running.name.flash_running)
        {
            app_spi_flash_shutdown(sys_flash());
            spi_deinit();
            ctx->peripheral_running.name.flash_running = 0;
        }
#endif
        m_sensor_msq->state = MEASUREMENT_QUEUE_STATE_IDLE;
        if (m_retransmission_data_in_flash)
        {
            m_retransmission_data_in_flash = NULL;
        }

        m_sensor_msq = NULL;
    }
        // break; jump to case failed
    case GSM_HTTP_GET_EVENT_FINISH_FAILED:
    {
        DEBUG_WARN("HTTP event failed\r\n");

        // If in in ota update mode, save all data and perform reset
        if (ctx->status.enter_ota_update && ctx->status.delay_ota_update == 0)
        {
            // Save all data
            measure_input_save_all_data_to_flash();
            
            // Turn off module
            gsm_hw_config_t *gsm_hardware = gsm_hardware_get_configuration();
            
            if (gsm_hardware->io_set)
            {
                gsm_hardware->io_set(gsm_hardware->gpio.power_en, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.power_key, 0);
                gsm_hardware->io_set(gsm_hardware->gpio.reset_pin, 0);
            }
            ota_update_finish(false);
        }
        // If device in mode test new server =>> Store new server to flash
        if (ctx->status.try_new_server && ctx->status.new_server)
        {
            ctx->status.try_new_server = 0;
            //			umm_free(ctx->status.new_server);
            ctx->status.new_server = NULL;
            DEBUG_ERROR("Try new server failed\r\n");
        }

        if (m_last_http_msg)
        {
            m_malloc_count--;
//            umm_free(m_last_http_msg);
//            DEBUG_ERROR("Free um memory, malloc count[%u]\r\n", m_malloc_count);
            m_last_http_msg = NULL;
        }
        gsm_change_state(GSM_STATE_OK);
    }
    break;

    default:
        DEBUG_WARN("Unknown http event %d\r\n", (int)event);
        gsm_change_state(GSM_STATE_OK);
        break;
    }
}

static void on_ntp_complete(gsm_ntp_time_t *timestamp, bool valid)
{
    // If no error && valid timestamp
    if (valid && timestamp != NULL)
    {
        last_time_update_sntp = sys_get_ms();
        m_sntp_error = false;
        
        rtc_date_time_t settime;
        settime.hour = timestamp->hour;
        settime.minute = timestamp->min;
        settime.second = timestamp->sec;
        settime.day = timestamp->month_day;
        settime.month = timestamp->month;
        settime.year = timestamp->year_from_2k;
       
//        uint32_t new_counter = make_counter(&settime);
//        uint32_t current_counter = app_rtc_get_counter();
        if (/*need_update_time(new_counter, current_counter)*/1)
        {
            app_rtc_set_counter(&settime);
        }
    }
    else
    {
        DEBUG_INFO("STNP error\r\n");
        m_sntp_error = true;
    }
    gsm_change_state(GSM_STATE_OK);
}

uint32_t gsm_hardware_get_current_tick(void)
{
    gsm_hw_config_t *gsm_hardware = gsm_hardware_get_configuration();
    return gsm_hardware->sys_now_ms();
}

static uint32_t m_reset_sms_message_counter = 0;
void gsm_mnr_task(void *arg)
{
    static uint32_t last_poll;
    uint32_t current_tick = gsm_hardware_get_current_tick();
    if (current_tick - last_poll >= (uint32_t)1000)
    {
        sys_ctx_t *ctx = sys_ctx();
        if (current_tick - m_reset_sms_message_counter >= (uint32_t)86400)     // 1 day is over
        {
            m_reset_sms_message_counter = current_tick;
            ctx->status.total_sms_in_24_hour = 0;
        }
        
        m_poll_new_gsm_state = true;
        last_poll = current_tick;
        if (gsm_data_layer_is_module_sleeping())
        {
//        ctx->status.sleep_time_s++;
            ctx->status.disconnected_counter = 0;
        }
        else
        {
      uint32_t max_time_wakeup = GSM_MAX_DISCONNECT_COUNTER_SECOND;
            if (!app_eeprom_read_config_data()->io_enable.name.esim_active)          // neu chua active esim thi can nhieu thoi gian hon
            {
                max_time_wakeup = 240;
            }
            if (ctx->status.disconnected_counter++ > max_time_wakeup)
            {
                DEBUG_ERROR("GSM disconnected for a long time %u\r\n", 
                            ctx->status.disconnected_counter);
                app_eeprom_config_data_t *eeprom_cfg = app_eeprom_read_config_data();
                measure_input_save_all_data_to_flash();
                gsm_http_cleanup();
                gsm_hw_layer_reset_rx_buffer();

                if (m_last_http_msg)
                {
                    m_malloc_count--;
                    //                umm_free(m_last_http_msg);
                    m_last_http_msg = NULL;
                }

                ctx->status.disconnected_counter = 0;
                if (ctx->status.disconnected_count++ > 23)
                {
                    ctx->status.disconnected_count = 0;
                    
                    // If valid phone number && state switch from connected to disconnected -> send sms
                    // Only accept max 24 max_sms_1_day
                    if (strlen((char *)eeprom_cfg->phone) > 9 
                        && eeprom_cfg->io_enable.name.warning 
                        && (ctx->status.total_sms_in_24_hour < eeprom_cfg->max_sms_1_day)
                        && ctx->status.last_state_is_disconnect == 0)
                    {
                        char msg[156];
                        char *p = msg;
                        rtc_date_time_t time;
                        app_rtc_get_time(&time);

                        p += sprintf(p, "%04u/%02u/%02u %02u:%02u: ",
                                     time.year + 2000,
                                     time.month,
                                     time.day,
                                     time.hour,
                                     time.minute);
                        char *server = (char *)eeprom_cfg->http_server_address[APP_EEPROM_MAIN_SERVER_ADDR_INDEX];
                        
                        // If valid server
                        if (strlen((char *)eeprom_cfg->http_server_address[APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX]) 
                                > 8)
                        {
                            server = (char *)eeprom_cfg->http_server_address[APP_EEPROM_ALTERNATIVE_SERVER_ADDR_INDEX];
                        }
                        p += snprintf(p, 155, "TB %s %s, mat ket noi %s", 
                                    VERSION_CONTROL_DEVICE, 
                                    gsm_get_module_imei(), 
                                    server);
                        ctx->status.total_sms_in_24_hour++;
                        gsm_send_sms((char *)eeprom_cfg->phone, msg);
                    }
                    else
                    {
                        gsm_change_state(GSM_STATE_SLEEP);
                    }
                    ctx->status.last_state_is_disconnect = 1;
                }
                else
                {
                    gsm_change_state(GSM_STATE_SLEEP);
                }
            }
        }   
    }
    
    if (m_poll_new_gsm_state)
    {
        m_poll_new_gsm_state = false;
        gsm_manager_tick();
    }
}

uint32_t gsm_get_error_code(void)
{
    return m_gsm_error.error_no;
}

