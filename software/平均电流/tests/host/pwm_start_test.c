#include <assert.h>
#include <stdio.h>
#include "control_runtime.h"

// 仅模拟向上计数、周期影子加载和周期同步，不模拟模拟电路或栅极延迟。
static unsigned compare_events(uint16_t active, bool direct)
{
    uint16_t ctr = counter[1];
    unsigned events = 0;
    for (unsigned tick = 0; tick < PWM_COUNTS * 10U; ++tick) {
        if (tick % PWM_COUNTS == 2) ctr = phase[1];
        if (direct) assert(ctr == (tick + PWM_COUNTS / 2U) % PWM_COUNTS);
        if (ctr == 0 && !direct) active = period[1];
        if (ctr == cmp[1]) ++events;
        ctr = (ctr == active) ? 0 : (uint16_t)(ctr + 1U);
    }
    return events;
}

int main(void)
{
    Supercap_Init();
    run_duty(0.60f / 0.90f);
    // 复现旧顺序：有效周期为复位值或旧值，计数器被同步到更大的相位值。
    assert(compare_events(0, false) == 0);
    assert(compare_events(100, false) == 0);
    // 新初始化必须在启用时基前装好三路有效周期。
    for (unsigned b = 0; b < 3; ++b) assert(period_active[b] == 285);
    assert(compare_events(period_active[1], period_direct[1]) == 10);
    puts("通过：复现旧初始化丢失比较事件；修正后 B 桥每周期均命中比较点");
}
