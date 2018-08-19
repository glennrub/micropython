/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2016 - 2018 Glenn Ruben Bakke
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
#include "py/obj.h"

#if NRFX_SPI_ENABLED
#include "nrfx_spi.h"
#else
#include "nrfx_spim.h"
#endif

#if NRFX_SPIM_ENABLED

#define nrfx_spi_t                  nrfx_spim_t
#define nrfx_spi_config_t           nrfx_spim_config_t
#define nrfx_spi_xfer_desc_t        nrfx_spim_xfer_desc_t

#define NRFX_SPI_PIN_NOT_USED       NRFX_SPIM_PIN_NOT_USED
#define NRFX_SPI_INSTANCE           NRFX_SPIM_INSTANCE
#define NRF_SPI_BIT_ORDER_LSB_FIRST NRF_SPIM_BIT_ORDER_LSB_FIRST
#define NRF_SPI_BIT_ORDER_MSB_FIRST NRF_SPIM_BIT_ORDER_MSB_FIRST
#define NRF_SPI_MODE_0              NRF_SPIM_MODE_0
#define NRF_SPI_MODE_1              NRF_SPIM_MODE_1
#define NRF_SPI_MODE_2              NRF_SPIM_MODE_2
#define NRF_SPI_MODE_3              NRF_SPIM_MODE_3
#define NRF_SPI_FREQ_125K           NRF_SPIM_FREQ_125K
#define NRF_SPI_FREQ_250K           NRF_SPIM_FREQ_250K
#define NRF_SPI_FREQ_500K           NRF_SPIM_FREQ_500K
#define NRF_SPI_FREQ_1M             NRF_SPIM_FREQ_1M
#define NRF_SPI_FREQ_2M             NRF_SPIM_FREQ_2M
#define NRF_SPI_FREQ_4M             NRF_SPIM_FREQ_4M
#define NRF_SPI_FREQ_8M             NRF_SPIM_FREQ_8M

#define nrfx_spi_init               nrfx_spim_init
#define nrfx_spi_uninit             nrfx_spim_uninit
#define nrfx_spi_xfer               nrfx_spim_xfer

#endif // NRFX_SPIM_ENABLED

typedef struct _machine_hard_spi_obj_t machine_hard_spi_obj_t;
extern const mp_obj_type_t machine_hard_spi_type;

typedef struct _machine_hard_spi_obj_t {
    mp_obj_base_t       base;
    const nrfx_spi_t   * p_spi;    // Driver instance
    nrfx_spi_config_t  * p_config; // pointer to volatile part
} machine_hard_spi_obj_t;

void spi_init0(void);
void spi_transfer(const machine_hard_spi_obj_t * self,
                  size_t                         len,
                  const void *                   src,
                  void *                         dest);
