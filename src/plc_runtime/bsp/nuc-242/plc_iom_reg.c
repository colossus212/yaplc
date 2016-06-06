#include <plc_config.h>
#include <plc_abi.h>
#include <plc_wait_tmr.h>
#include <plc_hw.h>
#include <plc_iom.h>

PLC_IOM_METH_DECLS(dio);

const plc_io_metods_t plc_iom_registry[] =
{
    PLC_IOM_RECORD(dio),
};
//Must be declared after plc_iom_registry
PLC_IOM_REG_SZ_DECL;

uint8_t mid_from_pid( uint16_t proto )
{
    switch(proto)
    {
    case 0:
        return 0;
    default:
        return PLC_IOM_MID_ERROR;
    }
    return PLC_IOM_MID_ERROR;
}
