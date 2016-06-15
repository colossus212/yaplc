#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rtc.h>

#include <plc_rtc.h>
#include <plc_hw.h>
#include <plc_config.h>

#define PLC_RTC_DIV_VAL 0x7FFF

uint32_t plc_rtc_is_ok(void)
{
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_BKP);

    return rcc_rtc_clock_enabled_flag();
}

/*
 *  Julian Day Number computation
 *  See http://en.wikipedia.org/wiki/Julian_day
 */
static uint32_t jdn( tm *date_time )
{
    uint8_t a,m;
    uint16_t y;
    uint32_t ret;

    /* Compute helper factors */
    a = (14 - (uint8_t)date_time->tm_mon)/12;
    m = (uint8_t)date_time->tm_mon + 12*a - 3;
    y = (uint16_t)date_time->tm_year + 4800 - a;

    /* Julian day number computation */
    ret = (uint32_t)date_time->tm_day;
    ret += ( 153ul * (uint32_t)m + 2ul )/ 5ul;
    ret += 365ul * (uint32_t)y;
    ret += (uint32_t)( y/4   );
    ret -= (uint32_t)( y/100 );
    ret += (uint32_t)( y/400 );
    ret -= 32045ul;

    return ret;
}

/*
 * Compute unix-time seconds
 */
static uint32_t dt_to_sec( tm *date_time )
{
    uint32_t ret;

    /*Count days from 01.01.1970*/
    ret = jdn(date_time);
    ret -= 2440588ul;

    /*Convert day number to seconds*/
    ret *= 24ul;
    ret += date_time->tm_hour;
    ret *= 60ul;
    ret += date_time->tm_min;
    ret *= 60ul;
    ret += date_time->tm_sec;

    return ret;
}

static void  jdn_to_dt ( uint32_t jdn, tm *date_time)
{
    uint32_t a,b,c,d,e,m;

    a = jdn + 32044;
    b = (4 * a + 3) /146097;
    c = a - (146097 * b) /4;
    d = (4 * c + 3) /1461;
    e = c - (1461 * d) /4;
    m = (5 * e + 2) /153;

    date_time->tm_year = 100*b + d - 4800 + (m/10);
    date_time->tm_mon = m + 3 - 12 * (m /10);
    date_time->tm_day = e - (153 * m + 2) /5 + 1;
}

void  sec_to_dt ( uint32_t utime, tm *date_time)
{
    date_time->tm_sec = utime%60;
    utime /= 60;
    date_time->tm_min = utime%60;
    utime /= 60;
    date_time->tm_hour = utime%24;
    utime /= 24;       //Unix day number
    utime += 2440588ul; //JDN

    jdn_to_dt(utime, date_time);
}


void plc_rtc_init( tm* time )
{
    uint32_t i;

    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_BKP);

    pwr_disable_backup_domain_write_protect();

    /* LSE oscillator clock used as the RTC clock */

    if (!rcc_rtc_clock_enabled_flag())
    {
        RCC_BDCR |= RCC_BDCR_LSEON;

        i = 0;
        while (0 == (RCC_BDCR & RCC_BDCR_LSERDY))
        {
            if (i++ > 1000000)
            {
                goto lse_error;
            }
        }
        //LSE is OK, may now set the clock
        RCC_BDCR &= ~((1 << 8) | (1 << 9));
        RCC_BDCR |= (1 << 8);
    }

    rtc_enter_config_mode();

    /* Reset values */
    RTC_CRH = 0;
    RTC_CRL = 0x20;
    /* We have 32,768KHz clock, set 0x7fff*/
    RTC_PRLL = PLC_RTC_DIV_VAL     & 0xffff;;
    RTC_PRLH = PLC_RTC_DIV_VAL>>16 & 0x000f;;
    //Get date and time, set counters.
    i = dt_to_sec( time );
    RTC_CNTH = (i >> 16) & 0xffff;
    RTC_CNTL =         i & 0xffff;

    RTC_ALRH = 0xFFFF;
    RTC_ALRL = 0xFFFF;

    rtc_exit_config_mode();

    rcc_enable_rtc_clock();

    /* Wait for the RSF bit in RTC_CRL to be set by hardware. */
    RTC_CRL &= ~RTC_CRL_RSF;
    i = 0;
    while (0 == (RTC_CRL & RTC_CRL_RSF))
    {
        if (i++ > 1000000)
        {
            goto lse_error;
        }
    }
    /* Wait for the last write operation to finish. */
    /* TODO: Necessary? */
    i = 0;
    while (0 == (RTC_CRL & RTC_CRL_RSF))
    {
        if (i++ > 1000000)
        {
            goto lse_error;
        }
    }
    /* Normal termination */
    pwr_enable_backup_domain_write_protect();
    return;
    /* LSE error occured */
lse_error:
    plc_hw_status |= PLC_HW_ERR_LSE;
    pwr_enable_backup_domain_write_protect();
}

void plc_rtc_time_get( IEC_TIME *current_time )
{
    uint32_t sub_sec;

    current_time->tv_sec = rtc_get_counter_val();
    /* Add subseconds! */
    sub_sec = rtc_get_prescale_div_val();
    /* Check for time shift... */
    if (sub_sec > PLC_RTC_DIV_VAL)
    {
        /* This is NOT normal RTC operation!!! */
        current_time->tv_nsec = 0;
        return;
    }
    else
    {
        current_time->tv_nsec = (((PLC_RTC_DIV_VAL - sub_sec)*10000ul)/(PLC_RTC_DIV_VAL + 1))*100000ul;
    }
}



