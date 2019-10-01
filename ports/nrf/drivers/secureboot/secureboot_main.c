/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Glenn Ruben Bakke
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

#include <arm_cmse.h>
#include <nrf.h>

#define SECUREBOOT_TDFU (1)

#if SECUREBOOT_TDFU
#include <string.h>
#include "nrf_gpio.h"
#include "nrfx_uarte.h"
#include "nrfx_nvmc.h"
#include "mpconfigboard.h"

#if defined(PCA10090)
#define SECUREBOOT_TDFU_BUTTON         (6)
#define SECUREBOOT_TDFU_BUTTON_PULLUP  (1)
#define SECUREBOOT_TDFU_LED            (MICROPY_HW_LED4)
#define SECUREBOOT_TDFU_LED_PULLUP     (MICROPY_HW_LED_PULLUP)
#define SECUREBOOT_TDFU_BAUDRATE       1000000
#elif defined(PCA20035)
#define SECUREBOOT_TDFU_BUTTON         (26)
#define SECUREBOOT_TDFU_BUTTON_PULLUP  (1)
#define SECUREBOOT_TDFU_LED            (MICROPY_HW_LED3)
#define SECUREBOOT_TDFU_LED_PULLUP     (MICROPY_HW_LED_PULLUP)
#define SECUREBOOT_TDFU_BAUDRATE       115200
#else
#define SECUREBOOT_TDFU_BUTTON         (5)
#define SECUREBOOT_TDFU_BUTTON_PULLUP  (1)
#define SECUREBOOT_TDFU_LED            (MICROPY_HW_LED3)
#define SECUREBOOT_TDFU_LED_PULLUP     (MICROPY_HW_LED_PULLUP)
#define SECUREBOOT_TDFU_BAUDRATE       1000000
#endif

#endif

// Secure flash 32K.
#define SECURE_32K_FLASH_PAGE_START    (0)
#define SECURE_32K_FLASH_PAGE_END      (0)

// Non-secure flash 992K.
#define NONSECURE_32K_FLASH_PAGE_START (1)
#define NONSECURE_32K_FLASH_PAGE_END   (31)

// Secure RAM 64K.
#define SECURE_8K_RAM_BLOCK_START      (0)
#define SECURE_8K_RAM_BLOCK_END        (7)

// Non-secure RAM 128K + 64K BSD lib.
#define NONSECURE_8K_RAM_BLOCK_START   (8)
#define NONSECURE_8K_RAM_BLOCK_END     (31)

#define PERIPHERAL_ID_GET(base_addr) (((uint32_t)(base_addr) >> 12) & 0xFF)

#if !defined(__ARM_FEATURE_CMSE)
    #pragma warning "CMSE not enabled"
#endif

static void configure_flash(void) {
    for (uint8_t i = SECURE_32K_FLASH_PAGE_START; i <= SECURE_32K_FLASH_PAGE_END; i++) {
        uint32_t perm = 0;
        perm |= (SPU_FLASHREGION_PERM_EXECUTE_Enable << SPU_FLASHREGION_PERM_EXECUTE_Pos);
        perm |= (SPU_FLASHREGION_PERM_WRITE_Enable   << SPU_FLASHREGION_PERM_WRITE_Pos);
        perm |= (SPU_FLASHREGION_PERM_READ_Enable    << SPU_FLASHREGION_PERM_READ_Pos);
        perm |= (SPU_FLASHREGION_PERM_LOCK_Locked    << SPU_FLASHREGION_PERM_LOCK_Pos);
        perm |= (SPU_FLASHREGION_PERM_SECATTR_Secure << SPU_FLASHREGION_PERM_SECATTR_Pos);
        NRF_SPU_S->FLASHREGION[i].PERM = perm;
    }

    for (uint8_t i = NONSECURE_32K_FLASH_PAGE_START; i <= NONSECURE_32K_FLASH_PAGE_END; i++) {
        uint32_t perm = 0;
        perm |= (SPU_FLASHREGION_PERM_EXECUTE_Enable     << SPU_FLASHREGION_PERM_EXECUTE_Pos);
        perm |= (SPU_FLASHREGION_PERM_WRITE_Enable       << SPU_FLASHREGION_PERM_WRITE_Pos);
        perm |= (SPU_FLASHREGION_PERM_READ_Enable        << SPU_FLASHREGION_PERM_READ_Pos);
        perm |= (SPU_FLASHREGION_PERM_LOCK_Locked        << SPU_FLASHREGION_PERM_LOCK_Pos);
        perm |= (SPU_FLASHREGION_PERM_SECATTR_Non_Secure << SPU_FLASHREGION_PERM_SECATTR_Pos);
        NRF_SPU_S->FLASHREGION[i].PERM = perm;
    }
}

