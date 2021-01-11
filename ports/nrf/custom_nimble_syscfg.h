/**
 * This file was generated by Apache Newt version: 1.4.0
 */

#ifndef H_MYNEWT_SYSCFG_
#define H_MYNEWT_SYSCFG_

#ifndef ARRAY_SIZE
#include "py/misc.h"
#define ARRAY_SIZE MP_ARRAY_SIZE
#endif

/**
 * This macro exists to ensure code includes this header when needed.  If code
 * checks the existence of a setting directly via ifdef without including this
 * header, the setting macro will silently evaluate to 0.  In contrast, an
 * attempt to use these macros without including this header will result in a
 * compiler error.
 */
#define MYNEWT_VAL(x)                           MYNEWT_VAL_ ## x



/*** @apache-mynewt-core/compiler/arm-none-eabi-m4 */
/* TO ADJUST this to be board defined.

#ifndef MYNEWT_VAL_HARDFLOAT
#define MYNEWT_VAL_HARDFLOAT (0)
#endif
*/

#ifndef MYNEWT_VAL_BLE_VERSION
#define MYNEWT_VAL_BLE_VERSION (50)
#endif

/*** @apache-mynewt-core/hw/mcu/nordic/nrf52xxx */
#ifndef MYNEWT_VAL_ADC_0
#define MYNEWT_VAL_ADC_0 (0)
#endif

#ifndef MYNEWT_VAL_ADC_0_REFMV_0
#define MYNEWT_VAL_ADC_0_REFMV_0 (0)
#endif

#ifndef MYNEWT_VAL_I2C_0
#define MYNEWT_VAL_I2C_0 (0)
#endif

#ifndef MYNEWT_VAL_I2C_1
#define MYNEWT_VAL_I2C_1 (0)
#endif

#ifndef MYNEWT_VAL_TIMER_5
#define MYNEWT_VAL_TIMER_5 (1)
#endif


/* Overridden by @apache-mynewt-core/hw/bsp/nrf52dk (defined by @apache-mynewt-core/hw/mcu/nordic/nrf52xxx) */
#ifndef MYNEWT_VAL_MCU_DCDC_ENABLED
#define MYNEWT_VAL_MCU_DCDC_ENABLED (1)
#endif

#ifndef MYNEWT_VAL_MCU_FLASH_MIN_WRITE_SIZE
#define MYNEWT_VAL_MCU_FLASH_MIN_WRITE_SIZE (1)
#endif

#ifndef MYNEWT_VAL_PWM_0
#define MYNEWT_VAL_PWM_0 (0)
#endif

#ifndef MYNEWT_VAL_PWM_1
#define MYNEWT_VAL_PWM_1 (0)
#endif

#ifndef MYNEWT_VAL_PWM_2
#define MYNEWT_VAL_PWM_2 (0)
#endif

#ifndef MYNEWT_VAL_QSPI_ADDRMODE
#define MYNEWT_VAL_QSPI_ADDRMODE (0)
#endif

#ifndef MYNEWT_VAL_QSPI_DPMCONFIG
#define MYNEWT_VAL_QSPI_DPMCONFIG (0)
#endif

#ifndef MYNEWT_VAL_QSPI_ENABLE
#define MYNEWT_VAL_QSPI_ENABLE (0)
#endif

#ifndef MYNEWT_VAL_QSPI_FLASH_PAGE_SIZE
#define MYNEWT_VAL_QSPI_FLASH_PAGE_SIZE (0)
#endif

#ifndef MYNEWT_VAL_QSPI_FLASH_SECTOR_COUNT
#define MYNEWT_VAL_QSPI_FLASH_SECTOR_COUNT (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_FLASH_SECTOR_SIZE
#define MYNEWT_VAL_QSPI_FLASH_SECTOR_SIZE (0)
#endif

#ifndef MYNEWT_VAL_QSPI_PIN_CS
#define MYNEWT_VAL_QSPI_PIN_CS (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_PIN_DIO0
#define MYNEWT_VAL_QSPI_PIN_DIO0 (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_PIN_DIO1
#define MYNEWT_VAL_QSPI_PIN_DIO1 (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_PIN_DIO2
#define MYNEWT_VAL_QSPI_PIN_DIO2 (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_PIN_DIO3
#define MYNEWT_VAL_QSPI_PIN_DIO3 (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_PIN_SCK
#define MYNEWT_VAL_QSPI_PIN_SCK (-1)
#endif

