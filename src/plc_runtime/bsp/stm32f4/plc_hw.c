#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>

#include <plc_abi.h>

#include <plc_wait_tmr.h>
#include <plc_hw.h>
#include <plc_config.h>

uint32_t plc_hw_status = 0;

bool  plc_check_hw(void)
{
    ///Write your code here!!!
    return false;  ///Everuthing is OK by default
}
bool plc_dbg_jmpr_get(void)
{
    ///Write your code here!!!
    return false;
}

volatile bool reset_jmp = false;

bool plc_rst_jmpr_get(void)
{
    ///Write your code here!!!
    return plc_get_din(8);
}

void plc_error_hse(void)
{
    ///Write your code here!!!
    return;  ///Must return!!!
}

//Led blink timer
static uint32_t blink_tmr;

void plc_hw_init(void)
{
    //Boot pin config
    rcc_periph_clock_enable( PLC_BOOT_PERIPH );
    gpio_mode_setup(PLC_BOOT_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_BOOT_PIN);
    gpio_set_output_options(PLC_BOOT_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_BOOT_PIN);
    gpio_set( PLC_BOOT_PORT, PLC_BOOT_PIN ); //Exit boot mode

    ///LEDs
    PLC_CLEAR_TIMER( blink_tmr );
    //LED1
    rcc_periph_clock_enable( PLC_LED1_PERIPH );
    gpio_mode_setup(PLC_LED1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_LED1_PIN);
    gpio_set_output_options(PLC_LED1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_LED1_PIN);
    gpio_clear( PLC_LED1_PORT, PLC_LED1_PIN );
    //LED2
    rcc_periph_clock_enable( PLC_LED2_PERIPH );
    gpio_mode_setup(PLC_LED2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_LED2_PIN);
    gpio_set_output_options(PLC_LED2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_LED2_PIN);
    gpio_clear( PLC_LED2_PORT, PLC_LED2_PIN );
    //LED3
    rcc_periph_clock_enable( PLC_LED3_PERIPH );
    gpio_mode_setup(PLC_LED3_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_LED3_PIN);
    gpio_set_output_options(PLC_LED3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_LED3_PIN);
    gpio_clear( PLC_LED3_PORT, PLC_LED3_PIN );
    ///Outputs
    //DO1
    rcc_periph_clock_enable( PLC_O1_PERIPH );
    gpio_mode_setup(PLC_O1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_O1_PIN);
    gpio_set_output_options(PLC_O1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_O1_PIN);
    gpio_clear( PLC_O1_PORT, PLC_O1_PIN );
    //DO2
    rcc_periph_clock_enable( PLC_O2_PERIPH );
    gpio_mode_setup(PLC_O2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_O2_PIN);
    gpio_set_output_options(PLC_O2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_O2_PIN);
    gpio_clear( PLC_O2_PORT, PLC_O2_PIN );
    //DO3
    rcc_periph_clock_enable( PLC_O3_PERIPH );
    gpio_mode_setup(PLC_O3_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_O3_PIN);
    gpio_set_output_options(PLC_O3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_O3_PIN);
    gpio_clear( PLC_O3_PORT, PLC_O3_PIN );
    //DO4
    rcc_periph_clock_enable( PLC_O4_PERIPH );
    gpio_mode_setup(PLC_O4_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_O4_PIN);
    gpio_set_output_options(PLC_O4_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_O4_PIN);
    gpio_clear( PLC_O4_PORT, PLC_O4_PIN );
    ///Inputs
    //DI1
    rcc_periph_clock_enable(PLC_I1_PERIPH);
    gpio_mode_setup(PLC_I1_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I1_PIN);
    //DI2
    rcc_periph_clock_enable(PLC_I2_PERIPH);
    gpio_mode_setup(PLC_I2_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I2_PIN);
    //DI3
    rcc_periph_clock_enable(PLC_I3_PERIPH);
    gpio_mode_setup(PLC_I3_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I3_PIN);
    //DI4
    rcc_periph_clock_enable(PLC_I4_PERIPH);
    gpio_mode_setup(PLC_I4_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I4_PIN);
    //DI5
    rcc_periph_clock_enable(PLC_I5_PERIPH);
    gpio_mode_setup(PLC_I5_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I5_PIN);
    //DI6
    rcc_periph_clock_enable(PLC_I6_PERIPH);
    gpio_mode_setup(PLC_I6_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I6_PIN);
    //DI7
    rcc_periph_clock_enable(PLC_I7_PERIPH);
    gpio_mode_setup(PLC_I7_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I7_PIN);
    //DI8
    rcc_periph_clock_enable(PLC_I8_PERIPH);
    gpio_mode_setup(PLC_I8_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I8_PIN);

	//Enable power interface
	rcc_periph_clock_enable( RCC_PWR );
}

