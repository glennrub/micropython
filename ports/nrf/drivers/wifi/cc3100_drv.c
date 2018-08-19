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

#include "py/mpconfig.h"

#if (MICROPY_PY_NETWORK && MICROPY_PY_CC31K)
#include "cc3100_drv.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "pin.h"
#include "spi.h"
#include "mphalport.h"

#if NRFX_SPI_ENABLED
#include "nrfx_spi.h"
#else
#include "nrfx_spim.h"
#endif

#define CS_LOW()            nrf_gpio_pin_clear(mp_pin_cs->pin)
#define CS_HIGH()           nrf_gpio_pin_set(mp_pin_cs->pin)

STATIC const machine_hard_spi_obj_t * mp_spi = NULL;

STATIC const pin_obj_t * mp_pin_cs  = NULL;
STATIC const pin_obj_t * mp_pin_en  = NULL;
STATIC const pin_obj_t * mp_pin_irq = NULL;

STATIC volatile irq_handler_t mp_cc3100_irq_handler = NULL;

STATIC const mp_obj_fun_builtin_fixed_t irq_callback_obj;

STATIC mp_obj_t irq_callback(mp_obj_t pin_obj) {
    if (mp_cc3100_irq_handler)
    {
        mp_cc3100_irq_handler(0);
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(irq_callback_obj, irq_callback);

void cc3100_drv_enable(void) {
    nrf_gpio_pin_set(mp_pin_en->pin);
}

void cc3100_drv_disable(void) {
    nrf_gpio_pin_clear(mp_pin_en->pin);
}

void cc3100_drv_init(void *spi, const void *pin_cs, const void *pin_en, const void *pin_irq) {
    machine_hard_spi_obj_t * spi_obj = spi;

    mp_spi     = spi_obj;
    mp_pin_cs  = pin_cs;
    mp_pin_en  = pin_en;
    mp_pin_irq = pin_irq;
}

int cc3100_drv_open(char * interface, unsigned long flags) {
    nrf_gpio_cfg_output(mp_pin_cs->pin);
    nrf_gpio_cfg_output(mp_pin_en->pin);

    nrf_gpio_pin_set(mp_pin_cs->pin);
    nrf_gpio_cfg_input(mp_pin_irq->pin, NRF_GPIO_PIN_PULLUP);

    // register interrupt handler
    extint_register(mp_pin_irq->pin, NRF_GPIOTE_POLARITY_LOTOHI, NRF_GPIO_PIN_PULLDOWN, (mp_obj_t)&irq_callback_obj);
    extint_enable(mp_pin_irq->pin);

    nrf_gpio_pin_clear(mp_pin_en->pin);
    mp_hal_delay_ms(500);

    return 0;
}

int cc3100_drv_close(int handle) {
    return -1;
}

int cc3100_drv_read(int handle, unsigned char * p_buffer, int len) {
    nrfx_spi_xfer_desc_t xfer_desc = {
        .p_tx_buffer = NULL,
	    .tx_length   = 0,
	    .p_rx_buffer = p_buffer,
	    .rx_length   = len
    };

    CS_LOW();
    (void)nrfx_spi_xfer(mp_spi->p_spi, &xfer_desc, 0);
    CS_HIGH();

    return 0;
}

int cc3100_drv_write(int handle, unsigned char * p_buffer, int len) {
    nrfx_spi_xfer_desc_t xfer_desc = {
        .p_tx_buffer = p_buffer,
        .tx_length   = len,
        .p_rx_buffer = NULL,
        .rx_length   = 0
    };

    CS_LOW();
    (void)nrfx_spi_xfer(mp_spi->p_spi, &xfer_desc, 0);
    CS_HIGH();

    return 0;
}

int cc3100_drv_irq_handler_register(irq_handler_t handler, void * p_val) {
    mp_cc3100_irq_handler = handler;

    return 0;
}

void cc3100_drv_irq_handler_mask(void) {
}

void cc3100_drv_irq_handler_unmask(void) {
}

#endif // (MICROPY_PY_NETWORK && MICROPY_PY_CC31K)
