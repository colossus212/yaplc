#ifndef _PLC_IO_PROTO_*_H_
#define _PLC_IO_PROTO_*_H_

#include <plc_iom.h>

#define LOCAL_PROTO p*
void PLC_IOM_LOCAL_INIT(void);
bool PLC_IOM_LOCAL_CHECK(uint16_t lid);
void PLC_IOM_LOCAL_START(uint16_t lid);
void PLC_IOM_LOCAL_END(uint16_t lid);
uint32_t PLC_IOM_LOCAL_SCHED(uint16_t lid, uint32_t tick);
void PLC_IOM_LOCAL_POLL(uint32_t tick)
uint32_t PLC_IOM_LOCAL_WEIGTH(uint16_t lid);
uint32_t PLC_IOM_LOCAL_GET(uint16_t lid);
uint32_t PLC_IOM_LOCAL_SET(uint16_t lid);
#undef LOCAL_PROTO

#endif // _PLC_IO_PROTO_*_H_
