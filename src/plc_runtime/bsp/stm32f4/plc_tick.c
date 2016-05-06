#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <plc_tick.h>
#include <frac_div.h>

static bool systick_set_period(uint32_t period, uint32_t ahb, uint8_t clk_source)
{
    if (period >= (STK_RVR_RELOAD/ahb))
    {
        return false;
    }
    else
    {
        systick_set_clocksource( clk_source );
        systick_set_reload( ahb * period - 1 );
        return true;
    }
}

//����������� ������ � ��.
#define TICK_MIN_PER    100000ULL
//��������� ������
#define TICK_THR_PER 500000000ULL
//������� � ���
#define RCC_AHB_FREQ    168UL

//���������� ���������� �������.
static frac_div_t systick_ctrl;

//��������� ���������� �������
#define TICK_STATE_HIGH 0
#define TICK_STATE_MID  1
#define TICK_STATE_LOW  2
static uint32_t tick_state = TICK_STATE_HIGH;

void sys_tick_handler(void)
{
    switch( tick_state )
    {
        case TICK_STATE_MID:
        case TICK_STATE_HIGH:
        default:
        {
            systick_set_reload( RCC_AHB_FREQ * (uint32_t)frac_div_icalc( &systick_ctrl ) - 1 );
            plc_tick_flag = true;
            break;
        }
        case TICK_STATE_LOW:
        {
            plc_tick_flag = frac_div_run( &systick_ctrl );
            break;
        }
    }
}

bool plc_tick_flag = false;

//Tick period in ns
void plc_tick_setup( uint64_t tick_next, uint64_t tick_period )
{
    (void)tick_next;//disable warning

    if( tick_period < TICK_MIN_PER )
    {
        tick_period = TICK_MIN_PER;
    }

    if( tick_period < TICK_THR_PER )
    {
        //���������� ������� ������ ���������� �������.
        if( systick_set_period( (uint32_t)(tick_period/1000ULL), RCC_AHB_FREQ, STK_CSR_CLKSOURCE_AHB) )
        {
            //�������� �� 0.1�
            tick_state = TICK_STATE_HIGH;
            frac_div_init( &systick_ctrl, tick_period, 1000ULL ); //��������� ������
        }
        else
        {
            //�� 0.5�
            tick_state = TICK_STATE_MID;
            systick_set_period( (uint32_t)(tick_period/8000ULL), RCC_AHB_FREQ, STK_CSR_CLKSOURCE_AHB_DIV8);
            frac_div_init( &systick_ctrl, tick_period, 8000ULL ); //��������� �������
        }
    }
    else
    {
        //���������� ������� ������ ���������� �������.
        tick_state = TICK_STATE_LOW;
        systick_set_period( 500, RCC_AHB_FREQ, STK_CSR_CLKSOURCE_AHB);//500���
        frac_div_init( &systick_ctrl, tick_period, 500000ULL );
    }

    systick_counter_enable();
	systick_interrupt_enable();
}
