#ifndef HOST_DRIVERLIB_H
#define HOST_DRIVERLIB_H
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
// 只模拟寄存器接口，不模拟模拟电路、死区和硬件响应时间。
#define __interrupt
#define asm(...) // 主机忽略 DCLCLA 的 CLA 舍入模式指令；算法保持原式。
#define EPWM1_BASE 0U
#define EPWM2_BASE 1U
#define EPWM_COUNTER_COMPARE_A 0
#define EPWM_COUNTER_COMPARE_B 1
#define EPWM_TZ_FORCE_EVENT_OST 4
#define EPWM_TZ_FLAG_OST 4
#define ADC_INT_NUMBER1 0
#define INTERRUPT_ACK_GROUP1 1
#define INTERRUPT_ACK_GROUP11 11
#define INTERRUPT_ACK_GROUP7 7
#define INT_DMA_CH1 7
#define CLA1_BASE 4U
#define CLA_TASK_1 0
#define CLA_TASKFLAG_8 128U
#define CLA_TASKFLAG_1 1U
static bool cla_init_timeout, cla_task_overflow;
static bool cla_task_running, cla_task_pending, cla_task_enabled = true;
static bool cla_program_memory, cla_program_loaded, cla_copy_blocked;
#define MEMCFG_SECT_LS0 0U
#define MEMCFG_SECT_LS1 1U
#define MEMCFG_LSRAMCONTROLLER_CPU_ONLY 0U
void Cla1Task8(void);
void host_memcfg_init(void);
#define MEMCFG_init() host_memcfg_init()
static inline void CLA_performHardReset(uint32_t b) {
    assert(b == CLA1_BASE);
    cla_task_overflow = false;
    cla_task_pending = cla_task_running = false;
    // CLA 复位不改变 MemCfg 的程序 RAM 所有权。
}
static inline void MemCfg_setLSRAMControllerSel(uint32_t section, uint16_t owner) {
    assert((section == MEMCFG_SECT_LS0 || section == MEMCFG_SECT_LS1) && owner == MEMCFG_LSRAMCONTROLLER_CPU_ONLY);
    cla_program_memory = false;
}
static inline void CLA_init(void) {
    // 模拟 SysConfig 的 Flash -> LS0 复制，以及硬件的 CLA 程序 RAM 访问限制。
    cla_copy_blocked = cla_program_memory;
    cla_program_loaded = !cla_copy_blocked;
}
static inline void CLA_forceTasks(uint32_t b, uint16_t f) {
    assert(b == CLA1_BASE);
    if (f == CLA_TASKFLAG_8) {
        if (!cla_init_timeout && cla_program_loaded && cla_program_memory) Cla1Task8();
    } else {
        assert(f == CLA_TASKFLAG_1);
        if (cla_task_running || cla_task_pending) cla_task_overflow = true;
        else cla_task_pending = true;
    }
}
static inline bool CLA_getTaskOverflowFlag(uint32_t b, int n) { assert(b == CLA1_BASE && n == CLA_TASK_1); return cla_task_overflow; }
static inline void CLA_disableTasks(uint32_t b, uint16_t f) { assert(b == CLA1_BASE && f == CLA_TASKFLAG_1); cla_task_enabled = false; }
static inline void CLA_enableTasks(uint32_t b, uint16_t f) { assert(b == CLA1_BASE && f == CLA_TASKFLAG_1); cla_task_enabled = true; }
static inline bool CLA_getTaskRunStatus(uint32_t b, int n) { assert(b == CLA1_BASE && n == CLA_TASK_1); return cla_task_running; }
#define EPWM_TZ_ACTION_EVENT_TZA 0
#define EPWM_TZ_ACTION_EVENT_TZB 1
#define EPWM_TZ_ACTION_EVENT_DCAEVT1 2
#define EPWM_TZ_ACTION_EVENT_DCAEVT2 3
#define EPWM_TZ_ACTION_EVENT_DCBEVT1 4
#define EPWM_TZ_ACTION_EVENT_DCBEVT2 5
#define EPWM_TZ_ACTION_LOW 2
#define EPWM_TZ_ACTION_DISABLE 3
#define EPWM_AQ_OUTPUT_A 0
#define EPWM_AQ_OUTPUT_LOW 1
#define EPWM_AQ_OUTPUT_HIGH 2
#define EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO 0
#define EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA 4
#define EPWM_COUNTER_MODE_UP 0
#define EPWM_PERIOD_DIRECT_LOAD 1
#define EPWM_COMP_LOAD_ON_CNTR_ZERO 0
#define HRPWM_COUNTER_COMPARE_A 0
#define HRPWM_COUNTER_COMPARE_B 1
#define HRPWM_CHANNEL_A 0
#define HRPWM_CHANNEL_B 1
#define HRPWM_MEP_CTRL_RISING_EDGE 1
#define HRPWM_MEP_CTRL_FALLING_EDGE 2
#define HRPWM_MEP_DUTY_PERIOD_CTRL 0
#define HRPWM_LOAD_ON_CNTR_ZERO 0
#define HRPWM_OUTPUT_ON_B_NORMAL 0
#define EPWM_DB_COUNTER_CLOCK_FULL_CYCLE 0
#define EPWM_SOC_A 0
#define EPWM_SOC_B 1
#define INT_ADCC1 1
#define SYSCTL_PERIPH_CLK_TBCLKSYNC 1
static uint16_t cmp[4], cmpB[4], period[4], phase[4], counter[4], prescale[2];
static uint16_t trip[2], adc[3][4], signals[2], actions[2][6], aq[2][8];
static bool overflow;
static bool period_direct[4];
static uint16_t period_active[4];
static uint32_t cmp_q8[2][2];
static int hr_edge[2][2], hr_load[2][2], coarse_load[4][2];
static bool auto_conversion[2];
static uint16_t db_red[2], db_fed[2];
static inline void HRPWM_setCounterCompareValue(uint32_t b, int c, uint32_t v) { cmp_q8[b][c] = v; if (c == 0) cmp[b] = v >> 8; else cmpB[b] = v >> 8; }
static inline void HRPWM_setMEPEdgeSelect(uint32_t b, int c, int e) { hr_edge[b][c] = e; }
static inline void HRPWM_setCounterCompareShadowLoadEvent(uint32_t b, int c, int e) { hr_load[b][c] = e; }
static inline void EPWM_setCounterCompareShadowLoadMode(uint32_t b, int c, int e) { coarse_load[b][c] = e; }
static inline void HRPWM_setMEPControlMode(uint32_t b, int c, int m) { (void)b; (void)c; assert(m == HRPWM_MEP_DUTY_PERIOD_CTRL); }
static inline void HRPWM_enableAutoConversion(uint32_t b) { auto_conversion[b] = true; }
#define HRPWM_disablePeriodControl(b) ((void)0)
#define HRPWM_disablePhaseShiftLoad(b) ((void)0)
#define HRPWM_setChannelBOutputPath(b, p) ((void)0)
#define HRPWM_setOutputSwapMode(b, s) ((void)0)
static inline void EPWM_setDeadBandCounterClock(uint32_t b, int c) { (void)b; assert(c == EPWM_DB_COUNTER_CLOCK_FULL_CYCLE); }
static inline void EPWM_setRisingEdgeDelayCount(uint32_t b, uint16_t v) { db_red[b] = v; }
static inline void EPWM_setFallingEdgeDelayCount(uint32_t b, uint16_t v) { db_fed[b] = v; }
static inline void EPWM_setCounterCompareValue(uint32_t b, int c, uint16_t v) { if (c == EPWM_COUNTER_COMPARE_A) cmp[b] = v; else cmpB[b] = v; }
static inline void EPWM_forceTripZoneEvent(uint32_t b, uint16_t f) { trip[b] |= f; }
static inline uint16_t EPWM_getTripZoneFlagStatus(uint32_t b) { return trip[b]; }
static inline void EPWM_clearTripZoneFlag(uint32_t b, uint16_t f) { trip[b] &= ~f; }
static inline uint16_t ADC_readResult(uint32_t b, uint16_t ch) { return adc[(b - 0xB00) / 0x20][ch]; }
static inline bool ADC_getInterruptOverflowStatus(uint32_t b, int n) { (void)b; (void)n; return overflow; }
static inline void EPWM_disableTripZoneSignals(uint32_t b, uint16_t f) { signals[b] &= ~f; }
#define ADC_clearInterruptStatus(b, n) ((void)0)
#define ADC_clearInterruptOverflowStatus(b, n) (overflow = false)
#define Interrupt_clearACKGroup(g) ((void)0)
static inline void EPWM_setTripZoneAction(uint32_t b, int e, int a) { actions[b][e] = a; }
static inline void EPWM_setActionQualifierAction(uint32_t b, int o, int a, int e) { if (b < 2 && o == EPWM_AQ_OUTPUT_A) aq[b][e] = a; }
#define EPWM_setTimeBaseCounterMode(b, m) ((void)0)
static inline void EPWM_setPeriodLoadMode(uint32_t b, int m) { period_direct[b] = (m == EPWM_PERIOD_DIRECT_LOAD); }
static inline void EPWM_setTimeBasePeriod(uint32_t b, uint16_t v) { period[b] = v; if (period_direct[b]) period_active[b] = v; }
static inline void EPWM_setPhaseShift(uint32_t b, uint16_t v) { phase[b] = v; }
static inline void EPWM_setTimeBaseCounter(uint32_t b, uint16_t v) { counter[b] = v; }
static inline void EPWM_setADCTriggerEventPrescale(uint32_t b, int s, uint16_t n) { assert(b == 2); prescale[s] = n; }
#define ADC_setInterruptSource(b, n, s) ((void)0)
#define ADC_disableContinuousMode(b, n) ((void)0)
#define ADC_enableInterrupt(b, n) ((void)0)
#define Interrupt_register(n, f) ((void)0)
#define Interrupt_enable(n) ((void)0)
#define GPIO_setPinConfig(p) ((void)0)
#define SysCtl_enablePeripheral(p) ((void)0)

