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
#include <plc_dbg.h>

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

// Debug fifo buffer
void dbg_fifo_flush( dbg_fifo_t * fifo )
{
    fifo->bytes = 0;
    fifo->read = 0;
    fifo->write = 0;
}

int dbg_fifo_write_byte(dbg_fifo_t * fifo, unsigned char d)
{
    if( fifo->bytes < DBG_FIFO_SIZE )
    {
        fifo->buf[fifo->write++] = d;
        fifo->write %= DBG_FIFO_SIZE;
        fifo->bytes++;
        return 1; // Success
    }
    else
    {
        return 0; // Fail
    }
}

int dbg_fifo_write(dbg_fifo_t * fifo, unsigned char *d, unsigned short n)
{
    int res;
    res = 0;
    while( n-- )
    {
        if( dbg_fifo_write_byte( fifo, *d ) )
        {
            d++;
            res++;
        }
        else
        {
            break;
        }
    }
    return res; //Number of written bytes
}

int dbg_fifo_read_byte(dbg_fifo_t * fifo, unsigned char *d)
{
    if( fifo->bytes != 0 )
    {
        *d = fifo->buf[fifo->read++];
        fifo->read %= DBG_FIFO_SIZE;
        fifo->bytes--;
        return 1; //Success
    }
    else
    {
        return 0; //Fail
    }
}

int dbg_fifo_read( dbg_fifo_t * fifo, unsigned char *d, unsigned short n)
{
    int res;
    res = 0;
    while( n-- )
    {
        if( dbg_fifo_read_byte( fifo, d ) )
        {
            d++;
            res++;
        }
        else
        {
            break;
        }
    }
    return res; //Number of bytes read
}

uint8_t buf[256];


extern void dio_init(void);

int main(void)
{
    PLC_DISABLE_INTERRUPTS();
    //rcc_clock_setup_in_hse_16mhz_out_72mhz();
    plc_clock_setup();

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
    dbg_serial_init();
    PLC_ENABLE_INTERRUPTS();

    if( plc_hw_status )
    {
        gpio_set(PLC_LED_STR_PORT, PLC_LED_STR_PIN);
    }

    plc_backup_retain(0, 10,  (void *)retain_buff);

    plc_backup_validate();

    while(1){
        static uint32_t i = 0,i_z = 0, j=0;

        i = rtc_get_counter_val();
        if (i != i_z){
            i_z = i;
            gpio_toggle(PLC_LED_TX_PORT, PLC_LED_TX_PIN);
        }

        if (plc_tick_flag)
        {
            plc_tick_flag = false;
            j++;
        }
        if (j>=500)
        {
            j = 0;
            //gpio_toggle(PLC_LED_STG_PORT, PLC_LED_STG_PIN);
        }

        plc_heart_beat_poll();

        plc_set_dout(1, plc_get_din(1)||plc_get_din(5));
        plc_set_dout(2, plc_get_din(2)||plc_get_din(6));
        plc_set_dout(3, plc_get_din(3)||plc_get_din(7));
        plc_set_dout(4, plc_get_din(4)||plc_get_din(8));

        dbg_serial_write(buf, dbg_serial_read( buf, 128 ));
    }
    return -1;
}
