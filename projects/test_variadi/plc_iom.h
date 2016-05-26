#ifndef _PLC_IOM_H_
#define _PLC_IOM_H_

typedef struct _plc_io_metods_t plc_io_metods_t;
struct _plc_io_metods_t
{
    void     (*init)  (void);                /* void     init  ( void);                                */
    bool     (*check) (uint16_t);            /* bool     check ( uint16_t lid );                       */
    void     (*start) (uint16_t);            /* void     start ( uint16_t lid );                       */
    void     (*end)   (uint16_t);            /* void     end   ( uint16_t lid );                       */
    uint32_t (*sched) (uint16_t, uint32_t);  /* uint32_t sched ( uint16_t lid, uint32_t tick );        */
    void     (*poll)  (uint32_t);            /* void     poll  ( uint32_t tick );                      */
    uint32_t (*weigth)(uint16_t);            /* uint32_t weigth( uint16_t lid );                       */
    uint32_t (*get)   (uint16_t);            /* uint32_t get   ( uint16_t lid );                       */
    uint32_t (*set)   (uint16_t);            /* uint32_t set   ( uint16_t lid );                       */
};

extern const plc_io_metods_t plc_iom_registry[];
extern const uint8_t plc_iom_reg_sz;

extern uint8_t mid_from_pid( uint16_t proto );
#define PLC_IOM_MID_ERROR (0xff)
// plc_iom_reg_sz must be declared after plc_iom_registry initiation
// as before initiation plc_iom_registry has inclomplete type
#define PLC_IOM_REG_SZ_DECL const uint8_t plc_iom_reg_sz = (uint8_t)(sizeof(plc_iom_registry)/sizeof(plc_io_metods_t))

#define PLC_IOM_CONCAT(a,b)       a##b
#define PLC_IOM_CONCAT2(a,b)      PLC_IOM_CONCAT(a,b)

#define PLC_IOM_RC_LINE(proto,method) .method = PLC_IOM_CONCAT2(proto, PLC_IOM_CONCAT(_,method))

#define PLC_IOM_RECORD(proto)     \
{                                 \
    PLC_IOM_RC_LINE(proto,init),  \
    PLC_IOM_RC_LINE(proto,check), \
    PLC_IOM_RC_LINE(proto,start), \
    PLC_IOM_RC_LINE(proto,end),   \
    PLC_IOM_RC_LINE(proto,sched), \
    PLC_IOM_RC_LINE(proto,poll),  \
    PLC_IOM_RC_LINE(proto,weigth),\
    PLC_IOM_RC_LINE(proto,get),   \
    PLC_IOM_RC_LINE(proto,set),   \
}

#define PLC_IOM_LOCAL_INIT   PLC_IOM_CONCAT2(LOCAL_PROTO,_init)
#define PLC_IOM_LOCAL_CHECK  PLC_IOM_CONCAT2(LOCAL_PROTO,_check)
#define PLC_IOM_LOCAL_START  PLC_IOM_CONCAT2(LOCAL_PROTO,_start)
#define PLC_IOM_LOCAL_END    PLC_IOM_CONCAT2(LOCAL_PROTO,_end)
#define PLC_IOM_LOCAL_SCHED  PLC_IOM_CONCAT2(LOCAL_PROTO,_sched)
#define PLC_IOM_LOCAL_POLL   PLC_IOM_CONCAT2(LOCAL_PROTO,_poll)
#define PLC_IOM_LOCAL_WEIGTH PLC_IOM_CONCAT2(LOCAL_PROTO,_weigth)
#define PLC_IOM_LOCAL_GET    PLC_IOM_CONCAT2(LOCAL_PROTO,_get)
#define PLC_IOM_LOCAL_SET    PLC_IOM_CONCAT2(LOCAL_PROTO,_set)

typedef struct _plc_iom_t plc_iom_t;

struct _plc_iom_t
{
    uint32_t tick;    /*!<tick counter           */
    uint16_t m_start; /*!<Memory locations start */
    uint16_t m_end;   /*!<Memory locations end   */
    bool     tflg;    /*!<tick flag              */
};

extern plc_iom_t plc_iom; /*!<plc io maschine*/

void plc_iom_init(void);
void plc_iom_tick(void);
bool plc_iom_check_and_sort(void);
void plc_iom_get(void);
void plc_iom_set(void);
void plc_iom_poll(void);

#endif // _PLC_IOM_H_
