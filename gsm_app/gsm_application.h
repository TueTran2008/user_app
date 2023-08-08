#ifndef GSM_APPLICATION_H
#define GSM_APPLICATION_H

#include <stdint.h>
#include "measure_input.h"

/**
 * @brief       Build sensor message
 * @param[in]   ptr Pointer to buffer hold data
 * @param[in]   msg Message to build
 * @retval      Message len
 */
uint16_t gsm_application_build_http_post_message(char *ptr, measure_input_peripheral_data_t *msg);

/**
 * @brief       Estimate nextime wakeup and send date to server
 * @retval      Next wakeup time in unix timestamp
 */
uint32_t gsm_application_estimate_wakeup_time(void);

/**
 * @brief       Send sms to master phone if needed
 * @retval      TRUE New sms put into queue
 *              FALSE No new sms
 */
bool gsm_application_send_sms_to_master_phone(void);

/**
 * @brief       Get next time poll broadcast command from server
 * @retval      Next timestamp, 0 on error
 */
uint32_t gsm_appication_next_time_poll_boardcast_command(void);

/**
 * @brief       Process http-get data
 * @paranm[in]  data Pointer to server data
 * @param[in]   len Data len
 * @param[in]   has_new_config Number of configuration is changed
 */
void gsm_application_on_http_get_data(uint8_t *data, uint32_t len, uint8_t *has_new_config);

#endif