// DMA 状态替身，测试按实际生成的 burst/transfer 步长执行 ADC 结果搬运。
#define DMA_CH1_BASE 7U
#define DMA_TRIGGER_ADCC1 11U
#define DMA_CFG_ONESHOT_ENABLE 1U
#define DMA_CFG_CONTINUOUS_DISABLE 0U
#define DMA_CFG_SIZE_16BIT 0U
#define DMA_INT_AT_END 1U
#define DMA_EMULATION_STOP 0U
static struct {
    uintptr_t source;
    uint16_t *destination;
    uint16_t burst, transfers;
    int16_t source_burst_step, dest_burst_step, source_transfer_step, dest_transfer_step;
    uint32_t mode, trigger;
    bool running, active, overflow, triggered, interrupt_enabled, overrun_interrupt;
} dma;
static inline void DMA_initController(void) { dma.running = dma.active = dma.overflow = false; }
static inline void DMA_setEmulationMode(uint16_t mode) { assert(mode == DMA_EMULATION_STOP); }
static inline void DMA_configAddresses(uint32_t b, const void *dest, const void *src) {
    assert(b == DMA_CH1_BASE); dma.destination = (uint16_t *)dest; dma.source = (uintptr_t)src;
}
static inline void DMA_configBurst(uint32_t b, uint16_t size, int16_t src, int16_t dest) {
    assert(b == DMA_CH1_BASE); dma.burst = size; dma.source_burst_step = src; dma.dest_burst_step = dest;
}
static inline void DMA_configTransfer(uint32_t b, uint16_t size, int16_t src, int16_t dest) {
    assert(b == DMA_CH1_BASE); dma.transfers = size; dma.source_transfer_step = src; dma.dest_transfer_step = dest;
}
static inline void DMA_configWrap(uint32_t b, uint16_t srcSize, int16_t srcStep, uint16_t dstSize, int16_t dstStep) {
    assert(b == DMA_CH1_BASE && srcSize == 65535 && dstSize == 65535 && srcStep == 0 && dstStep == 0);
}
static inline void DMA_configMode(uint32_t b, uint32_t trigger, uint32_t mode) {
    assert(b == DMA_CH1_BASE); dma.trigger = trigger; dma.mode = mode;
}
static inline void DMA_setInterruptMode(uint32_t b, uint16_t mode) { assert(b == DMA_CH1_BASE && mode == DMA_INT_AT_END); }
static inline void DMA_enableInterrupt(uint32_t b) { assert(b == DMA_CH1_BASE); dma.interrupt_enabled = true; }
static inline void DMA_enableOverrunInterrupt(uint32_t b) { assert(b == DMA_CH1_BASE); dma.overrun_interrupt = true; }
static inline void DMA_enableTrigger(uint32_t b) { assert(b == DMA_CH1_BASE); dma.triggered = true; }
static inline void DMA_disableTrigger(uint32_t b) { assert(b == DMA_CH1_BASE); dma.triggered = false; }
static inline void DMA_startChannel(uint32_t b) { assert(b == DMA_CH1_BASE); dma.running = true; }
static inline void DMA_stopChannel(uint32_t b) { assert(b == DMA_CH1_BASE); dma.running = false; }
static inline void DMA_clearErrorFlag(uint32_t b) { assert(b == DMA_CH1_BASE); dma.overflow = false; }
static inline void DMA_clearTriggerFlag(uint32_t b) { assert(b == DMA_CH1_BASE); }
static inline bool DMA_getOverflowFlag(uint32_t b) { assert(b == DMA_CH1_BASE); return dma.overflow; }
static inline bool DMA_getTransferStatusFlag(uint32_t b) { assert(b == DMA_CH1_BASE); return dma.active; }
static inline bool DMA_getRunStatusFlag(uint32_t b) { assert(b == DMA_CH1_BASE); return dma.running; }

