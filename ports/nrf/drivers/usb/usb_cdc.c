/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2019-2020 Glenn Ruben Bakke
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
 *
 * This file is part of the TinyUSB stack.
 */

#include "py/mphal.h"

#if MICROPY_HW_USB_CDC

#include "tusb.h"
#include "nrfx.h"
#include "nrfx_power.h"
#include "nrfx_uart.h"
#include "py/ringbuf.h"

#ifdef BLUETOOTH_SD
#include "nrf_sdm.h"
#include "nrf_soc.h"
#include "ble_drv.h"
#endif

#if MICROPY_HW_USB_CDC_DUAL
#include "nrf_gpio.h"
#endif

extern void tusb_hal_nrf_power_event(uint32_t event);

static void cdc_task(void);

static uint8_t rx_ringbuf_array[1024];
static uint8_t tx_ringbuf_array[1024];
static volatile ringbuf_t rx_ringbuf;
static volatile ringbuf_t tx_ringbuf;

#if MICROPY_HW_USB_CDC_DUAL
static uint8_t uarte_rx_ringbuf_array[1024];
static const nrfx_uart_t uart_instance = NRFX_UART_INSTANCE(0);
static uint8_t uarte_rx_buf[1];

static volatile ringbuf_t uarte_rx_ringbuf;
static volatile ringbuf_t uarte_tx_ringbuf;

static void uart_event_handler(nrfx_uart_event_t const *p_event, void *p_context) {
    if (p_event->type == NRFX_UART_EVT_RX_DONE) {
        int chr = uarte_rx_buf[0];
        nrfx_uart_rx(&uart_instance, &uarte_rx_buf[0], 1);
        ringbuf_put((ringbuf_t*)&uarte_rx_ringbuf, chr);
    }
}

bool uarte_rx_any(void) {
    return uarte_rx_ringbuf.iput != uarte_rx_ringbuf.iget;
}

int uarte_rx_char(void) {
    return ringbuf_get((ringbuf_t*)&uarte_rx_ringbuf);
}
#endif

static void board_init(void) {
    // Config clock source.
#ifndef BLUETOOTH_SD
    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
    NRF_CLOCK->TASKS_LFCLKSTART = 1UL;
#endif

    // Priorities 0, 1, 4 (nRF52) are reserved for SoftDevice
    // 2 is highest for application
    NRFX_IRQ_PRIORITY_SET(USBD_IRQn, 2);

    // USB power may already be ready at this time -> no event generated
    // We need to invoke the handler based on the status initially
    uint32_t usb_reg;

#ifdef BLUETOOTH_SD
    uint8_t sd_en = false;
    sd_softdevice_is_enabled(&sd_en);

    if (sd_en) {
        sd_power_usbdetected_enable(true);
        sd_power_usbpwrrdy_enable(true);
        sd_power_usbremoved_enable(true);

        sd_power_usbregstatus_get(&usb_reg);
    } else
#endif
    {
        // Power module init
        const nrfx_power_config_t pwr_cfg = { 0 };
        nrfx_power_init(&pwr_cfg);

        // Register tusb function as USB power handler
        const nrfx_power_usbevt_config_t config = { .handler = (nrfx_power_usb_event_handler_t) tusb_hal_nrf_power_event };
        nrfx_power_usbevt_init(&config);

        nrfx_power_usbevt_enable();

        usb_reg = NRF_POWER->USBREGSTATUS;
    }

    if (usb_reg & POWER_USBREGSTATUS_VBUSDETECT_Msk) {
        tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
    }

#ifndef BLUETOOTH_SD
    if (usb_reg & POWER_USBREGSTATUS_OUTPUTRDY_Msk) {
        tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
    }
#endif
}

static bool cdc_rx_any(void) {
    return rx_ringbuf.iput != rx_ringbuf.iget;
}

static int cdc_rx_char(void) {
    return ringbuf_get((ringbuf_t*)&rx_ringbuf);
}

static bool cdc_tx_any(void) {
    return tx_ringbuf.iput != tx_ringbuf.iget;
}

static int cdc_tx_char(void) {
    return ringbuf_get((ringbuf_t*)&tx_ringbuf);
}

static void cdc_task(void)
{
    for (uint8_t itf = 0; itf < CFG_TUD_CDC; itf++) {
        if (itf == 0) {
            if ( tud_cdc_n_connected(itf) ) {
                // connected and there are data available
                while (tud_cdc_n_available(itf)) {
                    int c;
                    uint32_t count = tud_cdc_n_read(itf, &c, 1);
                    (void)count;
                    ringbuf_put((ringbuf_t*)&rx_ringbuf, c);
                }

                int chars = 0;
                while (cdc_tx_any()) {
                    if (chars < 64) {
                       tud_cdc_n_write_char(itf, cdc_tx_char());
                       chars++;
                    } else {
                       chars = 0;
                       tud_cdc_n_write_flush(itf);
                    }
                }

                tud_cdc_n_write_flush(itf);
            }
        }
        #if MICROPY_HW_USB_CDC_DUAL
        else {
            if (tud_cdc_n_connected(itf)) {
                if (tud_cdc_n_available(itf)) {
                     uint8_t buf[4096];
                     uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

                     while (nrfx_uart_tx_in_progress(&uart_instance)) {
                         ;
                     }

                     nrfx_uart_tx(&uart_instance, buf, count);
                 }

                 while (uarte_rx_any()) {
                     tud_cdc_n_write_char(itf, uarte_rx_char());
                     tud_cdc_n_write_flush(itf);
                 }
             }
        }
        #endif
    }
}

