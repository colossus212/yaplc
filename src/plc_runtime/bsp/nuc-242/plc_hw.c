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
    rcc_periph_clock_enable(PLC_JMP_RST_PERIPH);
    gpio_set_mode(PLC_JMP_RST_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_JMP_RST_PIN);
    gpio_clear(PLC_JMP_RST_PORT, PLC_JMP_RST_PIN); //Pull down!
    /*!TODO: Add debug jumper!!!*/
}

bool plc_dbg_jmpr_get(void)
{
    ///Write your code here!!!
    return false;
}

volatile bool reset_jmp = false;

bool plc_rst_jmpr_get(void)
{
    return gpio_get(PLC_JMP_RST_PORT, PLC_JMP_RST_PIN);
}

void plc_boot_init(void)
{
    //Boot pin config
    rcc_periph_clock_enable( RCC_AFIO );
    AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON;

    rcc_periph_clock_enable( PLC_BOOT_PERIPH );
    gpio_set_mode(PLC_BOOT_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_BOOT_PIN);
    gpio_set( PLC_BOOT_PORT, PLC_BOOT_PIN ); //Exit boot mode
}

void plc_boot_mode_enter(void)
{
//    uint32_t delay;
    gpio_clear( PLC_BOOT_PORT, PLC_BOOT_PIN );
//
//    PLC_CLEAR_TIMER( delay );
//    while( PLC_TIMER(delay) < 2000 );
//    scb_reset_system();
}


//Led blink timer
static uint32_t blink_tmr;
void plc_heart_beat_init(void)
{
    //LEDs
    PLC_CLEAR_TIMER( blink_tmr );

    rcc_periph_clock_enable( PLC_LED_STG_PERIPH );
    gpio_set_mode(PLC_LED_STG_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_LED_STG_PIN);
    gpio_clear( PLC_LED_STG_PORT, PLC_LED_STG_PIN );

    rcc_periph_clock_enable( PLC_LED_STR_PERIPH );
    gpio_set_mode(PLC_LED_STR_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_LED_STR_PIN);
    gpio_clear( PLC_LED_STR_PORT, PLC_LED_STR_PIN );

    rcc_periph_clock_enable( PLC_LED_TX_PERIPH );
    gpio_set_mode(PLC_LED_TX_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_LED_TX_PIN);
    gpio_clear( PLC_LED_TX_PORT, PLC_LED_TX_PIN );

    rcc_periph_clock_enable( PLC_LED_RX_PERIPH );
    gpio_set_mode(PLC_LED_RX_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_LED_RX_PIN);
    gpio_clear( PLC_LED_RX_PORT, PLC_LED_RX_PIN );
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
        //if(  *(uint8_t *)BKPSRAM_BASE == 1 )
        if(plc_hw_status  & PLC_HW_ERR_HSE)
        {
            if( hse_post_flag )
            {
                hse_post_flag = false;
                //plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_hse_err_msg, sizeof(plc_hse_err_msg));
            }
            gpio_set( PLC_LED_STR_PORT, PLC_LED_STR_PIN );
        }
        else
        {
            gpio_set( PLC_LED_STG_PORT, PLC_LED_STG_PIN );
        }
        //if( *( (uint8_t *)BKPSRAM_BASE + 1) == 1 )
        if(plc_hw_status  & PLC_HW_ERR_LSE)
        {
            if( lse_post_flag )
            {
                lse_post_flag = false;
                //plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_lse_err_msg, sizeof(plc_lse_err_msg));
            }
            gpio_set( PLC_LED_STR_PORT, PLC_LED_STR_PIN );
        }
    }

    if( PLC_TIMER(blink_tmr) > blink_thr )
    {
        PLC_CLEAR_TIMER(blink_tmr);
        gpio_clear( PLC_LED_STR_PORT, PLC_LED_STR_PIN );
        gpio_clear( PLC_LED_STG_PORT, PLC_LED_STG_PIN );
    }
}