void enter_boot_mode(void)
{
    uint32_t delay;
    gpio_clear( PLC_BOOT_PORT, PLC_BOOT_PIN );

    PLC_CLEAR_TIMER( delay );
    while( PLC_TIMER(delay) < 2000 );

    scb_reset_system();
}

void plc_start_delay(void)
{
    uint32_t delay;
    PLC_CLEAR_TIMER( delay );
    while( PLC_TIMER(delay) < 100 );
}

extern uint32_t plc_backup_satus;

extern plc_app_abi_t * plc_app;

static bool hse_post_flag = true;
const char plc_hse_err_msg[] = "HSE oscilator failed!";
static bool lse_post_flag = true;
const char plc_lse_err_msg[] = "LSE oscilator failed!";

void plc_heart_beat(void)
{
    uint32_t blink_thr;
    if( plc_hw_status > 0 )
    {
        blink_thr = 500;
    }
    else
    {
        blink_thr = 1000;
    }

    if( PLC_TIMER(blink_tmr) > (blink_thr>>1) )
    {
        gpio_set( PLC_LED1_PORT, PLC_LED1_PIN );
        //if(  *(uint8_t *)BKPSRAM_BASE == 1 )
        if(plc_hw_status  & PLC_HW_ERR_HSE)
        {
            if( hse_post_flag )
            {
                hse_post_flag = false;
                plc_app->log_msg_post(LOG_CRITICAL, (char *)plc_hse_err_msg, sizeof(plc_hse_err_msg));
            }
            gpio_set( PLC_LED2_PORT, PLC_LED2_PIN );
        }
        //if( *( (uint8_t *)BKPSRAM_BASE + 1) == 1 )
        if(plc_hw_status  & PLC_HW_ERR_LSE)
        {
            if( lse_post_flag )
            {
                lse_post_flag = false;
                plc_app->log_msg_post(LOG_CRITICAL, (char *)plc_lse_err_msg, sizeof(plc_lse_err_msg));
            }
            gpio_set( PLC_LED3_PORT, PLC_LED3_PIN );
        }
    }

    if( PLC_TIMER(blink_tmr) > blink_thr )
    {
        PLC_CLEAR_TIMER(blink_tmr);
        gpio_clear( PLC_LED1_PORT, PLC_LED1_PIN );
        gpio_clear( PLC_LED2_PORT, PLC_LED2_PIN );
        gpio_clear( PLC_LED3_PORT, PLC_LED3_PIN );
    }
}

bool plc_get_din(uint32_t i)
{
    switch( i )
    {
    case 1:
    {
        return !gpio_get( PLC_I1_PORT, PLC_I1_PIN );
    }
    case 2:
    {
        return !gpio_get( PLC_I2_PORT, PLC_I2_PIN );
    }
    case 3:
    {
        return !gpio_get( PLC_I3_PORT, PLC_I3_PIN );
    }
    case 4:
    {
        return !gpio_get( PLC_I4_PORT, PLC_I4_PIN );
    }
    case 5:
    {
        return !gpio_get( PLC_I5_PORT, PLC_I5_PIN );
    }
    case 6:
    {
        return !gpio_get( PLC_I6_PORT, PLC_I6_PIN );
    }
    case 7:
    {
        return !gpio_get( PLC_I7_PORT, PLC_I7_PIN );
    }
    case 8:
    {
        return !gpio_get( PLC_I8_PORT, PLC_I8_PIN );
    }
    default:
    {
        return false;
    }
    }
}

void plc_set_dout( uint32_t i, bool val )
{
    void (*do_set)(uint32_t, uint16_t);

    do_set = (val)?gpio_set:gpio_clear;

    switch(i)
    {
    case 1:
    {
        do_set( PLC_O1_PORT, PLC_O1_PIN );
        break;
    }
    case 2:
    {
        do_set( PLC_O2_PORT, PLC_O2_PIN );
        break;
    }
    case 3:
    {
        do_set( PLC_O3_PORT, PLC_O3_PIN );
        break;
    }
    case 4:
    {
        do_set( PLC_O4_PORT, PLC_O4_PIN );
        break;
    }
    default:
    {
        break;
    }
    }
}

uint32_t plc_get_ain(uint32_t i)
{
    (void)i;
    return 0;
}

void plc_set_aout( uint32_t i, uint32_t val )
{
    (void)i;
    (void)val;
}
