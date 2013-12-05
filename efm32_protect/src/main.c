#include <iot-os.h>
#include "mpu.h"
#include "svc.h"
#include "debug.h"

void default_handler(void)
{
//    dprintf("IPSR=0x%03X\r\n",__get_IPSR());
}

static inline void hardware_init(void)
{
    /* Enable clocks for peripherals */
    CMU->HFPERCLKDIV = CMU_HFPERCLKDIV_HFPERCLKEN;
    CMU->HFPERCLKEN0 = CMU_HFPERCLKEN0_GPIO;

    /* Set calibration for 28MHz crystal */
    CMU->HFRCOCTRL = CMU_HFRCOCTRL_BAND_28MHZ |
        ((DEVINFO->HFRCOCAL1 & _DEVINFO_HFRCOCAL1_BAND28_MASK)
        >> _DEVINFO_AUXHFRCOCAL1_BAND28_SHIFT);
    while(!(CMU->STATUS & CMU_STATUS_HFRCORDY));

    /* Enable output */
    DEBUG_init();

    /* register SVC call interface */
    svc_init();

    /* setup MPU */
    mpu_init();
}

void main(void)
{
    uint32_t i;

    /* initialize hardware */
    hardware_init();

    /* configure MPU */
    mpu_set(7, (void*)RAM_MEM_BASE, SRAM_SIZE, 3<<MPU_RASR_AP_Pos);
    mpu_set(6, (void*)FLASH_MEM_BASE, FLASH_SIZE, 2<<MPU_RASR_AP_Pos);
    mpu_set(5, DEBUG_USART, 1024, 3<<MPU_RASR_AP_Pos);
    MPU->CTRL = MPU_CTRL_ENABLE_Msk|MPU_CTRL_PRIVDEFENA_Msk;
    mpu_debug();

    /* switch to unprivileged */
    __set_CONTROL(__get_CONTROL()|1);
    __ISB();
    __DSB();

    SVC(2);
    SVC(1);

    i=0;
    while(true)
        dprintf("Hello World 12345 %06d\r\n",i++);
}