#ifdef PLC_CFG_INPF
/*50Hz filters*/
#define PLC_INPF_TIMEOUT 20 /*20ms timeout*/
typedef struct
{
    uint32_t cnt;
    bool    flg;
} plc_inpf;

void plc_inpf_init(plc_inpf * self)
{
    self->flg = false;
    self->cnt = 0;
}

bool plc_inpf_do(plc_inpf * self, bool pin)
{
    if (pin)
    {
        self->flg = true;
        self->cnt = 0;
        return true;
    }
    else
    {
        return self->flg;
    }
}

void plc_inpf_poll(plc_inpf * self, bool pin)
{
    if (pin)
    {
        self->flg = true;
        self->cnt = 0;
    }
    else
    {
        if (PLC_INPF_TIMEOUT < self->cnt)
        {
            self->flg = false;
        }
        else
        {
            self->cnt++;
        }
    }
}

plc_inpf inpf[8];

#define PLC_INPF_INIT(i) plc_inpf_init(inpf+i-1)
#define PLC_INPF_GET(i) (plc_inpf_do(inpf+i-1, plc_get_din(i)))
#define PLC_INPF_POLL(i) plc_inpf_poll(inpf+i-1, plc_get_din(i))

#else //PLC_CFG_INPF

#define PLC_INPF_INIT(i) do{}while(0)
#define PLC_INPF_GET(i) (plc_get_din(i))
#define PLC_INPF_POLL(i) do{}while(0)

#endif //PLC_CFG_INPF
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
//Digital io
#define LOCAL_PROTO dio
void PLC_IOM_LOCAL_INIT(void)
{
    ///Outputs
    //DO1
    rcc_periph_clock_enable( PLC_O1_PERIPH );
    gpio_set_mode(PLC_O1_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_O1_PIN);
    gpio_clear( PLC_O1_PORT, PLC_O1_PIN );
    //DO2
    rcc_periph_clock_enable( PLC_O2_PERIPH );
    gpio_set_mode(PLC_O2_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_O2_PIN);
    gpio_clear( PLC_O2_PORT, PLC_O2_PIN );
    //DO3
    rcc_periph_clock_enable( PLC_O3_PERIPH );
    gpio_set_mode(PLC_O3_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_O3_PIN);
    gpio_clear( PLC_O3_PORT, PLC_O3_PIN );
    //DO4
    rcc_periph_clock_enable( PLC_O4_PERIPH );
    gpio_set_mode(PLC_O4_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PLC_O4_PIN);
    gpio_clear( PLC_O4_PORT, PLC_O4_PIN );
    ///Inputs
    //DI1
    PLC_INPF_INIT(1);
    rcc_periph_clock_enable(PLC_I1_PERIPH);
    gpio_set_mode(PLC_I1_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I1_PIN);
    gpio_set(PLC_I1_PORT, PLC_I1_PIN);
    //DI2
    PLC_INPF_INIT(2);
    rcc_periph_clock_enable(PLC_I2_PERIPH);
    gpio_set_mode(PLC_I2_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I2_PIN);
    gpio_set(PLC_I2_PORT, PLC_I2_PIN);
    //DI3
    PLC_INPF_INIT(3);
    rcc_periph_clock_enable(PLC_I3_PERIPH);
    gpio_set_mode(PLC_I3_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I3_PIN);
    gpio_set(PLC_I3_PORT, PLC_I3_PIN);
    //DI4
    PLC_INPF_INIT(4);
    rcc_periph_clock_enable(PLC_I4_PERIPH);
    gpio_set_mode(PLC_I4_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I4_PIN);
    gpio_set(PLC_I4_PORT, PLC_I4_PIN);
    //DI5
    PLC_INPF_INIT(5);
    rcc_periph_clock_enable(PLC_I5_PERIPH);
    gpio_set_mode(PLC_I5_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I5_PIN);
    gpio_set(PLC_I5_PORT, PLC_I5_PIN);
    //DI6
    PLC_INPF_INIT(6);
    rcc_periph_clock_enable(PLC_I6_PERIPH);
    gpio_set_mode(PLC_I6_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I6_PIN);
    gpio_set(PLC_I6_PORT, PLC_I6_PIN);
    //DI7
    PLC_INPF_INIT(7);
    rcc_periph_clock_enable(PLC_I7_PERIPH);
    gpio_set_mode(PLC_I7_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I7_PIN);
    gpio_set(PLC_I7_PORT, PLC_I7_PIN);
    //DI8
    PLC_INPF_INIT(8);
    rcc_periph_clock_enable(PLC_I8_PERIPH);
    gpio_set_mode(PLC_I8_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, PLC_I8_PIN);
    gpio_set(PLC_I8_PORT, PLC_I8_PIN);
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
    PLC_APP->log_msg_post(LOG_DEBUG, (char *)print_buf, cnt+1);
}

