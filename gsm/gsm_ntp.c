#include "gsm_ntp.h"
#include "gsm.h"
#include "app_debug.h"
#include "gsm_utilities.h"

static gsm_ntp_callback_t m_ntp_callback;

static gsm_analyzer_err_t parse_final_sntp(void *response_buffer)
{
    DEBUG_INFO("%s\r\n", (char*)response_buffer);
    gsm_ntp_time_t ntp_time;
    // Parse response timestamp buffer
    bool retval = gsm_utilities_parse_sntp_timestamp_buffer(response_buffer, 
                                                            &ntp_time.year_from_2k, &ntp_time.month, &ntp_time.month_day,
                                                            &ntp_time.hour, &ntp_time.min, &ntp_time.sec);
    
    // If valid time
    if (retval
        && ntp_time.year_from_2k > 21  // 2021
        && ntp_time.year_from_2k < 50)	// 2050
    {
        retval = true;
    }
    else
    {
        DEBUG_ERROR("Invalid ntp time\r\n");
        retval = false;
    }
    
    if (m_ntp_callback)
    {
        m_ntp_callback(&ntp_time, retval);
    }
    
    return retval ? GSM_ANALYZER_OK : GSM_ANALYZER_ERROR;
}


static int8_t m_sntp_step = 0;
static void sntp_step_by_step(gsm_response_event_t event, void *resp_buffer);

static gsm_analyzer_err_t stnp_error(gsm_response_event_t event, void *response_buffer)
{
    DEBUG_INFO("GSM NTP not response cmd\r\n");
    if (m_ntp_callback)
    {
        m_ntp_callback(NULL, false);
    }
    return GSM_ANALYZER_ERROR;
}

static gsm_analyzer_err_t gsm_parse_qiact(void *resp_buffer)
{
    DEBUG_INFO("QIACT %s\r\n", (char*)resp_buffer);
    gsm_utilites_qiact_state_t qiact_state;
    gsm_analyzer_err_t retval = GSM_ANALYZER_OK;
    
    if (gsm_utilities_parse_qiact_query_response((char*)resp_buffer, &qiact_state))
    {
        DEBUG_INFO("SNTP =>> PDP already activated\r\n");
        m_sntp_step++;      // skip AT+QIACT=1
    }
    
    return retval;
}

static gsm_at_lut_t m_sntp_lut[] = 
{
    {"AT+QIACT?\r\n",           "OK\r\n",   "",   500,        5,          sntp_step_by_step,    stnp_error,   gsm_parse_qiact     },
    {"AT+QIACT=1\r\n",          "OK\r\n",   "",   5000,       3,          sntp_step_by_step,    stnp_error,   NULL                },
    {"AT+QNTP=1,"
    "\"pool.ntp.org\"\r\n",     "+QNTP",    "",   10000,      2,          sntp_step_by_step,    stnp_error,   parse_final_sntp    },
};


static void sntp_step_by_step(gsm_response_event_t event, void *resp_buffer)
{
    gsm_at_lut_t *lut;
    char *ptr = (char*)resp_buffer;
    if (!ptr)
    {
        ptr = "null";
    }
    DEBUG_VERBOSE("SNTP step %d, res %s\r\n", m_sntp_step, ptr);
    
    if (event == GSM_EVENT_CONTINUES)
    {
        goto send_cmd;
    }
    
    if (event == GSM_EVENT_OK)
    {
        gsm_analyzer_err_t analyzer_error = GSM_ANALYZER_OK;
        if (m_sntp_lut[m_sntp_step].analyzer_response)
        {
            analyzer_error = m_sntp_lut[m_sntp_step].analyzer_response(resp_buffer);
        }
        if (analyzer_error == GSM_ANALYZER_ERROR)
        {
            return;
        }
    }
    
    if (event != GSM_EVENT_OK)
    {
        if (m_sntp_lut[m_sntp_step].error_cb)
        {
            m_sntp_lut[m_sntp_step].error_cb(event, resp_buffer);
        }
        return;
    }
    
send_cmd:    
    m_sntp_step++;
    // Close seq on finish
    if (m_sntp_step == sizeof(m_sntp_lut)/sizeof(m_sntp_lut[0]))
    {
        m_sntp_step = 0;
        return;
    };
    
    lut = &m_sntp_lut[m_sntp_step];
    gsm_hw_send_at_cmd(lut->cmd, 
                        lut->response,
                        lut->expected_at_end,
                        lut->timeout_ms,
                        lut->retries, 
                        lut->callback);
}

void gsm_ntp_start(gsm_ntp_callback_t callback)
{
    m_ntp_callback = callback;
    m_sntp_step = -1;
    sntp_step_by_step(GSM_EVENT_CONTINUES, NULL);
}
