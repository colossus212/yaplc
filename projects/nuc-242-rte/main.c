#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rtc.h>

#include <plc_config.h>
#include <plc_hw.h>
#include <plc_tick.h>
#include <plc_wait_tmr.h>
#include <plc_rtc.h>
#include <plc_backup.h>

uint32_t plc_hw_status = 0;

void plc_iom_tick(void)
{

}

const tm default_date =
{
    .tm_sec  = 0,
    .tm_min  = 0,
    .tm_hour = 0,
    .tm_day  = 1,
    .tm_mon  = 3,
    .tm_year = 2016
};

uint8_t retain_buff[10] = {5,4,3,2,1,6,7,8,9,10};

extern void dio_init(void);
int main(void)
{
    PLC_DISABLE_INTERRUPTS();
    rcc_clock_setup_in_hse_16mhz_out_72mhz();

    plc_boot_init();

    dio_init();

    plc_heart_beat_init();

    gpio_set(PLC_LED_STG_PORT, PLC_LED_STG_PIN);
    gpio_set(PLC_LED_STR_PORT, PLC_LED_STR_PIN);
    gpio_set(PLC_LED_TX_PORT, PLC_LED_TX_PIN);
    gpio_set(PLC_LED_RX_PORT, PLC_LED_RX_PIN);

    gpio_clear(PLC_LED_STG_PORT, PLC_LED_STG_PIN);
    gpio_clear(PLC_LED_STR_PORT, PLC_LED_STR_PIN);
    gpio_clear(PLC_LED_TX_PORT, PLC_LED_TX_PIN);
    gpio_clear(PLC_LED_RX_PORT, PLC_LED_RX_PIN);

    plc_tick_setup(0, 1000000);
    plc_wait_tmr_init();
    plc_backup_init();
    plc_rtc_init( (tm *)&default_date );
    PLC_ENABLE_INTERRUPTS();

    if( plc_hw_status )
    {
        gpio_set(PLC_LED_STR_PORT, PLC_LED_STR_PIN);
    }

    plc_backup_retain(0, 10,  (void *)retain_buff);

    plc_backup_validate();

    while(1){
        static uint32_t i = 0,i_z = 0;

        i = rtc_get_counter_val();
        if (i != i_z){
            i_z = i;
            gpio_toggle(PLC_LED_TX_PORT, PLC_LED_TX_PIN);
        }

        plc_heart_beat_poll();

        plc_set_dout(1, plc_get_din(1)||plc_get_din(5));
        plc_set_dout(2, plc_get_din(2)||plc_get_din(6));
        plc_set_dout(3, plc_get_din(3)||plc_get_din(7));
        plc_set_dout(4, plc_get_din(4)||plc_get_din(8));
    }
    return -1;
}
