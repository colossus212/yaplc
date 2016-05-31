
#include <plc_iom.h>

#define LOCAL_PROTO p1
void PLC_IOM_LOCAL_INIT(void)
{
}
bool PLC_IOM_LOCAL_TEST_HW(void)
{
    return true;
}
bool PLC_IOM_LOCAL_CHECK(uint16_t lid)
{
    return false;
}
void PLC_IOM_LOCAL_START(uint16_t lid)
{
}
void PLC_IOM_LOCAL_END(uint16_t lid)
{
}
uint32_t PLC_IOM_LOCAL_SCHED(uint16_t lid, uint32_t tick)
{
    return 0;
}
void PLC_IOM_LOCAL_POLL(uint32_t tick)
{
}
uint32_t PLC_IOM_LOCAL_WEIGTH(uint16_t lid)
{
    return plc_app->l_tab[lid]->a_data[0];
}
uint32_t PLC_IOM_LOCAL_GET(uint16_t lid)
{
    return 0;
}
uint32_t PLC_IOM_LOCAL_SET(uint16_t lid)
{
    return 0;
}
#undef LOCAL_PROTO
