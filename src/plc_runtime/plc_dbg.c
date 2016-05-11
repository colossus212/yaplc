#include "plc_dbg.h"

#include <plc_backup.h>
#include <plc_hw.h>

#include <plc_config.h>


// Debug fifo buffer
void dbg_fifo_flush( dbg_fifo_t * fifo )
{
    fifo->bytes = 0;
    fifo->read = 0;
    fifo->write = 0;
}

int dbg_fifo_write_byte(dbg_fifo_t * fifo, unsigned char d)
{
    if( fifo->bytes < DBG_FIFO_SIZE )
    {
        fifo->buf[fifo->write++] = d;
        fifo->write %= DBG_FIFO_SIZE;
        fifo->bytes++;
        return 1; // Success
    }
    else
    {
        return 0; // Fail
    }
}

int dbg_fifo_write(dbg_fifo_t * fifo, unsigned char *d, unsigned short n)
{
    int res;
    res = 0;
    while( n-- )
    {
        if( dbg_fifo_write_byte( fifo, *d ) )
        {
            d++;
            res++;
        }
        else
        {
            break;
        }
    }
    return res; //Number of written bytes
}

int dbg_fifo_read_byte(dbg_fifo_t * fifo, unsigned char *d)
{
    if( fifo->bytes != 0 )
    {
        *d = fifo->buf[fifo->read++];
        fifo->read %= DBG_FIFO_SIZE;
        fifo->bytes--;
        return 1; //Success
    }
    else
    {
        return 0; //Fail
    }
}

int dbg_fifo_read( dbg_fifo_t * fifo, unsigned char *d, unsigned short n)
{
    int res;
    res = 0;
    while( n-- )
    {
        if( dbg_fifo_read_byte( fifo, d ) )
        {
            d++;
            res++;
        }
        else
        {
            break;
        }
    }
    return res; //Number of bytes read
}

//====================================================================================
plc_dbg_proto_t plc_dbg_ctrl =
{
    .state = GET_CMD
};

// Application API
//const char * PLC_ID = PLC_MD5;
//
//extern unsigned long __tick;
//
//extern int startPLC(int argc,char **argv);
//extern int stopPLC();
//
//extern void resumeDebug(void);
//extern void suspendDebug(int disable);
//extern void FreeDebugData(void);
//extern void __publish_debug(void);
//extern void ResetDebugVariables(void);
//extern void RegisterDebugVariable(int idx, void* force);
//
//extern int GetDebugData(unsigned long *tick, unsigned long *size, void **buffer);
//
//extern void ResetLogCount(void);
//extern uint32_t GetLogCount(uint8_t level);
//extern uint32_t GetLogMessage(uint8_t level, uint32_t msgidx, char* buf, uint32_t max_size, uint32_t* tick, uint32_t* tv_sec, uint32_t* tv_nsec);

void dbg_init(void)
{
    dbg_serial_init();
    plc_dbg_ctrl.state = GET_CMD;
    plc_dbg_ctrl.timer = 0;
    plc_dbg_ctrl.data_hook = (void(*)(void))0;
}