#ifndef MYNEWT_VAL_QSPI_READOC
#define MYNEWT_VAL_QSPI_READOC (0)
#endif

#ifndef MYNEWT_VAL_QSPI_SCK_DELAY
#define MYNEWT_VAL_QSPI_SCK_DELAY (0)
#endif

#ifndef MYNEWT_VAL_QSPI_SCK_FREQ
#define MYNEWT_VAL_QSPI_SCK_FREQ (0)
#endif

#ifndef MYNEWT_VAL_QSPI_SPI_MODE
#define MYNEWT_VAL_QSPI_SPI_MODE (0)
#endif

#ifndef MYNEWT_VAL_QSPI_WRITEOC
#define MYNEWT_VAL_QSPI_WRITEOC (0)
#endif

#ifndef MYNEWT_VAL_SOFT_PWM
#define MYNEWT_VAL_SOFT_PWM (0)
#endif

#ifndef MYNEWT_VAL_SPI_0_MASTER
#define MYNEWT_VAL_SPI_0_MASTER (0)
#endif

#ifndef MYNEWT_VAL_SPI_0_SLAVE
#define MYNEWT_VAL_SPI_0_SLAVE (0)
#endif

#ifndef MYNEWT_VAL_SPI_1_MASTER
#define MYNEWT_VAL_SPI_1_MASTER (0)
#endif

#ifndef MYNEWT_VAL_SPI_1_SLAVE
#define MYNEWT_VAL_SPI_1_SLAVE (0)
#endif

#ifndef MYNEWT_VAL_SPI_2_MASTER
#define MYNEWT_VAL_SPI_2_MASTER (0)
#endif

#ifndef MYNEWT_VAL_SPI_2_SLAVE
#define MYNEWT_VAL_SPI_2_SLAVE (0)
#endif

#ifndef MYNEWT_VAL_TRNG
#define MYNEWT_VAL_TRNG (0)
#endif

/* Overridden by @apache-mynewt-core/hw/bsp/nrf52dk (defined by @apache-mynewt-core/hw/mcu/nordic/nrf52xxx) */
#ifndef MYNEWT_VAL_XTAL_32768
#define MYNEWT_VAL_XTAL_32768 (1)
#endif

#ifndef MYNEWT_VAL_XTAL_32768_SYNTH
#define MYNEWT_VAL_XTAL_32768_SYNTH (0)
#endif

#ifndef MYNEWT_VAL_XTAL_RC
#define MYNEWT_VAL_XTAL_RC (0)
#endif

/*** @apache-mynewt-core/kernel/os */
#ifndef MYNEWT_VAL_FLOAT_USER
#define MYNEWT_VAL_FLOAT_USER (0)
#endif

#ifndef MYNEWT_VAL_MSYS_1_BLOCK_COUNT
#define MYNEWT_VAL_MSYS_1_BLOCK_COUNT (12)
#endif

#ifndef MYNEWT_VAL_MSYS_2_BLOCK_COUNT
#define MYNEWT_VAL_MSYS_2_BLOCK_COUNT (0)
#endif

#ifndef MYNEWT_VAL_MSYS_2_BLOCK_SIZE
#define MYNEWT_VAL_MSYS_2_BLOCK_SIZE (0)
#endif

#ifndef MYNEWT_VAL_OS_CLI
#define MYNEWT_VAL_OS_CLI (0)
#endif

#ifndef MYNEWT_VAL_OS_COREDUMP
#define MYNEWT_VAL_OS_COREDUMP (0)
#endif

/*** @apache-mynewt-core/libc/baselibc */
#ifndef MYNEWT_VAL_BASELIBC_ASSERT_FILE_LINE
#define MYNEWT_VAL_BASELIBC_ASSERT_FILE_LINE (0)
#endif

#ifndef MYNEWT_VAL_BASELIBC_PRESENT
#define MYNEWT_VAL_BASELIBC_PRESENT (1)
#endif