static inline bool dma_capture_hardware(void)
{
    if (!dma.running || !dma.triggered) return false;
    assert(dma.burst == 4 && dma.transfers == 3 && dma.mode == DMA_CFG_ONESHOT_ENABLE);
    assert(dma.trigger == DMA_TRIGGER_ADCC1 && dma.interrupt_enabled && dma.overrun_interrupt);
    intptr_t source = dma.source, dest = 0;
    dma.active = true;
    for (unsigned burst = 0; burst < dma.transfers; ++burst) {
        for (unsigned word = 0; word < dma.burst; ++word) {
            unsigned module = (source - 0xB00) / 0x20, result = (source - 0xB00) % 0x20;
            assert(module < 3 && result < 4 && dest >= 0 && dest < 12);
            dma.destination[dest] = adc[module][result];
            if (word + 1 < dma.burst) {
                source += dma.source_burst_step; dest += dma.dest_burst_step;
            }
        }
        if (burst + 1 < dma.transfers) {
            source += dma.source_transfer_step; dest += dma.dest_transfer_step;
        }
    }
    dma.active = dma.running = false; // CONTINUOUS_DISABLE：提交前保持结果不变。
    return true;
}

static uint32_t background_period;
static bool background_timer_running, background_overflow;
#define CPUTimer_setEmulationMode(...) ((void)0)
static inline void CPUTimer_setPreScaler(uint32_t b, uint16_t p) { assert(b == 6 && p == 0); }
static inline void CPUTimer_setPeriod(uint32_t b, uint32_t p) { assert(b == 6); background_period = p; }
#define CPUTimer_disableInterrupt(...) ((void)0)
static inline void CPUTimer_stopTimer(uint32_t b) { assert(b == 6); background_timer_running = false; }
#define CPUTimer_reloadTimerCounter(...) ((void)0)
static inline void CPUTimer_startTimer(uint32_t b) { assert(b == 6); background_timer_running = true; }
static inline bool CPUTimer_getTimerOverflowStatus(uint32_t b) { assert(b == 6); return background_overflow; }
static inline void CPUTimer_clearOverflowFlag(uint32_t b) { assert(b == 6); background_overflow = false; }

