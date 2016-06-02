#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


#include <plc_config.h>
#include <plc_hw.h>

void do_nothing(void)
{

}
extern void dio_init(void);
void main(void)
{
    rcc_clock_setup_in_hse_16mhz_out_72mhz();

    plc_boot_init();

    dio_init();

    plc_heart_beat_init();

    gpio_set( PLC_LED_STG_PORT, PLC_LED_STG_PIN );
    gpio_set( PLC_LED_STR_PORT, PLC_LED_STR_PIN );
    gpio_set( PLC_LED_TX_PORT, PLC_LED_TX_PIN );
    gpio_set( PLC_LED_RX_PORT, PLC_LED_RX_PIN );

    gpio_clear( PLC_LED_STG_PORT, PLC_LED_STG_PIN );
    gpio_clear( PLC_LED_STR_PORT, PLC_LED_STR_PIN );
    gpio_clear( PLC_LED_TX_PORT, PLC_LED_TX_PIN );
    gpio_clear( PLC_LED_RX_PORT, PLC_LED_RX_PIN );


    while(1)
    {
        plc_set_dout(1,plc_get_din(1)||plc_get_din(5));
        plc_set_dout(2,plc_get_din(2)||plc_get_din(6));
        plc_set_dout(3,plc_get_din(3)||plc_get_din(7));
        plc_set_dout(4,plc_get_din(4)||plc_get_din(8));
    }
}
