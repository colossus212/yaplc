#ifndef _PLC_TICK_H_
#define _PLC_TICK_H_

#include <stdbool.h>

extern bool plc_tick_flag;

void plc_tick_setup( unsigned long long tick_next, unsigned long long tick_period );

#endif /* _PLC_TICK_H_ */
