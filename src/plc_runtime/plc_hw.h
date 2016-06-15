#ifndef _PLC_HW_H_
#define _PLC_HW_H_

#include <stdbool.h>
#include <stdint.h>

#define PLC_HW_ERR_HSE 0x1
#define PLC_HW_ERR_LSE 0x2

#define PLC_HW_ERR_CRITICAL 0xFC

extern uint32_t plc_hw_status;
void plc_jmpr_init(void);
bool plc_dbg_jmpr_get(void);
bool plc_rst_jmpr_get(void);

void plc_heart_beat_init(void);
void plc_heart_beat_poll(void);

void plc_boot_init(void);
void plc_boot_mode_enter(void);

bool plc_get_din(uint32_t i);
void plc_set_dout( uint32_t i, bool val );

uint32_t plc_get_ain(uint32_t i);
void plc_set_aout( uint32_t i, uint32_t val );
#endif /* _PLC_HW_H_ */
