#include <plc_app_default.h>
#include <plc_tick.h>

//static unsigned long __tick;

static int startPLC(int argc,char **argv)
{
    (void)argc;
    (void)argv;
    plc_tick_setup( 0, 1000000ull );
    return 0;
}
static int stopPLC()
{
    return 0;
}
static void runPLC(void)
{
    return;
}

static void resumeDebug(void)
{
    return;
}
static void suspendDebug(int disable)
{
    return;
}

static void FreeDebugData(void)
{
    return;
}

const uint32_t default_len = 0;
static int GetDebugData(unsigned long *tick, unsigned long *size, void **buffer)
{
    *tick = 0;
    *size = 0;
    *buffer = (void *)&default_len;
    return 0;
}
static void ResetDebugVariables(void)
{
    return;
}
static void RegisterDebugVariable(int idx, void* force)
{
    (void)idx;
    (void)force;
    return;
}

static void ResetLogCount(void)
{
    return;
}
static uint32_t GetLogCount(uint8_t level)
{
    (void)level;
    return 0;
}
static uint32_t GetLogMessage(uint8_t level, uint32_t msgidx, char* buf, uint32_t max_size, uint32_t* tick, uint32_t* tv_sec, uint32_t* tv_nsec)
{
    (void)level;
    (void)msgidx;
    (void)buf;
    (void)max_size;
    (void)tick;
    (void)tv_sec;
    (void)tv_nsec;
    return 0;
}

const plc_app_abi_t plc_app_default =
{
    .id   = "DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF",

    .start = startPLC,
    .stop  = stopPLC,
    .run   = runPLC,

    .dbg_resume    = resumeDebug,
    .dbg_suspend   = suspendDebug,

    .dbg_data_get  = GetDebugData,
    .dbg_data_free = FreeDebugData,

    .dbg_vars_reset   = ResetDebugVariables,
    .dbg_var_register = RegisterDebugVariable,

    .log_cnt_get   = GetLogCount,
    .log_msg_get   = GetLogMessage,
    .log_cnt_reset = ResetLogCount
};
