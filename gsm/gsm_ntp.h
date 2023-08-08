#ifndef GSM_NTP_H
#define GSM_NTP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t year_from_2k;
    uint8_t month;
    uint8_t month_day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} gsm_ntp_time_t;

typedef void (*gsm_ntp_callback_t)(gsm_ntp_time_t *time, bool valid);

void gsm_ntp_start(gsm_ntp_callback_t callback);

#endif
