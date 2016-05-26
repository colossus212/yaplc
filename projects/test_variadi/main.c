#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* ======================   plc_abi.h   ========================*/

#include <plc_iom_abi.h>

/*   ====================   plc_main.c   ===========================*/
/*!< TODO: добавть специальную секцию для
type * name = &(PLC_LOC_BUF(name));
Чтобы можно было разместть их во flash-памяти.
*/

#define PLC_LOC_BUF(name)  PLC_LOC_CONCAT(name, _BUF)
#define PLC_LOC_ADDR(name) PLC_LOC_CONCAT(name, _ADDR)
#define PLC_LOC_DSC(name)  PLC_LOC_CONCAT(name, _LDSC)

#define __LOCATED_VAR( type, name, lt, lsz, io_proto, ... ) \
type PLC_LOC_BUF(name);                                     \
type * name = &(PLC_LOC_BUF(name));                         \
const uint32_t PLC_LOC_ADDR(name)[] = {__VA_ARGS__};        \
const plc_loc_dsc_t PLC_LOC_DSC(name) =                     \
    {                                                       \
     .v_buf  = (void *)&(PLC_LOC_BUF(name)),                \
     .v_type = PLC_LOC_TYPE(lt),                            \
     .v_size = PLC_LOC_SIZE(lsz),                           \
     .a_size = sizeof(PLC_LOC_ADDR(name))/sizeof(uint32_t), \
     .a_data = &(PLC_LOC_ADDR(name)[0]),                    \
     .proto  = io_proto                                     \
    };

#include "LOCATED_VARIABLES.h"
#undef __LOCATED_VAR

#define __LOCATED_VAR(type, name, ...) &(PLC_LOC_DSC(name)),
plc_loc_tbl_t plc_loc_table[] =
{
#include "LOCATED_VARIABLES.h"
};
#undef __LOCATED_VAR

#define PLC_LOC_TBL_SIZE (sizeof(plc_loc_table)/sizeof(plc_loc_dsc_t *))

uint32_t plc_loc_weigth[PLC_LOC_TBL_SIZE];

/*!<plc_app_abi*/
const plc_app_abi_t plc_app_abi =
{
    .l_tab = &plc_loc_table[0],
    .w_tab = &plc_loc_weigth[0],
    .l_sz  = PLC_LOC_TBL_SIZE
};
/*========================================================================*/
#include <plc_iom.h>
/* =======================   proto1.c   ==================================*/
#define LOCAL_PROTO p1
void PLC_IOM_LOCAL_INIT(void)
{
}
bool PLC_IOM_LOCAL_CHECK(uint16_t lid)
{
    return true;
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
    int32_t i, o_end;

    o_end   = app->l_sz;
    for (i = 0; i < o_end; i++)
    {
        if (7 == app->l_tab[i]->proto)
        {
            printf("P1 poll %d\n", (int)i);
            app->w_tab[i] = 0;
        }
    }
}
uint32_t PLC_IOM_LOCAL_WEIGTH(uint16_t lid)
{
    return app->l_tab[lid]->a_data[0];
}
uint32_t PLC_IOM_LOCAL_GET(uint16_t lid)
{
    printf("P1 get sched %d %d\n", (int)lid, (int)app->w_tab[lid]+1);
    return 1;
}
uint32_t PLC_IOM_LOCAL_SET(uint16_t lid)
{
    printf("P1 set sched %d %d\n", (int)lid, (int)app->w_tab[lid]+1);
    return 1;
}
#undef LOCAL_PROTO
/* =======================   proto2.c   ==================================*/
#define LOCAL_PROTO p2
void PLC_IOM_LOCAL_INIT(void)
{
}
bool PLC_IOM_LOCAL_CHECK(uint16_t lid)
{
    return true;
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
    return app->l_tab[lid]->a_data[0];
}
uint32_t PLC_IOM_LOCAL_GET(uint16_t lid)
{
    printf("P2 get %d\n", (int)lid);
    return 0;
}
uint32_t PLC_IOM_LOCAL_SET(uint16_t lid)
{
    printf("P2 set %d\n", (int)lid);
    return 0;
}
#undef LOCAL_PROTO
/* =======================   glue code   =================================*/
const plc_io_metods_t plc_iom_registry[] =
{
    PLC_IOM_RECORD(p1),
    PLC_IOM_RECORD(p2)
};
//Must be declared after plc_iom_registry
PLC_IOM_REG_SZ_DECL;

uint8_t mid_from_pid( uint16_t proto )
{
    switch(proto)
    {
    case 1:
        return 1;
    case 7:
        return 0;
    default:
        return PLC_IOM_MID_ERROR;
    }
    return PLC_IOM_MID_ERROR;
}
/* =======================    main.c   ===================================*/
plc_app_abi_t * app = (plc_app_abi_t *)&plc_app_abi;

int main()
{
    int32_t i, o_end;

    plc_iom_init();
    plc_iom_check_and_sort();


    o_end   = app->l_sz;
    for (i = 0; i < o_end; i++)
    {
        printf( "(%d,%d,%d)\n",
                (int)app->l_tab[i]->v_type,
                (int)app->l_tab[i]->proto,
                (int)app->l_tab[i]->a_data[0]);
    }

    printf("\nGet inputs\n");
    plc_iom_get();

    printf("\nSet outputs\n");
    plc_iom_set();

    printf("\nPoll\n");
    plc_iom_poll();

    printf("\nTick\n");
    plc_iom_tick();

    printf("\nPoll\n");
    plc_iom_poll();

    return 0;
}
