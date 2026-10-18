#ifndef HOST_SFO_V8_H
#define HOST_SFO_V8_H
#define SFO_INCOMPLETE 0
#define SFO_COMPLETE 1
#define SFO_ERROR 2
static unsigned sfo_calls, sfo_pending = 2;
static int sfo_result = SFO_COMPLETE;
static inline int SFO(void)
{
    ++sfo_calls;
    if (sfo_pending) { --sfo_pending; return SFO_INCOMPLETE; }
    return sfo_result;
}
#endif
