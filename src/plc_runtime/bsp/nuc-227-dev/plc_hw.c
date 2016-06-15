#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>

#include <plc_config.h>
#include <plc_abi.h>
#include <plc_wait_tmr.h>
#include <plc_hw.h>
#include <plc_iom.h>

void plc_jmpr_init(void)
{
    ///Write your code here!!!
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

void plc_boot_init(void)
{
    //Boot pin config
    rcc_periph_clock_enable( PLC_BOOT_PERIPH );
    gpio_mode_setup(PLC_BOOT_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PLC_BOOT_PIN);
    gpio_set_output_options(PLC_BOOT_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, PLC_BOOT_PIN);
    gpio_set( PLC_BOOT_PORT, PLC_BOOT_PIN ); //Exit boot mode
}

void plc_boot_mode_enter(void)
{
    uint32_t delay;
    gpio_clear( PLC_BOOT_PORT, PLC_BOOT_PIN );

    PLC_CLEAR_TIMER( delay );
    while( PLC_TIMER(delay) < 2000 );

    scb_reset_system();
}


//Led blink timer
static uint32_t blink_tmr;
void plc_heart_beat_init(void)
{
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
}

extern uint32_t plc_backup_satus;

static bool hse_post_flag = true;
const char plc_hse_err_msg[] = "HSE oscilator failed!";
static bool lse_post_flag = true;
const char plc_lse_err_msg[] = "LSE oscilator failed!";

void plc_heart_beat_poll(void)
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
                plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_hse_err_msg, sizeof(plc_hse_err_msg));
            }
            gpio_set( PLC_LED2_PORT, PLC_LED2_PIN );
        }
        //if( *( (uint8_t *)BKPSRAM_BASE + 1) == 1 )
        if(plc_hw_status  & PLC_HW_ERR_LSE)
        {
            if( lse_post_flag )
            {
                lse_post_flag = false;
                plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_lse_err_msg, sizeof(plc_lse_err_msg));
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

#include "dbnc_flt.h"
dbnc_flt_t in_flt[8];

//Digital io
#define LOCAL_PROTO dio
void PLC_IOM_LOCAL_INIT(void)
{
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
    dbnc_flt_init(in_flt + 0);
    //DI2
    rcc_periph_clock_enable(PLC_I2_PERIPH);
    gpio_mode_setup(PLC_I2_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I2_PIN);
    dbnc_flt_init(in_flt + 1);
    //DI3
    rcc_periph_clock_enable(PLC_I3_PERIPH);
    gpio_mode_setup(PLC_I3_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I3_PIN);
    dbnc_flt_init(in_flt + 2);
    //DI4
    rcc_periph_clock_enable(PLC_I4_PERIPH);
    gpio_mode_setup(PLC_I4_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I4_PIN);
    dbnc_flt_init(in_flt + 3);
    //DI5
    rcc_periph_clock_enable(PLC_I5_PERIPH);
    gpio_mode_setup(PLC_I5_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I5_PIN);
    dbnc_flt_init(in_flt + 4);
    //DI6
    rcc_periph_clock_enable(PLC_I6_PERIPH);
    gpio_mode_setup(PLC_I6_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I6_PIN);
    dbnc_flt_init(in_flt + 5);
    //DI7
    rcc_periph_clock_enable(PLC_I7_PERIPH);
    gpio_mode_setup(PLC_I7_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I7_PIN);
    dbnc_flt_init(in_flt + 6);
    //DI8
    rcc_periph_clock_enable(PLC_I8_PERIPH);
    gpio_mode_setup(PLC_I8_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PLC_I8_PIN);
    dbnc_flt_init(in_flt + 7);
}

bool PLC_IOM_LOCAL_TEST_HW(void)
{
    return true;
}

static char print_buf[128];
static const char print_lt[] = {'I','M','Q'};
static const char print_sz[] = {'X','B','W','D','L'};

void plc_iom_check_print(uint16_t i)
{
    int cnt;
    cnt = sprintf(
              print_buf,
              "Checking: %%%c%c%d",
              print_lt[PLC_APP->l_tab[i]->v_type],
              print_sz[PLC_APP->l_tab[i]->v_size],
              (int)PLC_APP->l_tab[i]->proto
          );
    if( PLC_APP->l_tab[i]->a_size )
    {
        uint16_t j;
        for (j = 0; j < PLC_APP->l_tab[i]->a_size; j++)
        {
            cnt += sprintf( print_buf + cnt, ".%d", (int)PLC_APP->l_tab[i]->a_data[j] );
            //Overflow check
            if( cnt >= 115 )
            {
                //Maximum print length is 12bytes
                print_buf[115] = 0;
                cnt = 114;
                break;
            }
        }
    }
    //Must use default app here
    plc_curr_app->log_msg_post(LOG_DEBUG, (char *)print_buf, cnt+1);
}

const char plc_iom_err_proto[]   = "IO protocol is not supported!";
const uint32_t plc_iom_err_psz   = sizeof(plc_iom_err_proto);

const char plc_dio_err_sz[]      = "Wrong variable size!";

const char plc_dio_err_asz[]     = "Digital I/O address must be one number!";
const char plc_dio_err_asz_flt[] = "Digital input filter address must consist of two numbers!";

const char plc_dio_err_tp[]      = "Digital I/O does not support memory locations!";
const char plc_dio_err_tp_flt[]  = "Digital input filter must have memory type!";

const char plc_dio_err_ilim[]    = "Digital input must have address 1...8!";
const char plc_dio_err_flt_lim[] = "Digital input must have 0 or 1 address!";
const char plc_dio_err_olim[]    = "Digital output must have address 1...4!";

bool PLC_IOM_LOCAL_CHECK(uint16_t i)
{
    uint32_t addr;
    switch( PLC_APP->l_tab[i]->v_size )
    {
    case PLC_LSZ_X:
        if (1 != PLC_APP->l_tab[i]->a_size){
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_asz, sizeof(plc_dio_err_asz));
            return false;
        }

        addr = PLC_APP->l_tab[i]->a_data[0];
        //Check type and address
        switch (PLC_APP->l_tab[i]->v_type)
        {
        case PLC_LT_I:
            if (addr < 1 || addr > 8){
                plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_ilim, sizeof(plc_dio_err_ilim));
                return false;
            }else{
                return true;
            }

        case PLC_LT_M:
        default:
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_sz, sizeof(plc_dio_err_sz));
            return false;

        case PLC_LT_Q:
            if (addr < 1 || addr > 4){
                plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_olim, sizeof(plc_dio_err_olim));
                return false;
            }else{
                return true;
            }
        }
        return true;

    case PLC_LSZ_B:
        if (2 != PLC_APP->l_tab[i]->a_size){
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_asz_flt, sizeof(plc_dio_err_asz_flt));
            return false;
        }

        if (PLC_LT_M != PLC_APP->l_tab[i]->v_type){
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_tp_flt, sizeof(plc_dio_err_tp_flt));
            return false;
        }

        addr = PLC_APP->l_tab[i]->a_data[0];

        if (addr < 1 || addr > 8){
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_ilim, sizeof(plc_dio_err_ilim));
            return false;
        }

        addr = PLC_APP->l_tab[i]->a_data[1];
        if( addr > 1 ){
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_flt_lim, sizeof(plc_dio_err_flt_lim));
            return false;
        }
        return true;

    default:
        plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_sz, sizeof(plc_dio_err_sz));
        return false;
    }
}