static void configure_ram(void) {
    for (uint8_t i = SECURE_8K_RAM_BLOCK_START; i <= SECURE_8K_RAM_BLOCK_END; i++) {
        uint32_t perm = 0;
        perm |= (SPU_RAMREGION_PERM_EXECUTE_Enable << SPU_RAMREGION_PERM_EXECUTE_Pos);
        perm |= (SPU_RAMREGION_PERM_WRITE_Enable << SPU_RAMREGION_PERM_WRITE_Pos);
        perm |= (SPU_RAMREGION_PERM_READ_Enable << SPU_RAMREGION_PERM_READ_Pos);
        perm |= (SPU_RAMREGION_PERM_LOCK_Locked << SPU_RAMREGION_PERM_LOCK_Pos);
        perm |= (SPU_RAMREGION_PERM_SECATTR_Secure << SPU_RAMREGION_PERM_SECATTR_Pos);
        NRF_SPU_S->RAMREGION[i].PERM = perm;
    }

    for (uint8_t i = NONSECURE_8K_RAM_BLOCK_START; i <= NONSECURE_8K_RAM_BLOCK_END; i++) {
        uint32_t perm = 0;
        perm |= (SPU_RAMREGION_PERM_EXECUTE_Enable << SPU_RAMREGION_PERM_EXECUTE_Pos);
        perm |= (SPU_RAMREGION_PERM_WRITE_Enable << SPU_RAMREGION_PERM_WRITE_Pos);
        perm |= (SPU_RAMREGION_PERM_READ_Enable << SPU_RAMREGION_PERM_READ_Pos);
        perm |= (SPU_RAMREGION_PERM_LOCK_Locked << SPU_RAMREGION_PERM_LOCK_Pos);
        perm |= (SPU_RAMREGION_PERM_SECATTR_Non_Secure << SPU_RAMREGION_PERM_SECATTR_Pos);
        NRF_SPU_S->RAMREGION[i].PERM = perm;
    }
}

static void peripheral_setup(uint8_t peripheral_id)
{
    NVIC_DisableIRQ(peripheral_id);
    uint32_t perm = 0;
    perm |= (SPU_PERIPHID_PERM_PRESENT_IsPresent << SPU_PERIPHID_PERM_PRESENT_Pos);
    perm |= (SPU_PERIPHID_PERM_SECATTR_NonSecure << SPU_PERIPHID_PERM_SECATTR_Pos);
    perm |= (SPU_PERIPHID_PERM_LOCK_Locked << SPU_PERIPHID_PERM_LOCK_Pos);
    NRF_SPU_S->PERIPHID[peripheral_id].PERM = perm;

    NVIC_SetTargetState(peripheral_id);
}

static void configure_peripherals(void)
{
    NRF_SPU_S->GPIOPORT[0].PERM = 0;
    peripheral_setup(PERIPHERAL_ID_GET(NRF_REGULATORS_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_CLOCK_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_UARTE0_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_UARTE1_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_UARTE2_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_UARTE3_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_SAADC_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_TIMER0_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_TIMER1_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_TIMER2_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_RTC0_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_RTC1_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_DPPIC_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_WDT_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_EGU1_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_EGU2_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_EGU3_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_EGU4_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_EGU5_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_PWM0_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_PWM1_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_PWM2_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_PWM3_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_PDM_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_I2S_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_IPC_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_FPU_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_GPIOTE1_NS));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_NVMC_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_VMC_S));
    peripheral_setup(PERIPHERAL_ID_GET(NRF_P0_NS));
}