#define EPWM_AQ_SW_DISABLED 0
#define EPWM_AQ_SW_OUTPUT_LOW 1
static bool buzzer_interrupt_enabled;
static int buzzer_force;
static uint16_t buzzer_irq_div;
static inline void EPWM_enableInterrupt(uint32_t b) { assert(b == 3); buzzer_interrupt_enabled = true; }
static inline void EPWM_disableInterrupt(uint32_t b) { assert(b == 3); buzzer_interrupt_enabled = false; }
static inline void EPWM_setInterruptEventCount(uint32_t b, uint16_t n) { assert(b == 3); buzzer_irq_div = n; }
static inline void EPWM_setActionQualifierContSWForceAction(uint32_t b, int o, int a) { assert(b == 3 && o == EPWM_AQ_OUTPUT_A); buzzer_force = a; }
#define EPWM_setActionQualifierContSWForceShadowMode(...) ((void)0)
#define EPWM_setInterruptSource(...) ((void)0)
#define EPWM_clearEventTriggerInterruptFlag(...) ((void)0)
#define GPIO_writePin(...) ((void)0)
#define GPIO_setDirectionMode(...) ((void)0)
#define GPIO_setPadConfig(...) ((void)0)
#define GPIO_setQualificationMode(...) ((void)0)

#define SCI_FIFO_TX16 16U
#define SCI_FIFO_TX0 0U
#define SCI_FIFO_RX0 0U
#define SCI_RXSTATUS_ERROR 0x80U
static uint16_t sci_bytes[16384], sci_byte_count, sci_fifo_used;
static uint16_t sci_rx_bytes[16], sci_rx_count, sci_rx_index, sci_rx_status;
static bool sci_rx_overflow;
static bool sci_shift_busy, sci_rx_fault_persistent;
static uint16_t sci_software_resets;
static bool sci_initialized;
static inline uint16_t SCI_getRxFIFOStatus(uint32_t b) { assert(b == 4); return sci_rx_count - sci_rx_index; }
static inline uint16_t SCI_readCharNonBlocking(uint32_t b) { assert(b == 4 && sci_rx_index < sci_rx_count); return sci_rx_bytes[sci_rx_index++]; }
static inline uint16_t SCI_getRxStatus(uint32_t b) { assert(b == 4); return sci_rx_status; }
static inline bool SCI_getOverflowStatus(uint32_t b) { assert(b == 4); return sci_rx_overflow; }
static inline void SCI_clearOverflowStatus(uint32_t b) { assert(b == 4); sci_rx_overflow = false; }
static inline void SCI_resetRxFIFO(uint32_t b) { assert(b == 4); sci_rx_count = sci_rx_index = 0; }
static inline void SCI_resetTxFIFO(uint32_t b) { assert(b == 4); sci_fifo_used = 0; }
static inline bool SCI_isTransmitterBusy(uint32_t b) { assert(b == 4); return sci_shift_busy || sci_fifo_used != 0; }
static inline void SCI_performSoftwareReset(uint32_t b) {
    assert(b == 4 && sci_fifo_used == 0 && !sci_shift_busy);
    ++sci_software_resets;
    sci_rx_status = sci_rx_fault_persistent ? SCI_RXSTATUS_ERROR : 0;
}
static inline uint16_t SCI_getTxFIFOStatus(uint32_t b) { assert(b == 4); return sci_fifo_used; }
static inline void SCI_writeCharNonBlocking(uint32_t b, uint16_t value) {
    assert(b == 4 && value <= 255 && sci_fifo_used < 16 && sci_byte_count < 16384);
    sci_bytes[sci_byte_count++] = value;
    ++sci_fifo_used;
}

