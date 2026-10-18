/* V2 仅使用 Flash 启动，控制中断复制到 RAM 执行。 */
MEMORY
{
PAGE 0:
    BEGIN    : origin = 0x080000, length = 0x000002
    FLASH    : origin = 0x080002, length = 0x01FFEE
    CLA_CODE : origin = 0x008000, length = 0x001000
    CLA_DATA : origin = 0x009000, length = 0x000800
    RAM_CODE : origin = 0x009800, length = 0x001000
    RESET    : origin = 0x3FFFC0, length = 0x000002
PAGE 1:
    RAMM1    : origin = 0x000400, length = 0x0003F8
    RAM_DATA : origin = 0x00A800, length = 0x000800
    CLA_MSG  : origin = 0x001480, length = 0x000080
    CPU_MSG  : origin = 0x001500, length = 0x000080
    DMA_DATA : origin = 0x00C000, length = 0x002000
}
SECTIONS
{
    codestart   : > BEGIN, PAGE = 0
    .text       : > FLASH, PAGE = 0, ALIGN(4)
    .cinit      : > FLASH, PAGE = 0, ALIGN(4)
    .switch     : > FLASH, PAGE = 0, ALIGN(4)
    .const      : > FLASH, PAGE = 0, ALIGN(4)
    .init_array : > FLASH, PAGE = 0, ALIGN(4)
    dclfuncs    : > FLASH, PAGE = 0, ALIGN(4)
    .reset      : > RESET, PAGE = 0, TYPE = DSECT
    .stack      : > RAMM1, PAGE = 1
    .bss        : > RAM_DATA, PAGE = 1
    .bss:output : > RAM_DATA, PAGE = 1
    .bss:cio    : > RAM_DATA, PAGE = 1
    .data       : > RAM_DATA, PAGE = 1
    .sysmem     : > RAM_DATA, PAGE = 1
    Cla1Prog    : LOAD = FLASH, RUN = CLA_CODE,
                  LOAD_START(Cla1ProgLoadStart), LOAD_SIZE(Cla1ProgLoadSize),
                  RUN_START(Cla1ProgRunStart), PAGE = 0, ALIGN(4)
    .const_cla  : LOAD = FLASH, RUN = CLA_DATA,
                  LOAD_START(Cla1ConstLoadStart), LOAD_SIZE(Cla1ConstLoadSize),
                  RUN_START(Cla1ConstRunStart), PAGE = 0, ALIGN(4)
    .scratchpad : > CLA_DATA, PAGE = 0
    .bss_cla    : > CLA_DATA, PAGE = 0
    ControlShared : > CLA_DATA, PAGE = 0, ALIGN(2)
    Cla1ToCpuMsgRAM : > CLA_MSG, PAGE = 1, TYPE = NOINIT
    CpuToCla1MsgRAM : > CPU_MSG, PAGE = 1, TYPE = NOINIT
    AdcDmaBuffer : > DMA_DATA, PAGE = 1, ALIGN(2), TYPE = NOINIT
    .TI.ramfunc : LOAD = FLASH, RUN = RAM_CODE,
                  LOAD_START(RamfuncsLoadStart),
                  LOAD_SIZE(RamfuncsLoadSize),
                  RUN_START(RamfuncsRunStart), PAGE = 0, ALIGN(4)
}