const char plc_iom_err_proto[] = "IO protocol is not supported!";
const uint32_t plc_iom_err_psz = sizeof(plc_iom_err_proto);

const char plc_dio_err_sz[]    = "All digital io mus be BOOL!";
const char plc_dio_err_asz[]   = "Digital io address must be one number!";
const char plc_dio_err_tp[]    = "Digital io does not support memory locations!";
const char plc_dio_err_ilim[]  = "Digital input must have address 1...8!";
const char plc_dio_err_olim[]  = "Digital input must have address 1...4!";

bool PLC_IOM_LOCAL_CHECK(uint16_t i)
{
    uint32_t addr;
    //Check size
    if (PLC_LSZ_X != PLC_APP->l_tab[i]->v_size)
    {
        plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_sz, sizeof(plc_dio_err_sz));
        return false;
    }

    if (1 != PLC_APP->l_tab[i]->a_size)
    {
        plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_asz, sizeof(plc_dio_err_asz));
        return false;
    }

    addr = PLC_APP->l_tab[i]->a_data[0];
    //Check type and address
    switch (PLC_APP->l_tab[i]->v_type)
    {
    case PLC_LT_I:
    {
        if (addr < 1 || addr > 8)
        {
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_ilim, sizeof(plc_dio_err_ilim));
            return false;
        }
        else
        {
            return true;
        }
    }
    case PLC_LT_M:
    default:
    {
        plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_sz, sizeof(plc_dio_err_sz));
        return false;
    }
    case PLC_LT_Q:
    {
        if (addr < 1 || addr > 4)
        {
            plc_curr_app->log_msg_post(LOG_CRITICAL, (char *)plc_dio_err_olim, sizeof(plc_dio_err_olim));
            return false;
        }
        else
        {
            return true;
        }
    }
    }
    return true;
}

void PLC_IOM_LOCAL_START(uint16_t lid)
{
}
void PLC_IOM_LOCAL_END(uint16_t lid)
{
}
uint32_t PLC_IOM_LOCAL_SCHED(uint16_t i, uint32_t tick)
{
    PLC_INPF_POLL( plc_curr_app->l_tab[i]->a_data[0] );
    return 0;
}
void PLC_IOM_LOCAL_POLL(uint32_t tick)
{
}
uint32_t PLC_IOM_LOCAL_WEIGTH(uint16_t lid)
{
    return PLC_APP->l_tab[lid]->a_data[0];
}
uint32_t PLC_IOM_LOCAL_GET(uint16_t i)
{
    if( PLC_LT_I == plc_curr_app->l_tab[i]->v_type )
    {
        *(bool *)(plc_curr_app->l_tab[i]->v_buf) = PLC_INPF_GET( plc_curr_app->l_tab[i]->a_data[0] );
    }
    return 0;
}
uint32_t PLC_IOM_LOCAL_SET(uint16_t i)
{
    if( PLC_LT_Q == plc_curr_app->l_tab[i]->v_type )
    {
        plc_set_dout( plc_curr_app->l_tab[i]->a_data[0], *(bool *)(plc_curr_app->l_tab[i]->v_buf) );
    }
    return 0;
}
#undef LOCAL_PROTO