void dbg_handler(void)
{
    //Check timer, break connection on timeout
    if( PLC_TIMER( plc_dbg_ctrl.timer ) > PLC_DBG_TIMER_THR )
    {
        plc_dbg_ctrl.state = GET_CMD;
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );
        plc_dbg_ctrl.data_hook = (void(*)(void))0;
    }
    //Bebug FSM
    switch( plc_dbg_ctrl.state )
    {
    case GET_CMD:
    default:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( dbg_serial_read(&plc_dbg_ctrl.cmd, 1) )
        {
            //Handle command stage 1 (Immediate actions)
            switch( plc_dbg_ctrl.cmd )
            {
            case DBG_CMD_START:
            {
                plc_state = PLC_STATE_STARTED;
                plc_app->start(0,0);
            }
            break;

            case DBG_CMD_STOP:
            {
                plc_state = PLC_STATE_STOPED;
                plc_app->stop();
            }
            break;
            }

            // Answer to host
            plc_dbg_ctrl.state = PUT_ACK;

            plc_dbg_ctrl.tr_buf.data[0] = plc_dbg_ctrl.cmd;
            plc_dbg_ctrl.tr_buf.data[1] = plc_state;

            plc_dbg_ctrl.tmp_len = 2;
            plc_dbg_ctrl.tmp = &plc_dbg_ctrl.tr_buf.data[0];
        }
    }
    break;

    case PUT_ACK:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp     += write_res;
            plc_dbg_ctrl.tmp_len  -= write_res;
        }
        else
        {
            plc_dbg_ctrl.state = PARSE_CMD;
        }
    }
    break;

    case PARSE_CMD:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );
        //Handle command stage 2
        switch( plc_dbg_ctrl.cmd )
        {

        case DBG_CMD_START:
        case DBG_CMD_STOP:
        {
            plc_dbg_ctrl.state = GET_CMD;
        }
        break;

        case DBG_CMD_RESET:
        {
            plc_backup_invalidate();
            enter_boot_mode();
        }
        break;

        case DBG_CMD_GET_V:
        {
            plc_dbg_ctrl.state = GET_DEBUG_DATA;
            plc_dbg_ctrl.data_hook = plc_app->dbg_data_free; //Free buffer when transfer ends.
        }
        break;

        case DBG_CMD_SET_V:
        {
            plc_dbg_ctrl.state = GET_DATA_LEN;

            plc_app->dbg_suspend(0);
            plc_app->dbg_vars_reset();
            //Get 4 byte data length, little endian
            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.data_len;
        }
        break;

        case DBG_CMD_GET_ID:
        {
            plc_dbg_ctrl.state = PUT_ID_LEN;
            //Transfer 4 byte data length, little endian
            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.data_len;
            //Than Transfer 32 bytes of data
            plc_dbg_ctrl.data_len = 32;
            plc_dbg_ctrl.data = (unsigned char *)plc_app->id;
        }
        break;

        case DBG_CMD_GET_LC:
        {
            unsigned char i;
            for(i = 0; i < 4; i++)
            {
                plc_dbg_ctrl.tr_buf.log_cnt[i] = plc_app->log_cnt_get(i);
            }
            plc_dbg_ctrl.state = PUT_LC_LEN;
            //Transfer 4 byte data length, little endian
            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.data_len;
            //Than Transfer 32 bytes of data
            plc_dbg_ctrl.data_len = 16;
            plc_dbg_ctrl.data = (unsigned char *)&plc_dbg_ctrl.tr_buf.log_cnt[0];
        }
        break;

        case DBG_CMD_GET_LM:
        {
            plc_dbg_ctrl.state = GET_MSG_LEVEL;
        }
        break;

        case DBG_CMD_CLR_LM:
        {
            plc_dbg_ctrl.state = GET_CMD;
            plc_app->log_cnt_reset();
        }
        break;

        case DBG_CMD_IDLE:
        default:
        {
            plc_dbg_ctrl.state = GET_CMD;
        }
        break;

        }
    }
    break;
    //==========================================================================================
    case GET_DEBUG_DATA:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( 0 == plc_app->dbg_data_get( &plc_dbg_ctrl.tr.get_val.tick, (long unsigned int *)&plc_dbg_ctrl.data_len, (void **)&plc_dbg_ctrl.data ) )
        {
            //Transfer data
            plc_dbg_ctrl.data_len += 4; //Must transfer tick
        }
        else
        {
            plc_dbg_ctrl.data_len = 0; //No data available
        }
        plc_dbg_ctrl.state = PUT_DEBUG_LEN;

        plc_dbg_ctrl.tmp_len = 4;
        plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.data_len;
    }
    break;

    case PUT_DEBUG_LEN:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
        }
        else
        {
            if(plc_dbg_ctrl.data_len > 0)
            {
                //When length transfered transfer data
                plc_dbg_ctrl.state = PUT_DEBUG_TICK;
                plc_dbg_ctrl.tmp_len = 4;
                plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.get_val.tick;
            }
            else
            {
                plc_dbg_ctrl.state = GET_CMD;
            }
        }
    }
    break;

    case PUT_DEBUG_TICK:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
            plc_dbg_ctrl.data_len -= write_res;
        }
        else
        {
            //When length transfered transfer data
            plc_dbg_ctrl.state = PUT_DATA;
        }
    }
    break;
    //==========================================================================================
    case GET_DATA_LEN:
    {
        if( plc_dbg_ctrl.tmp_len )
        {
            int read_res;
            //Must read before timeout!
            read_res = dbg_serial_read( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp     += read_res;
            plc_dbg_ctrl.tmp_len  -= read_res;
        }
        else
        {
            PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

            if( plc_dbg_ctrl.data_len > 0 )
            {
                plc_dbg_ctrl.state = GET_VAR_IDX;
                plc_dbg_ctrl.tmp_len = 4;
                plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.set_val.var_idx;
            }
            else
            {
                plc_app->dbg_resume();
                plc_dbg_ctrl.state = GET_CMD;
            }
        }
    }
    break;

    case GET_VAR_IDX:
    {
        if( plc_dbg_ctrl.tmp_len )
        {
            //Must read before timeout!
            int read_res;
            read_res = dbg_serial_read( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp         += read_res;
            plc_dbg_ctrl.tmp_len     -= read_res;
            plc_dbg_ctrl.data_len    -= read_res;
        }
        else
        {
            PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

            plc_dbg_ctrl.state = GET_FORCE_LEN;
        }
    }
    break;

    case GET_FORCE_LEN:
    {
        if( dbg_serial_read( &plc_dbg_ctrl.tmp_len, 1) )
        {
            PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

            plc_dbg_ctrl.data_len--;
            if( 0 == plc_dbg_ctrl.tmp_len )
            {
                //No force data
                plc_app->dbg_var_register( plc_dbg_ctrl.tr.set_val.var_idx, (void *)0 );

                if( plc_dbg_ctrl.data_len <= 0 )
                {
                    //When all data transfered get next command
                    plc_app->dbg_resume();
                    plc_dbg_ctrl.state = GET_CMD;
                }
                else
                {
                    plc_dbg_ctrl.state = GET_VAR_IDX;
                    plc_dbg_ctrl.tmp_len = 4;
                    plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.set_val.var_idx;
                }
            }
            else
            {
                plc_dbg_ctrl.state = GET_FORCE_DATA;
                plc_dbg_ctrl.data = (unsigned char *)&plc_dbg_ctrl.tr_buf.data[0];
            }
        }
    }
    break;

    case GET_FORCE_DATA:
    {
        if( plc_dbg_ctrl.tmp_len )
        {
            int read_res;
            //Must read before timeout!
            read_res = dbg_serial_read( plc_dbg_ctrl.data, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.data        += read_res;
            plc_dbg_ctrl.tmp_len     -= (unsigned char)read_res;
            plc_dbg_ctrl.data_len    -= read_res;
            //Panic on error
        }
        else
        {
            PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

            plc_app->dbg_var_register( plc_dbg_ctrl.tr.set_val.var_idx, &plc_dbg_ctrl.tr_buf.data[0] );
            if( plc_dbg_ctrl.data_len <= 0 )
            {
                //When all data transfered get next command
                plc_app->dbg_resume();
                plc_dbg_ctrl.state = GET_CMD;
            }
            else
            {
                plc_dbg_ctrl.state = GET_VAR_IDX;
                plc_dbg_ctrl.tmp_len = 4;
                plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.set_val.var_idx;
            }
        }
    }
    break;
    //==========================================================================================
    case PUT_ID_LEN:
        //==========================================================================================
    case PUT_LC_LEN:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
        }
        else
        {
            //When length transfered transfer data
            plc_dbg_ctrl.state = PUT_DATA;
        }
    }
    break;
    //==========================================================================================
    case GET_MSG_LEVEL:
    {
        if( dbg_serial_read( &plc_dbg_ctrl.tr.get_log_msg.level, 1) )
        {
            PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

            plc_dbg_ctrl.state = GET_MSG_ID;
            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.get_log_msg.msg_id;
        }
    }
    break;

    case GET_MSG_ID:
    {
        if( plc_dbg_ctrl.tmp_len )
        {
            int read_res;
            //Must read before timeout
            read_res = dbg_serial_read( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp         += read_res;
            plc_dbg_ctrl.tmp_len      -= read_res;
            plc_dbg_ctrl.data_len     -= read_res;
        }
        else
        {
            PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

            plc_dbg_ctrl.state = PUT_MSG_LEN;

            plc_dbg_ctrl.data_len = plc_app->log_msg_get(
                                        plc_dbg_ctrl.tr.get_log_msg.level,
                                        plc_dbg_ctrl.tr.get_log_msg.msg_id,
                                        &plc_dbg_ctrl.tr_buf.log_msg[0],
                                        256,
                                        &plc_dbg_ctrl.tr.get_log_msg.tick,
                                        &plc_dbg_ctrl.tr.get_log_msg.sec,
                                        &plc_dbg_ctrl.tr.get_log_msg.nsec
                                    );

            plc_dbg_ctrl.data_len += 12; // Tick+Sec+NSec+Data
            plc_dbg_ctrl.data = (unsigned char *)&plc_dbg_ctrl.tr_buf.log_msg[0];

            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.data_len;
        }
    }
    break;

    case PUT_MSG_LEN:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
        }
        else
        {
            //When length transfered transfer data
            plc_dbg_ctrl.state = PUT_MSG_TICK;

            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.get_log_msg.tick;
        }
    }
    break;

    case PUT_MSG_TICK:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
            plc_dbg_ctrl.data_len -= write_res;
        }
        else
        {
            //When length transfered transfer data
            plc_dbg_ctrl.state = PUT_MSG_SEC;

            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.get_log_msg.sec;
        }
    }
    break;

    case PUT_MSG_SEC:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
            plc_dbg_ctrl.data_len -= write_res;
        }
        else
        {
            //When length transfered transfer data
            plc_dbg_ctrl.state = PUT_MSG_NSEC;

            plc_dbg_ctrl.tmp_len = 4;
            plc_dbg_ctrl.tmp = (unsigned char *)&plc_dbg_ctrl.tr.get_log_msg.nsec;
        }
    }
    break;

    case PUT_MSG_NSEC:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.tmp_len )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.tmp, plc_dbg_ctrl.tmp_len );
            plc_dbg_ctrl.tmp += write_res;
            plc_dbg_ctrl.tmp_len -= (unsigned char)write_res;
            plc_dbg_ctrl.data_len -= write_res;
        }
        else
        {
            //When length transfered transfer data
            plc_dbg_ctrl.state = PUT_DATA;
        }
    }
    break;
    //==========================================================================================
    case PUT_DATA:
    {
        PLC_CLEAR_TIMER( plc_dbg_ctrl.timer );

        if( plc_dbg_ctrl.data_len > 0 )
        {
            int write_res;
            write_res = dbg_serial_write( plc_dbg_ctrl.data, plc_dbg_ctrl.data_len );
            plc_dbg_ctrl.data += write_res;
            plc_dbg_ctrl.data_len -= write_res;
        }
        else
        {
            //When all data transfered get next command
            if( plc_dbg_ctrl.data_hook )
            {
                plc_dbg_ctrl.data_hook();
                plc_dbg_ctrl.data_hook = (void(*)(void))0;
            }
            plc_dbg_ctrl.state = GET_CMD;
        }
    }
    break;
    }
}
