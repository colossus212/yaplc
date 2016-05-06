#ifndef _PLC_ABI_H_
#define _PLC_ABI_H_

#include <stdbool.h>
#include <iec_types_all.h>

typedef void (*app_fp_t) (void);

typedef struct
{
    uint32_t * sstart;
    app_fp_t entry;
    //App startup interface
    uint32_t * data_loadaddr;
    uint32_t * data_start;
    uint32_t * data_end;
    uint32_t * bss_end;
    app_fp_t * pa_start;
    app_fp_t * pa_end;
    app_fp_t * ia_start;
    app_fp_t * ia_end;
    app_fp_t * fia_start;
    app_fp_t * fia_end;
    //RTE Version control
    //Semantic versioning is used
    uint32_t rte_ver_major;
    uint32_t rte_ver_minor;
    uint32_t rte_ver_patch;
    //Control instance of PLC_ID
    const char    * check_id; //Must be placed to the end of .text
    //App interface
    const char    * id;       //Must be placed near the start of .text

    int (*start)(int ,char **);
    int (*stop)(void);
    void (*run)(void);

    void (*dbg_resume)(void);
    void (*dbg_suspend)(int);

    int  (*dbg_data_get)(unsigned long *, unsigned long *, void **);
    void (*dbg_data_free)(void);

    void (*dbg_vars_reset)(void);
    void (*dbg_var_register)(int, void *);

    uint32_t (*log_cnt_get)(uint8_t);
    uint32_t (*log_msg_get)(uint8_t, uint32_t, char*, uint32_t, uint32_t*, uint32_t*, uint32_t*);
    void     (*log_cnt_reset)(void);
}
plc_app_abi_t;

typedef struct
{
    void (*get_time)(IEC_TIME *);
    void (*set_timer)(unsigned long long,unsigned long long);

    int  (*check_retain_buf)(void);
    void (*invalidate_retain_buf)(void); //Вызывается перед сохранением
    void (*validate_retain_buf)(void);   //Вызывается после сохранения

    void (*retain)(unsigned int,unsigned int,void *);
    void (*remind)(unsigned int,unsigned int,void *);

    void (*set_dout)( uint32_t, bool );
    void (*set_aout)( uint32_t, uint32_t );
    bool     (*get_din)( uint32_t );
    uint32_t (*get_ain)( uint32_t );

    void (*set_mem)( uint32_t, uint32_t, uint8_t * ); // mem addr, size, buff addr
    void (*get_mem)( uint32_t, uint32_t, uint8_t * ); // mem addr, size, buff addr
}
plc_rte_abi_t;

#endif // _PLC_ABI_H_