void PLC_IOM_LOCAL_START(uint16_t lid)
{
}
void PLC_IOM_LOCAL_END(uint16_t lid)
{
}
uint32_t PLC_IOM_LOCAL_SCHED(uint16_t lid, uint32_t tick)
{
    return 0;
}
void PLC_IOM_LOCAL_POLL(uint32_t tick)
{
    uint32_t i;
    for (i = 0; i < 8; i++){
        dbnc_flt_poll(in_flt+i, tick, plc_get_din(i+1));
    }
}
uint32_t PLC_IOM_LOCAL_WEIGTH(uint16_t lid)
{
    return PLC_APP->l_tab[lid]->a_data[0];
}
uint32_t PLC_IOM_LOCAL_GET(uint16_t i)
{
    switch( plc_curr_app->l_tab[i]->v_type )
    {
    case PLC_LT_I:
        *(bool *)(plc_curr_app->l_tab[i]->v_buf) = dbnc_flt_get( in_flt + plc_curr_app->l_tab[i]->a_data[0] - 1 );
        break;

    case PLC_LT_M:
        if (plc_curr_app->l_tab[i]->a_data[1]){
            *(uint8_t *)(plc_curr_app->l_tab[i]->v_buf) = (uint8_t)in_flt[plc_curr_app->l_tab[i]->a_data[0] - 1].thr_on;
        }else{
            *(uint8_t *)(plc_curr_app->l_tab[i]->v_buf) = (uint8_t)in_flt[plc_curr_app->l_tab[i]->a_data[0] - 1].thr_off;
        }
        break;

    default:
        break;
    }
    return 0;
}
uint32_t PLC_IOM_LOCAL_SET(uint16_t i)
{
    switch( plc_curr_app->l_tab[i]->v_type )
    {
    case PLC_LT_Q:
        plc_set_dout( plc_curr_app->l_tab[i]->a_data[0], *(bool *)(plc_curr_app->l_tab[i]->v_buf) );
        break;

    case PLC_LT_M:
        if (plc_curr_app->l_tab[i]->a_data[1]){
            in_flt[plc_curr_app->l_tab[i]->a_data[0] - 1].thr_on = (uint32_t)*(uint8_t *)(plc_curr_app->l_tab[i]->v_buf);
        }else{
            in_flt[plc_curr_app->l_tab[i]->a_data[0] - 1].thr_off = (uint32_t)*(uint8_t *)(plc_curr_app->l_tab[i]->v_buf);
        }
        break;
    }
    return 0;
}
#undef LOCAL_PROTO
const plc_io_metods_t plc_iom_registry[] =
{
    PLC_IOM_RECORD(dio),
};
//Must be declared after plc_iom_registry
PLC_IOM_REG_SZ_DECL;

uint8_t mid_from_pid( uint16_t proto )
{
    switch(proto)
    {
    case 0:
        return 0;
    default:
        return PLC_IOM_MID_ERROR;
    }
    return PLC_IOM_MID_ERROR;
}
