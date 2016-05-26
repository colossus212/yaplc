#ifndef _PLC_IOM_ABI_H_
#define _PLC_IOM_ABI_H_

#define BOOL uint8_t

#define PLC_LOC_CONCAT(a,b) a##b

#define PLC_LT_I 0
#define PLC_LT_M 1
#define PLC_LT_Q 2

#define PLC_LOC_TYPE(a) PLC_LOC_CONCAT(PLC_LT_,a)

#define PLC_LSZ_X 0
#define PLC_LSZ_B sizeof(uint8_t)
#define PLC_LSZ_W sizeof(uint16_t)
#define PLC_LSZ_D sizeof(uint32_t)
#define PLC_LSZ_L sizeof(uint64_t)

#define PLC_LOC_SIZE(a) PLC_LOC_CONCAT(PLC_LSZ_,a)

typedef struct _plc_loc_dsc_t plc_loc_dsc_t;

struct _plc_loc_dsc_t
{
    void           *v_buf;
    uint8_t         v_type;
    uint8_t         v_size;
    uint16_t        a_size;
    const uint32_t *a_data;
    uint16_t        proto;
};

typedef const plc_loc_dsc_t * plc_loc_tbl_t;


/*!<plc_app_abi*/
typedef struct _plc_app_abi_t plc_app_abi_t;

struct _plc_app_abi_t
{
    plc_loc_tbl_t *  l_tab; //Location table
    uint32_t      *  w_tab;
    uint16_t         l_sz; //Location table size
};

extern plc_app_abi_t * app;

#endif // _PLC_IOM_ABI_H_
