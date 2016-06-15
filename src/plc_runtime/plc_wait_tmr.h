#ifndef _PLC_WAIT_TMR_H_
#define _PLC_WAIT_TMR_H_

extern uint32_t plc_sys_timer;

void plc_wait_tmr_init(void);

#define PLC_TIMER(t) (plc_sys_timer - t)
#define PLC_CLEAR_TIMER(t) (t = plc_sys_timer)

#endif /* _PLC_WAIT_TMR_H_ */
