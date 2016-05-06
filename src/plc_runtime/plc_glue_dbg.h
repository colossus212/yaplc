#ifndef _PLC_DBG_GLUE_H_
#define _PLC_DBG_GLUE_H_

#include <plc_abi.h>

#define PLC_APP (&plc_glue_app)

#define PLC_ADDR(a) (&a)

extern const plc_app_abi_t plc_glue_app;
extern const plc_rte_abi_t plc_glue_rte;



#endif // _PLC_DBG_GLUE_H_