/*** @apache-mynewt-core/sys/console/stub */
#ifndef MYNEWT_VAL_CONSOLE_UART_BAUD
#define MYNEWT_VAL_CONSOLE_UART_BAUD (115200)
#endif

#ifndef MYNEWT_VAL_CONSOLE_UART_DEV
#define MYNEWT_VAL_CONSOLE_UART_DEV ("uart0")
#endif

#ifndef MYNEWT_VAL_CONSOLE_UART_FLOW_CONTROL
#define MYNEWT_VAL_CONSOLE_UART_FLOW_CONTROL (UART_FLOW_CTL_NONE)
#endif

/*** @apache-mynewt-core/sys/flash_map */
#ifndef MYNEWT_VAL_FLASH_MAP_MAX_AREAS
#define MYNEWT_VAL_FLASH_MAP_MAX_AREAS (10)
#endif

/*** @apache-mynewt-core/sys/log/stub */
#ifndef MYNEWT_VAL_LOG_CONSOLE
#define MYNEWT_VAL_LOG_CONSOLE (1)
#endif

#ifndef MYNEWT_VAL_LOG_FCB
#define MYNEWT_VAL_LOG_FCB (0)
#endif

#ifndef MYNEWT_VAL_LOG_FCB_SLOT1
#define MYNEWT_VAL_LOG_FCB_SLOT1 (0)
#endif

#ifndef MYNEWT_VAL_LOG_LEVEL
#define MYNEWT_VAL_LOG_LEVEL (255)
#endif

/*** @apache-mynewt-core/sys/sysinit */
#ifndef MYNEWT_VAL_SYSINIT_CONSTRAIN_INIT
#define MYNEWT_VAL_SYSINIT_CONSTRAIN_INIT (1)
#endif

#ifndef MYNEWT_VAL_SYSINIT_PANIC_FILE_LINE
#define MYNEWT_VAL_SYSINIT_PANIC_FILE_LINE (0)
#endif

