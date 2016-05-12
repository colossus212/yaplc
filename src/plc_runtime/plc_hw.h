#ifndef _PLC_HW_H_
#define _PLC_HW_H_

#include <stdbool.h>
#include <stdint.h>

#define PLC_HW_ERR_HSE 0x1
#define PLC_HW_ERR_LSE 0x2

#define PLC_HW_ERR_CRITICAL 0xFC

extern uint32_t plc_hw_status;

void plc_hw_init(void);

bool plc_get_din(uint32_t i);
void plc_set_dout( uint32_t i, bool val );

uint32_t plc_get_ain(uint32_t i);
void plc_set_aout( uint32_t i, uint32_t val );


bool plc_check_hw(void);
bool plc_dbg_jmpr_get(void);
bool plc_rst_jmpr_get(void);

void plc_heart_beat(void);

void enter_boot_mode(void);

#endif /* _PLC_HW_H_ */
