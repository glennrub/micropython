/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Glenn Ruben Bakke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

typedef void (*func)(void);

extern void  _start(void) __attribute__((noreturn));
extern void SystemInit(void);

void Default_Handler(void) {
    while (1);
}

void Reset_Handler(void) {
    uint32_t * p_src  = &_sidata;
    uint32_t * p_dest = &_sdata;

    while (p_dest < &_edata) {
      *p_dest++ = *p_src++;
    }

    uint32_t * p_bss     = &_sbss;
    uint32_t * p_bss_end = &_ebss;
    while (p_bss < p_bss_end) {
        *p_bss++ = 0ul;
    }

    SystemInit();
    _start();
}

void NMI_Handler                 (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void MemoryManagement_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler          (void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler                 (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler              (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler             (void) __attribute__ ((weak, alias("Default_Handler")));

void SPU_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void CLOCK_POWER_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void UARTE0_SPIM0_SPIS0_TWIM0_TWIS0_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void UARTE1_SPIM1_SPIS1_TWIM1_TWIS1_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void UARTE2_SPIM2_SPIS2_TWIM2_TWIS2_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void UARTE3_SPIM3_SPIS3_TWIM3_TWIS3_IRQHandler (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOTE0_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void SAADC_IRQHandler            (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_IRQHandler           (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_IRQHandler           (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER2_IRQHandler           (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void RTC1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void WDT_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void EGU0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void EGU1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void EGU2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void EGU3_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void EGU4_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void EGU5_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM0_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM1_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM2_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void PWM3_IRQHandler             (void) __attribute__ ((weak, alias("Default_Handler")));
void PDM_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void IPC_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void FPU_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIOTE1_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));
void KMU_IRQHandler              (void) __attribute__ ((weak, alias("Default_Handler")));
void CRYPTOCELL_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));

const func __Vectors[] __attribute__ ((section(".isr_vector"),used)) = {
    (func)&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemoryManagement_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    SecureFault_Handler,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,

    /* External Interrupts */
    0,
    0,
    0,
    SPU_IRQHandler,
    0,
    CLOCK_POWER_IRQHandler,
    0,
    0,
    UARTE0_SPIM0_SPIS0_TWIM0_TWIS0_IRQHandler,
    UARTE1_SPIM1_SPIS1_TWIM1_TWIS1_IRQHandler,
    UARTE2_SPIM2_SPIS2_TWIM2_TWIS2_IRQHandler,
    UARTE3_SPIM3_SPIS3_TWIM3_TWIS3_IRQHandler,
    0,
    GPIOTE0_IRQHandler,
    SAADC_IRQHandler,
    TIMER0_IRQHandler,
    TIMER1_IRQHandler,
    TIMER2_IRQHandler,
    0,
    0,
    RTC0_IRQHandler,
    RTC1_IRQHandler,
    0,
    0,
    WDT_IRQHandler,
    0,
    0,
    EGU0_IRQHandler,
    EGU1_IRQHandler,
    EGU2_IRQHandler,
    EGU3_IRQHandler,
    EGU4_IRQHandler,
    PWM0_IRQHandler,
    PWM1_IRQHandler,
    PWM2_IRQHandler,
    PWM3_IRQHandler,
    0,
    PDM_IRQHandler,
    0,
    I2S_IRQHandler,
    0,
    IPC_IRQHandler,
    0,
    FPU_IRQHandler,
    0,
    0,
    0,
    0,
    GPIOTE1_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KMU_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    CRYPTOCELL_IRQHandler,
};
