#include <stdbool.h>
#include <stdint.h>

#include <iec_std_lib.h>

#include <plc_config.h>

#include <plc_abi.h>
#include <plc_dbg.h>

#include <plc_iom.h>
#include <plc_hw.h>
#include <plc_clock.h>
#include <plc_wait_tmr.h>
#include <plc_backup.h>
#include <plc_rtc.h>
#include <plc_tick.h>
#include <plc_app_default.h>

uint32_t plc_hw_status = 0;

unsigned char plc_state = PLC_STATE_STOPED;
plc_app_abi_t * plc_curr_app = (plc_app_abi_t *)&plc_app_default;

extern bool plc_app_is_valid(void);
extern void plc_app_cstratup(void);

const tm default_date =
{
    .tm_sec  = 0,
    .tm_min  = 0,
    .tm_hour = 0,
    .tm_day  = 1,
    .tm_mon  = 3,
    .tm_year = 2016
};

const char plc_start_msg[] = "This is YAPLC!";

int main(void)
{
    PLC_DISABLE_INTERRUPTS();

    plc_app_default_init();
    plc_clock_setup();
    plc_wait_tmr_init();
    plc_jmpr_init();
    plc_boot_init();
    plc_heart_beat_init();
    plc_iom_init();

    if( plc_iom_test_hw() )
    {
        // H/W is OK, continue init...
        plc_backup_init();
        if( plc_rst_jmpr_get() )
        {
            //do reset
            plc_rtc_init( (tm *)&default_date );
            plc_backup_invalidate();
        }

        if(0 == plc_rtc_is_ok())
        {
            //rtc is not OK, must reset it!
            plc_rtc_init( (tm *)&default_date );
        }

        if( plc_app_is_valid() )
        {
            //App code is OK, do cstartup
            plc_app_cstratup();
            //Check plc io locations
            if(plc_iom_check_and_sort())
            {
                //Everything is OK may use app code
                plc_curr_app = (plc_app_abi_t *)PLC_APP;
                plc_curr_app->log_msg_post(LOG_DEBUG, (char *)plc_start_msg, sizeof(plc_start_msg));
            }
        }
    }
    dbg_init();
    PLC_ENABLE_INTERRUPTS();

    if( plc_dbg_jmpr_get() )
    {
        //Wait for debug connection, app won't be started!
        plc_state = PLC_STATE_STOPED;
    }
    else
    {
        //May start the app immediately
        plc_state = PLC_STATE_STARTED;
        plc_curr_app->start(0,0);
    }

    while(1)
    {
        //Hadnle debug connection
        dbg_handler();
        //Heart bit
        plc_heart_beat_poll();
        plc_iom_poll();
        //App run
        if( plc_tick_flag )
        {
            plc_tick_flag = false;
            if( PLC_STATE_STARTED == plc_state )
            {
                plc_iom_get();
                plc_curr_app->run();
                plc_iom_set();
            }
        }
    }
}
