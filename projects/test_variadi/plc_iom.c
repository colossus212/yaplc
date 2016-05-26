#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <plc_iom_abi.h>/*plc_abi.h*/
#include <plc_iom.h>

plc_iom_t plc_iom;

/*Insertion sort*/
static void _plc_loc_sort( uint16_t start, uint16_t end )
{
    int32_t       i, j;
    uint32_t      p_weigth;
    plc_loc_tbl_t p_loc;

    for (i=start; i < end; i++)
    {
        p_weigth = app->w_tab[i];
        p_loc    = app->l_tab[i];

        for (j = i-1; j >= start && app->w_tab[j] > p_weigth; j--)
        {
            app->w_tab[j+1] = app->w_tab[j];
            app->l_tab[j+1] = app->l_tab[j];
        }
        app->w_tab[++j] = p_weigth;
        app->l_tab[ j ] = p_loc;
    }
}
//Sort locations with the same protocol used
static uint32_t _plc_proto_sort( uint16_t l_start, uint16_t l_end )
{
    int32_t i;
    uint16_t proto;
    uint8_t j;

    proto = app->l_tab[l_start]->proto;

    for (i = l_start+1; i < l_end; i++)
    {
        if (proto != app->l_tab[i]->proto)
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

#ifndef PLC_IOM_ASSERT
#   include <stdio.h>
#   define PLC_IOM_ASSERT printf
#endif
bool plc_iom_check_and_sort(void)
{
    int32_t i, o_end, w;
    uint8_t j;

    o_end   = app->l_sz;

    for (i = 0; i < o_end; i++)
    {
        j = mid_from_pid( app->l_tab[i]->proto );
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
        w = app->l_tab[i]->v_type;
        w <<= 8;
        w |= j;

        app->w_tab[i] = w;
    }
    //Sort locations
    _plc_loc_sort(0, o_end);

    for (i = 0; i < o_end; i++)
    {
        //Weigth locations for protocol specific sort
        j = mid_from_pid( app->l_tab[i]->proto );
        app->w_tab[i] = plc_iom_registry[j].weigth(i);
        //Set plc_iom.m_start and plc_iom.m_end
        if (0 == plc_iom.m_start)
        {
            if (PLC_LT_M == app->l_tab[i]->v_type)
            {
                plc_iom.m_start = i;
            }
        }
        else
        {
            if (0 == plc_iom.m_end && PLC_LT_M != app->l_tab[i]->v_type)
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
        app->w_tab[i] = 0;
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
        j = mid_from_pid( app->l_tab[i]->proto );
        app->w_tab[i] += plc_iom_registry[j].get(i);
    }
}

void plc_iom_set(void)
{
    int32_t i, m_start, o_end;
    uint8_t j;

    m_start = plc_iom.m_start;
    o_end   = app->l_sz;

    for (i = m_start; i < o_end; i++)
    {
        j = mid_from_pid( app->l_tab[i]->proto );
        app->w_tab[i] += plc_iom_registry[j].set(i);
    }
}

void plc_iom_poll(void)
{
    int32_t i, o_end, tick;
    uint8_t j;

    PLC_DISABLE_INTERRUPTS();
    if (plc_iom.tflg)
    {
        plc_iom.tflg = false;
        tick = plc_iom.tick;
        PLC_ENABLE_INTERRUPTS();
    }
    else
    {
        PLC_ENABLE_INTERRUPTS();
        return;
    }

    o_end = app->l_sz;

    for (i = 0; i < o_end; i++)
    {
        j = mid_from_pid( app->l_tab[i]->proto );
        app->w_tab[i] += plc_iom_registry[j].sched(i, tick);
    }

    for (j = 0; j < plc_iom_reg_sz; j++)
    {
        plc_iom_registry[j].poll(tick);
    }
}
