#include <stdbool.h>
#include <stdint.h>

#include <iec_std_lib.h>

#include <plc_abi.h>
#include <plc_dbg.h>

#include <plc_hw.h>
#include <plc_clock.h>
#include <plc_backup.h>
#include <plc_rtc.h>
#include <plc_tick.h>
#include <plc_app_default.h>

#include <plc_config.h>

#ifndef PLC_BLINK
#   define PLC_BLINK() do{}while(0)
#endif

unsigned char plc_state = PLC_STATE_STOPED;
plc_app_abi_t * plc_app = (plc_app_abi_t *)&plc_app_default;

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

int main(void)
{
    PLC_DISABLE_INTERRUPTS();

    plc_clock_setup();
    plc_hw_init();

    if( plc_check_hw() )
    {
        // H/W check failed!!!
        plc_tick_setup( 0, 1000000ull );
        PLC_ENABLE_INTERRUPTS();

        while(1)
        {
            plc_panic_hw();
        }
    }
    else
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
            //App code is OK, start the app...
            plc_app = (plc_app_abi_t *)PLC_APP;
            plc_app_cstratup();
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
        plc_app->start(0,0);
    }

    //plc_start_delay();

    while(1)
    {
        //Hadnle debug connection
        dbg_handler();
        //Heart bit
        PLC_BLINK();
        //App run
        if( plc_tick_flag )
        {
            plc_tick_flag = false;
            if( PLC_STATE_STARTED == plc_state )
            {
                plc_app->run();
            }
            // Check clock system status
            if( plc_hw_status & PLC_HW_ERR_HSE )
            {
                plc_error_hse();
            }
        }
    }
}