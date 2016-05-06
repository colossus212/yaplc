#ifndef _DBG_BACKUP_H_
#define _DBG_BACKUP_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rtc.h>

void plc_backup_init(void);
void plc_backup_invalidate(void);
void plc_backup_validate(void);
int plc_backup_check(void);
void plc_backup_remind(unsigned int offset, unsigned int count, void *p);
void plc_backup_retain(unsigned int offset, unsigned int count, void *p);
#endif /* _DBG_BACKUP_H_ */
