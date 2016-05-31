#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <plc_config.h>
#include <plc_abi.h>/*plc_abi.h*/
#include <plc_iom.h>

plc_iom_t plc_iom;

#ifndef PLC_APP
#   define PLC_APP plc_curr_app
#endif

/*Insertion sort*/
static void _plc_loc_sort( uint16_t start, uint16_t end )
{
    int32_t       i, j;
    uint32_t      p_weigth;
    plc_loc_tbl_t p_loc;

    for (i=start; i < end; i++)
    {
        p_weigth = PLC_APP->w_tab[i];
        p_loc    = PLC_APP->l_tab[i];

        for (j = i-1; j >= start && PLC_APP->w_tab[j] > p_weigth; j--)
        {
            PLC_APP->w_tab[j+1] = PLC_APP->w_tab[j];
            PLC_APP->l_tab[j+1] = PLC_APP->l_tab[j];
        }
        PLC_APP->w_tab[++j] = p_weigth;
        PLC_APP->l_tab[ j ] = p_loc;
    }
}
//Sort locations with the same protocol used
static uint32_t _plc_proto_sort( uint16_t l_start, uint16_t l_end )
{
    int32_t i;
    uint16_t proto;
    uint8_t j;

    proto = PLC_APP->l_tab[l_start]->proto;

    for (i = l_start+1; i < l_end; i++)
    {
        if (proto != PLC_APP->l_tab[i]->proto)
        {
            l_end = i;
            break;
        }
    }

    _plc_loc_sort(l_start, l_end);
    //Register location chunk with the same proto
    j = mid_from_pid(proto);
    plc_iom_registry[j].start(l_start);
    plc_iom_registry[j].end(l_end);

    return l_end;
}
//Sort locations of the sam type
static void _plc_type_sort( uint16_t l_start, uint16_t l_end )
{
    while(l_start < l_end)
    {
        l_start = _plc_proto_sort(l_start, l_end);
    }
}

void plc_iom_init(void)
{
    int32_t i;

    plc_iom.m_start = 0;
    plc_iom.m_end   = 0;
    plc_iom.tflg    = false;
    plc_iom.tick    = 0;

    for (i = 0; i < plc_iom_reg_sz; i++)
    {
        plc_iom_registry[i].init();
    }
}

void plc_iom_tick(void)
{
    plc_iom.tflg = true;
    plc_iom.tick++;
}

bool plc_iom_test_hw(void)
{
    uint8_t j;
    for (j = 0; j < plc_iom_reg_sz; j++)
    {
        if (false == plc_iom_registry[j].test_hw())
        {
            return false;
        }
    }
    return true;
}

#ifndef PLC_IOM_ASSERT
#   include <stdio.h>
#   define PLC_IOM_ASSERT printf
#endif
bool plc_iom_check_and_sort(void)
{
    int32_t i, o_end, w;
    uint8_t j;

    o_end   = PLC_APP->l_sz;
    if( 0 == o_end )
    {
        return false;
    }

    for (i = 0; i < o_end; i++)
    {
        j = mid_from_pid( PLC_APP->l_tab[i]->proto );
        //Check protocol
        if (PLC_IOM_MID_ERROR == j)
        {
            PLC_IOM_ASSERT("Protocol %d is not supported!", (int)j);
            return false;
        }
        //Check location, plc_iom_registry[j].check(i) must print all error messages!
        if (false == plc_iom_registry[j].check(i))
        {
            return false;
        }
        //Weigth location
        w = PLC_APP->l_tab[i]->v_type;
        w <<= 8;
        w |= j;

        PLC_APP->w_tab[i] = w;
    }
    //Sort locations
    _plc_loc_sort(0, o_end);

    for (i = 0; i < o_end; i++)
    {
        //Weigth locations for protocol specific sort
        j = mid_from_pid( PLC_APP->l_tab[i]->proto );
        PLC_APP->w_tab[i] = plc_iom_registry[j].weigth(i);
        //Set plc_iom.m_start and plc_iom.m_end
        if (0 == plc_iom.m_start)
        {
            switch( PLC_APP->l_tab[i]->v_type )
            {
            case PLC_LT_I:
            default:
            {
                break;
            }
            case PLC_LT_M:
            {
                plc_iom.m_start = i;
                break;
            }
            case PLC_LT_Q:
            {
                plc_iom.m_start = i;
                plc_iom.m_end = i;
                break;
            }
            }
        }
        else
        {
            if (0 == plc_iom.m_end && PLC_LT_M != PLC_APP->l_tab[i]->v_type)
            {
                plc_iom.m_end = i;
            }
        }
    }
    // Protocol specific sort, each type of locations is sorted separately
    _plc_type_sort( 0,               plc_iom.m_start );//inputs
    _plc_type_sort( plc_iom.m_start, plc_iom.m_end   );//memory
    _plc_type_sort( plc_iom.m_end,   o_end           );//outputs
    // Clear weigths
    for (i = 0; i < o_end; i++)
    {
        PLC_APP->w_tab[i] = 0;
    }

    return true;
}

void plc_iom_get(void)
{
    int32_t i, m_end;
    uint8_t j;

    m_end = plc_iom.m_end;

    for (i = 0; i < m_end; i++)
    {
        j = mid_from_pid( plc_curr_app->l_tab[i]->proto );
        plc_curr_app->w_tab[i] += plc_iom_registry[j].get(i);
    }
}

void plc_iom_set(void)
{
    int32_t i, m_start, o_end;
    uint8_t j;

    m_start = plc_iom.m_start;
    o_end   = plc_curr_app->l_sz;

    for (i = m_start; i < o_end; i++)
    {
        j = mid_from_pid( plc_curr_app->l_tab[i]->proto );
        plc_curr_app->w_tab[i] += plc_iom_registry[j].set(i);
    }
}

void plc_iom_poll(void)
{
    int32_t i, o_end, tick;
    uint8_t j;

    PLC_DISABLE_INTERRUPTS();
    tick = plc_iom.tick;

    if (plc_iom.tflg)
    {
        plc_iom.tflg = false;
        PLC_ENABLE_INTERRUPTS();
        /*I/O scheduling is done on tick*/
        o_end = plc_curr_app->l_sz;

        for (i = 0; i < o_end; i++)
        {
            j = mid_from_pid( plc_curr_app->l_tab[i]->proto );
            plc_curr_app->w_tab[i] += plc_iom_registry[j].sched(i, tick);
        }
    }
    else
    {
        PLC_ENABLE_INTERRUPTS();
    }
    /*I/O polling is done always*/
    for (j = 0; j < plc_iom_reg_sz; j++)
    {
        plc_iom_registry[j].poll(tick);
    }
}