typedef void __attribute__((cmse_nonsecure_call)) nsfunc(void);

static void jump_to_non_secure(void)
{
    TZ_SAU_Disable();
    SAU->CTRL |= SAU_CTRL_ALLNS_Msk;

    // Set NS vector table.
    uint32_t * vtor_ns = (uint32_t *)0x8000;
    SCB_NS->VTOR = (uint32_t)vtor_ns;

    // Allow for FPU to be used by NS.
    SCB->NSACR |= (1UL << SCB_NSACR_CP10_Pos) | (1UL << SCB_NSACR_CP11_Pos);

    // Set stack pointers.
    __TZ_set_MSP_NS(vtor_ns[0]);
    __TZ_set_PSP_NS(0);

    uint32_t control_ns = __TZ_get_CONTROL_NS();
    control_ns &= ~(CONTROL_SPSEL_Msk | CONTROL_nPRIV_Msk);
    __TZ_set_CONTROL_NS(control_ns);

    // Cast NS Reset_Handler to a non-secure function.
    nsfunc *fp = (nsfunc *)vtor_ns[1];
    fp =  (nsfunc *)((intptr_t)(fp) & ~1);

    if (cmse_is_nsfptr(fp)) {
        __DSB();
        __ISB();

        // Jump to Non-Secure function.
        fp();
    }
}

#if SECUREBOOT_TDFU

#ifdef PCA10090
static void delay_us(uint32_t us)
{
    if (us == 0) {
        return;
    }
    register uint32_t delay __ASM ("r0") = us;
    __ASM volatile (
        "1:\n"
        " SUBS %0, %0, #1\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " NOP\n"
        " BNE 1b\n"
        : "+r" (delay));
}

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        delay_us(999);
    }
}
#endif

static const nrfx_uarte_t uarte_instance = NRFX_UARTE_INSTANCE(0);

uint8_t uart_rx_char(void) {
    uint8_t ch;
    nrfx_uarte_rx(&uarte_instance, &ch, 1);
    return ch;
}

void uart_tx_char(char c) {
    nrfx_uarte_tx(&uarte_instance, (uint8_t *)&c, 1);
}

void uart_tx_string(char * p_str, uint16_t str_len) {
    nrfx_uarte_tx(&uarte_instance, (uint8_t *)p_str, str_len);
}