static void usb_cdc_loop(void) {
    tud_task();
    cdc_task();
}

int usb_cdc_init(void)
{
    static bool initialized = false;
    if (!initialized) {

#if BLUETOOTH_SD
        // Initialize the clock and BLE stack.
        ble_drv_stack_enable();
#endif

        board_init();
        initialized = true;
    }

    rx_ringbuf.buf = rx_ringbuf_array;
    rx_ringbuf.size = sizeof(rx_ringbuf_array);
    rx_ringbuf.iget = 0;
    rx_ringbuf.iput = 0;

    tx_ringbuf.buf = tx_ringbuf_array;
    tx_ringbuf.size = sizeof(tx_ringbuf_array);
    tx_ringbuf.iget = 0;
    tx_ringbuf.iput = 0;

#if MICROPY_HW_USB_CDC_DUAL
    uarte_rx_ringbuf.buf = uarte_rx_ringbuf_array;
    uarte_rx_ringbuf.size = sizeof(uarte_rx_ringbuf_array);
    uarte_rx_ringbuf.iget = 0;
    uarte_rx_ringbuf.iput = 0;

    nrfx_uart_config_t config;

    config.hal_cfg.hwfc = MICROPY_HW_USB_CDC2_UART_HWFC;
    config.hal_cfg.parity = NRF_UART_PARITY_EXCLUDED;
    config.interrupt_priority = 3;
    config.interrupt_priority = 6;

    uint32_t baud = MICROPY_HW_USB_CDC2_UART_BAUDRATE;

    // Magic: calculate 'baudrate' register from the input number.
    // Every value listed in the datasheet will be converted to the
    // correct register value, except for 192600. I believe the value
    // listed in the nrf52 datasheet (0x0EBED000) is incorrectly rounded
    // and should be 0x0EBEE000, as the nrf51 datasheet lists the
    // nonrounded value 0x0EBEDFA4.
    // Some background:
    // https://devzone.nordicsemi.com/f/nordic-q-a/391/uart-baudrate-register-values/2046#2046
    config.baudrate = baud / 400 * (uint32_t)(400ULL * (uint64_t)UINT32_MAX / 16000000ULL);
    config.baudrate = (config.baudrate + 0x800) & 0xffffff000; // rounding

    config.pseltxd = MICROPY_HW_USB_CDC2_UART_TX;
    config.pselrxd = MICROPY_HW_USB_CDC2_UART_RX;
    config.pselrts = MICROPY_HW_USB_CDC2_UART_RTS;
    config.pselcts = MICROPY_HW_USB_CDC2_UART_CTS;

    // Set context to this instance of UART
    config.p_context = NULL;

    // Enable event callback and start asynchronous receive
    nrfx_uart_init(&uart_instance, &config, uart_event_handler);
    nrfx_uart_rx(&uart_instance, &uarte_rx_buf[0], 1);
    nrfx_uart_rx_enable(&uart_instance);

#endif

    tusb_init();

    return 0;
}

#ifdef BLUETOOTH_SD
// process SOC event from SD
void usb_cdc_sd_event_handler(uint32_t soc_evt) {
    /*------------- usb power event handler -------------*/
    int32_t usbevt = (soc_evt == NRF_EVT_POWER_USB_DETECTED   ) ? NRFX_POWER_USB_EVT_DETECTED:
                     (soc_evt == NRF_EVT_POWER_USB_POWER_READY) ? NRFX_POWER_USB_EVT_READY   :
                     (soc_evt == NRF_EVT_POWER_USB_REMOVED    ) ? NRFX_POWER_USB_EVT_REMOVED : -1;

    if (usbevt >= 0) {
        tusb_hal_nrf_power_event(usbevt);
    }
}
#endif

int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        usb_cdc_loop();
        if (cdc_rx_any()) {
            return cdc_rx_char();
        }
    }

    return 0;
}

void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {

    for (const char *top = str + len; str < top; str++) {
        ringbuf_put((ringbuf_t*)&tx_ringbuf, *str);
        usb_cdc_loop();
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, mp_uint_t len) {

    for (const char *top = str + len; str < top; str++) {
        if (*str == '\n') {
            ringbuf_put((ringbuf_t*)&tx_ringbuf, '\r');
            usb_cdc_loop();
        }
        ringbuf_put((ringbuf_t*)&tx_ringbuf, *str);
        usb_cdc_loop();
    }
}

#endif // MICROPY_HW_USB_CDC