#ifndef MYNEWT_VAL_SYSINIT_PANIC_MESSAGE
#define MYNEWT_VAL_SYSINIT_PANIC_MESSAGE (0)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_HW_WHITELIST_ENABLE
#define MYNEWT_VAL_BLE_HW_WHITELIST_ENABLE (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_ADD_STRICT_SCHED_PERIODS
#define MYNEWT_VAL_BLE_LL_ADD_STRICT_SCHED_PERIODS (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_CONN_PARAM_REQ
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_CONN_PARAM_REQ (1)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_DATA_LEN_EXT
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_DATA_LEN_EXT (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_2M_PHY
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_2M_PHY (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CODED_PHY
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CODED_PHY (0)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CSA2
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_CSA2 (1)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_ENCRYPTION
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_ENCRYPTION (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_PING
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_PING (MYNEWT_VAL_BLE_LL_CFG_FEAT_LE_ENCRYPTION)
#endif

/* Value copied from BLE_EXT_ADV */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_EXT_ADV
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_EXT_ADV (0)
#endif

/* Value copied from BLE_PERIODIC_ADV */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV (0)
#endif

/* Value copied from BLE_MAX_PERIODIC_SYNCS */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV_SYNC_CNT
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV_SYNC_CNT (0)
#endif

/* Value copied from BLE_MAX_PERIODIC_SYNCS */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV_SYNC_LIST_CNT
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV_SYNC_LIST_CNT (0)
#endif

/* Value copied from BLE_PERIODIC_ADV_SYNC_TRANSFER */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV_SYNC_TRANSFER
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PERIODIC_ADV_SYNC_TRANSFER (0)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PRIVACY
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_LL_PRIVACY (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CFG_FEAT_SLAVE_INIT_FEAT_XCHG
#define MYNEWT_VAL_BLE_LL_CFG_FEAT_SLAVE_INIT_FEAT_XCHG (1)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_CONN_INIT_MAX_TX_BYTES
#define MYNEWT_VAL_BLE_LL_CONN_INIT_MAX_TX_BYTES (MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CONN_INIT_MIN_WIN_OFFSET
#define MYNEWT_VAL_BLE_LL_CONN_INIT_MIN_WIN_OFFSET (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_CONN_INIT_SLOTS
#define MYNEWT_VAL_BLE_LL_CONN_INIT_SLOTS (4)
#endif

#ifndef MYNEWT_VAL_BLE_LL_DBG_HCI_CMD_PIN
#define MYNEWT_VAL_BLE_LL_DBG_HCI_CMD_PIN (-1)
#endif

#ifndef MYNEWT_VAL_BLE_LL_DBG_HCI_EV_PIN
#define MYNEWT_VAL_BLE_LL_DBG_HCI_EV_PIN (-1)
#endif

#ifndef MYNEWT_VAL_BLE_LL_DIRECT_TEST_MODE
#define MYNEWT_VAL_BLE_LL_DIRECT_TEST_MODE (0)
#endif

/* Value copied from BLE_LL_DIRECT_TEST_MODE */
#ifndef MYNEWT_VAL_BLE_LL_DTM
#define MYNEWT_VAL_BLE_LL_DTM (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_DTM_EXTENSIONS
#define MYNEWT_VAL_BLE_LL_DTM_EXTENSIONS (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_EXT_ADV_AUX_PTR_CNT
#define MYNEWT_VAL_BLE_LL_EXT_ADV_AUX_PTR_CNT (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_MASTER_SCA
#define MYNEWT_VAL_BLE_LL_MASTER_SCA (4)
#endif

#ifndef MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE
#define MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE (251)
#endif

#ifndef MYNEWT_VAL_BLE_LL_MFRG_ID
#define MYNEWT_VAL_BLE_LL_MFRG_ID (0xFFFF)
#endif

#ifndef MYNEWT_VAL_BLE_LL_NUM_COMP_PKT_ITVL_MS
#define MYNEWT_VAL_BLE_LL_NUM_COMP_PKT_ITVL_MS (2000)
#endif

#ifndef MYNEWT_VAL_BLE_LL_NUM_SCAN_DUP_ADVS
#define MYNEWT_VAL_BLE_LL_NUM_SCAN_DUP_ADVS (8)
#endif

#ifndef MYNEWT_VAL_BLE_LL_NUM_SCAN_RSP_ADVS
#define MYNEWT_VAL_BLE_LL_NUM_SCAN_RSP_ADVS (8)
#endif

#ifndef MYNEWT_VAL_BLE_LL_OUR_SCA
#define MYNEWT_VAL_BLE_LL_OUR_SCA (60)
#endif

#ifndef MYNEWT_VAL_BLE_LL_PRIO
#define MYNEWT_VAL_BLE_LL_PRIO (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_RESOLV_LIST_SIZE
#define MYNEWT_VAL_BLE_LL_RESOLV_LIST_SIZE (4)
#endif

/* Overridden by @apache-mynewt-core/hw/bsp/nordic_pca10056 (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_RFMGMT_ENABLE_TIME
#define MYNEWT_VAL_BLE_LL_RFMGMT_ENABLE_TIME (1500)
#endif

#ifndef MYNEWT_VAL_BLE_LL_RNG_BUFSIZE
#define MYNEWT_VAL_BLE_LL_RNG_BUFSIZE (32)
#endif

#ifndef MYNEWT_VAL_BLE_LL_SCHED_AUX_CHAIN_MAFS_DELAY
#define MYNEWT_VAL_BLE_LL_SCHED_AUX_CHAIN_MAFS_DELAY (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_SCHED_AUX_MAFS_DELAY
#define MYNEWT_VAL_BLE_LL_SCHED_AUX_MAFS_DELAY (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_SCHED_SCAN_AUX_PDU_LEN
#define MYNEWT_VAL_BLE_LL_SCHED_SCAN_AUX_PDU_LEN (41)
#endif

#ifndef MYNEWT_VAL_BLE_LL_SCHED_SCAN_SYNC_PDU_LEN
#define MYNEWT_VAL_BLE_LL_SCHED_SCAN_SYNC_PDU_LEN (32)
#endif

#ifndef MYNEWT_VAL_BLE_LL_STRICT_CONN_SCHEDULING
#define MYNEWT_VAL_BLE_LL_STRICT_CONN_SCHEDULING (0)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_SUPP_MAX_RX_BYTES
#define MYNEWT_VAL_BLE_LL_SUPP_MAX_RX_BYTES (MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE)
#endif

/* Overridden by @apache-mynewt-nimble/targets/riot (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_LL_SUPP_MAX_TX_BYTES
#define MYNEWT_VAL_BLE_LL_SUPP_MAX_TX_BYTES (MYNEWT_VAL_BLE_LL_MAX_PKT_SIZE)
#endif

#ifndef MYNEWT_VAL_BLE_LL_SYSINIT_STAGE
#define MYNEWT_VAL_BLE_LL_SYSINIT_STAGE (250)
#endif

#ifndef MYNEWT_VAL_BLE_LL_SYSVIEW
#define MYNEWT_VAL_BLE_LL_SYSVIEW (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_TX_PWR_DBM
#define MYNEWT_VAL_BLE_LL_TX_PWR_DBM (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_USECS_PER_PERIOD
#define MYNEWT_VAL_BLE_LL_USECS_PER_PERIOD (3250)
#endif

#ifndef MYNEWT_VAL_BLE_LL_VND_EVENT_ON_ASSERT
#define MYNEWT_VAL_BLE_LL_VND_EVENT_ON_ASSERT (0)
#endif

#ifndef MYNEWT_VAL_BLE_LL_WHITELIST_SIZE
#define MYNEWT_VAL_BLE_LL_WHITELIST_SIZE (8)
#endif

#ifndef MYNEWT_VAL_BLE_LP_CLOCK
#define MYNEWT_VAL_BLE_LP_CLOCK (1)
#endif

#ifndef MYNEWT_VAL_BLE_NUM_COMP_PKT_RATE
#define MYNEWT_VAL_BLE_NUM_COMP_PKT_RATE ((2 * OS_TICKS_PER_SEC))
#endif

#ifndef MYNEWT_VAL_BLE_PUBLIC_DEV_ADDR
#define MYNEWT_VAL_BLE_PUBLIC_DEV_ADDR ((uint8_t[6]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
#endif

/* Overridden by @apache-mynewt-core/hw/bsp/nrf52dk (defined by @apache-mynewt-nimble/nimble/controller) */
#ifndef MYNEWT_VAL_BLE_XTAL_SETTLE_TIME
#define MYNEWT_VAL_BLE_XTAL_SETTLE_TIME (1500)
#endif

/*** @apache-mynewt-nimble/nimble/drivers/nrf52 */
#ifndef MYNEWT_VAL_BLE_PHY_CODED_RX_IFS_EXTRA_MARGIN
#define MYNEWT_VAL_BLE_PHY_CODED_RX_IFS_EXTRA_MARGIN (0)
#endif

#ifndef MYNEWT_VAL_BLE_PHY_DBG_TIME_ADDRESS_END_PIN
#define MYNEWT_VAL_BLE_PHY_DBG_TIME_ADDRESS_END_PIN (-1)
#endif

#ifndef MYNEWT_VAL_BLE_PHY_DBG_TIME_TXRXEN_READY_PIN
#define MYNEWT_VAL_BLE_PHY_DBG_TIME_TXRXEN_READY_PIN (-1)
#endif

#ifndef MYNEWT_VAL_BLE_PHY_DBG_TIME_WFR_PIN
#define MYNEWT_VAL_BLE_PHY_DBG_TIME_WFR_PIN (-1)
#endif

#ifndef MYNEWT_VAL_BLE_PHY_NRF52840_ERRATA_164
#define MYNEWT_VAL_BLE_PHY_NRF52840_ERRATA_164 (0)
#endif

#ifndef MYNEWT_VAL_BLE_PHY_NRF52840_ERRATA_191
#define MYNEWT_VAL_BLE_PHY_NRF52840_ERRATA_191 (1)
#endif

#ifndef MYNEWT_VAL_BLE_PHY_SYSVIEW
#define MYNEWT_VAL_BLE_PHY_SYSVIEW (0)
#endif

#ifndef MYNEWT_VAL_BLE_CONTROLLER
#define MYNEWT_VAL_BLE_CONTROLLER (1)
#endif


#endif
