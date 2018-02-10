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

#ifndef NRFX_GLUE_H
#define NRFX_GLUE_H

#include <soc/nrfx_irqs.h>

#define NRFX_ASSERT(expression)

#if BLUETOOTH_SD

#if NRF51
#include "nrf_soc.h"
#else
#include "nrf_nvic.h"
#endif

#define NRFX_IRQ_ENABLE(irq_number) sd_nvic_EnableIRQ(irq_number)
#define NRFX_IRQ_DISABLE(irq_number) sd_nvic_DisableIRQ(irq_number)
#define NRFX_IRQ_PRIORITY_SET(irq_number, priority) sd_nvic_SetPriority(irq_number, priority)
#define NRFX_IRQ_PENDING_SET(irq_number) sd_nvic_SetPendingIRQ(irq_number)
#define NRFX_IRQ_PENDING_CLEAR(irq_number) sd_nvic_ClearPendingIRQ(irq_number)

#define NRFX_CRITICAL_SECTION_ENTER() \
    { \
        uint8_t _is_nested_critical_region; \
        sd_nvic_critical_region_enter(&_is_nested_critical_region);

#define NRFX_CRITICAL_SECTION_EXIT() \
        sd_nvic_critical_region_exit(_is_nested_critical_region); \
    }

#else // BLUETOOTH_SD

#define NRFX_IRQ_ENABLE(irq_number) NVIC_EnableIRQ(irq_number)
#define NRFX_IRQ_DISABLE(irq_number) NVIC_DisableIRQ(irq_number)
#define NRFX_IRQ_PRIORITY_SET(irq_number, priority) NVIC_SetPriority(irq_number, priority)
#define NRFX_IRQ_PENDING_SET(irq_number) NVIC_SetPendingIRQ(irq_number)
#define NRFX_IRQ_PENDING_CLEAR(irq_number) NVIC_ClearPendingIRQ(irq_number)

// Source:
// https://devzone.nordicsemi.com/f/nordic-q-a/8572/disable-interrupts-and-enable-interrupts-if-they-where-enabled/31347#31347
#define NRFX_CRITICAL_SECTION_ENTER() { int _old_primask = __get_PRIMASK(); __disable_irq();
#define NRFX_CRITICAL_SECTION_EXIT() __set_PRIMASK(_old_primask); }

#endif // !BLUETOOTH_SD

#endif // NRFX_GLUE_H