void secureboot_tdfu(void) {
#ifdef PCA10090
    delay_ms(200);
#endif
    nrf_gpio_cfg_input(SECUREBOOT_TDFU_BUTTON,
                       (SECUREBOOT_TDFU_BUTTON_PULLUP)
                       ? NRF_GPIO_PIN_PULLUP
                       : NRF_GPIO_PIN_PULLDOWN);

    uint32_t button_value = nrf_gpio_pin_read(SECUREBOOT_TDFU_BUTTON);
    bool enter_tdfu = (SECUREBOOT_TDFU_BUTTON_PULLUP)
                    ? (~button_value & 0x1)
                    : (button_value & 0x1);

    if (enter_tdfu) {

        nrf_gpio_cfg_output(SECUREBOOT_TDFU_LED);
        if (SECUREBOOT_TDFU_LED_PULLUP) {
            nrf_gpio_pin_clear(SECUREBOOT_TDFU_LED);
        } else {
            nrf_gpio_pin_set(SECUREBOOT_TDFU_LED);
        }

        nrfx_uarte_config_t config;
        config.hwfc               = NRF_UARTE_HWFC_ENABLED;
        config.parity             = NRF_UARTE_PARITY_EXCLUDED;
        config.interrupt_priority = 1;
        uint32_t baudrate = SECUREBOOT_TDFU_BAUDRATE;

        // Magic: calculate 'baudrate' register from the input number.
        // Every value listed in the datasheet will be converted to the
        // correct register value, except for 192600. I believe the value
        // listed in the nrf52 datasheet (0x0EBED000) is incorrectly rounded
        // and should be 0x0EBEE000, as the nrf51 datasheet lists the
        // nonrounded value 0x0EBEDFA4.
        // Some background:
        // https://devzone.nordicsemi.com/f/nordic-q-a/391/uart-baudrate-register-values/2046#2046
        config.baudrate = baudrate / 400 * (uint32_t)(400ULL * (uint64_t)UINT32_MAX / 16000000ULL);
        config.baudrate = (config.baudrate + 0x800) & 0xffffff000; // rounding

        config.pseltxd = MICROPY_HW_UART1_TX;
        config.pselrxd = MICROPY_HW_UART1_RX;
#if MICROPY_HW_UART1_HWFC
        config.pselrts = MICROPY_HW_UART1_RTS;
        config.pselcts = MICROPY_HW_UART1_CTS;
#endif
        config.p_context = NULL;

        nrfx_uarte_init(&uarte_instance, &config, NULL);

        bool hold = true;
        do {
            button_value = nrf_gpio_pin_read(SECUREBOOT_TDFU_BUTTON);
            hold = (SECUREBOOT_TDFU_BUTTON_PULLUP)
                 ? (~button_value & 0x1)
                 : (button_value & 0x1);
        } while (hold);

        char secureboot_string[] = "Secureboot TDFU\r\n";
        uart_tx_string(secureboot_string, strlen(secureboot_string));

        // Echo a char from sender, to acknowledge we are in sync.
        char c;
        nrfx_uarte_rx(&uarte_instance, (uint8_t *)&c, sizeof(char));
        uart_tx_char(c);

        uint32_t image_len = 0;
        nrfx_uarte_rx(&uarte_instance, (uint8_t *)&image_len, sizeof(uint32_t));
        uint16_t fragment_size = 0;
        nrfx_uarte_rx(&uarte_instance, (uint8_t *)&fragment_size, sizeof(uint16_t));

        uint16_t pages = image_len / 4096;
        uint16_t leftover = (image_len % 4096);
        uint16_t all_pages = pages + ((leftover) ? 1 : 0);
        uint16_t full_fragments = image_len / fragment_size;
        uint16_t leftover_fragment = image_len % fragment_size;
        uint32_t start_addr = (32768 * NONSECURE_32K_FLASH_PAGE_START);

        // Erase pages.
        for (int i = 0; i < all_pages; i++) {
            nrfx_err_t err = nrfx_nvmc_page_erase(start_addr + (i * 4096));
            if (err != NRFX_SUCCESS) {
                uart_tx_char('F');
                goto cleanup;
            }
        }

        if (all_pages) {
            uart_tx_char('E');
        } else {
            uart_tx_char('P');
        }

        // Start receive full pages.
        uint8_t buffer[4096];

        for (int i = 0; i < full_fragments; i++) {
            nrfx_uarte_rx(&uarte_instance, buffer, fragment_size);
            uint32_t write_addr = start_addr + (i * fragment_size);
            nrfx_nvmc_bytes_write(write_addr, buffer, fragment_size);
            uart_tx_char('A');
        }

        if (leftover_fragment) {
            nrfx_uarte_rx(&uarte_instance, (uint8_t *)buffer, leftover_fragment);
            uint32_t write_addr = start_addr + (full_fragments * fragment_size);
            nrfx_nvmc_bytes_write(write_addr, buffer, leftover_fragment);
            uart_tx_char('A');
        }

cleanup:
        uart_tx_char('D');
        // Clean up UARTE before leaving secure domain.
        nrfx_uarte_uninit(&uarte_instance);
    }

    // Cleanup use of GPIO before leaving secure domain.
    nrf_gpio_input_disconnect(SECUREBOOT_TDFU_BUTTON);
}
#endif

int main(void) {
#if SECUREBOOT_TDFU
    secureboot_tdfu();
#endif
    configure_flash();
    configure_ram();
    configure_peripherals();

    jump_to_non_secure();

    while (1) {
	;
    }

    return 0;
}

void _start(void) {main();}