// SysConfig 生成的初始化直接在寄存器替身上执行，避免另写一份配置。
#define EPWM_AQ_OUTPUT_B 1
#define EPWM_AQ_OUTPUT_NO_CHANGE 0
#define EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD 1
#define EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA 5
#define EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB 6
#define EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB 7
#define EPWM_setClockPrescaler(...) ((void)0)
#define EPWM_setSyncOutPulseMode(...) ((void)0)
#define EPWM_disablePhaseShiftLoad(...) ((void)0)
#define EPWM_enablePhaseShiftLoad(...) ((void)0)
#define EPWM_disableActionQualifierShadowLoadMode(...) ((void)0)
#define EPWM_setDeadBandDelayPolarity(...) ((void)0)
#define EPWM_setDeadBandDelayMode(...) ((void)0)
#define EPWM_setRisingEdgeDelayCountShadowLoadMode(...) ((void)0)
#define EPWM_disableRisingEdgeDelayCountShadowLoadMode(...) ((void)0)
#define EPWM_setFallingEdgeDelayCountShadowLoadMode(...) ((void)0)
#define EPWM_disableFallingEdgeDelayCountShadowLoadMode(...) ((void)0)
#define EPWM_setDeadBandControlShadowLoadMode(...) ((void)0)
#define EPWM_disableDeadBandControlShadowLoadMode(...) ((void)0)
#define EPWM_enableADCTrigger(...) ((void)0)
#define EPWM_setADCTriggerSource(...) ((void)0)
#define HRPWM_setClockPrescaler EPWM_setClockPrescaler
#define HRPWM_setPeriodLoadMode EPWM_setPeriodLoadMode
#define HRPWM_setTimeBaseCounter EPWM_setTimeBaseCounter
#define HRPWM_setTimeBaseCounterMode EPWM_setTimeBaseCounterMode
#define HRPWM_setSyncOutPulseMode EPWM_setSyncOutPulseMode
#define HRPWM_setCounterCompareShadowLoadMode EPWM_setCounterCompareShadowLoadMode
#define HRPWM_disableActionQualifierShadowLoadMode EPWM_disableActionQualifierShadowLoadMode
#define HRPWM_setActionQualifierAction EPWM_setActionQualifierAction
#define HRPWM_setDeadBandDelayPolarity EPWM_setDeadBandDelayPolarity
#define HRPWM_setDeadBandDelayMode EPWM_setDeadBandDelayMode
#define HRPWM_setRisingEdgeDelayCountShadowLoadMode EPWM_setRisingEdgeDelayCountShadowLoadMode
#define HRPWM_disableRisingEdgeDelayCountShadowLoadMode EPWM_disableRisingEdgeDelayCountShadowLoadMode
#define HRPWM_setRisingEdgeDelayCount EPWM_setRisingEdgeDelayCount
#define HRPWM_setFallingEdgeDelayCountShadowLoadMode EPWM_setFallingEdgeDelayCountShadowLoadMode
#define HRPWM_disableFallingEdgeDelayCountShadowLoadMode EPWM_disableFallingEdgeDelayCountShadowLoadMode
#define HRPWM_setFallingEdgeDelayCount EPWM_setFallingEdgeDelayCount
#define HRPWM_setDeadBandControlShadowLoadMode EPWM_setDeadBandControlShadowLoadMode
#define HRPWM_disableDeadBandControlShadowLoadMode EPWM_disableDeadBandControlShadowLoadMode
#define HRPWM_setTripZoneAction EPWM_setTripZoneAction
#endif
