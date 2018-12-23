#include <bsd_os.h>
#include <nrf_errno.h>
#include <bsd_platform.h>
#include <bsd_limits.h>

#include <nrf.h>
#include "nrf_gpio.h"
#include "errno.h"

#define BSD_OS_TIMER_ENABLED 0

#if BSD_OS_TIMER_ENABLED

#include "app_timer.h"

#define SEC_TO_MILLISEC(PARAM)         (PARAM * 1000)
#define BSD_OS_TIMER_RESOLUTION_IN_MS  5

// BSD_MAX_IP_SOCKET_COUNT + 1
APP_TIMER_DEF(m_timer0);
APP_TIMER_DEF(m_timer1);
APP_TIMER_DEF(m_timer2);
APP_TIMER_DEF(m_timer3);
APP_TIMER_DEF(m_timer4);
APP_TIMER_DEF(m_timer5);
APP_TIMER_DEF(m_timer6);
APP_TIMER_DEF(m_timer7);

static const app_timer_id_t m_timers[BSD_MAX_IP_SOCKET_COUNT + 1] = {
    &m_timer0_data,
    &m_timer1_data,
    &m_timer2_data,
    &m_timer3_data,
    &m_timer4_data,
    &m_timer5_data,
    &m_timer6_data,
    &m_timer7_data
};

#define INVALID_CONTEXT 0xFFFFFFFF

typedef struct _bsd_timer_instance_t {
    app_timer_id_t   timer;
    uint32_t         context;
    bool             started;
    bool             timeout;
} bsd_timer_instance_t;

// static volatile uint8_t m_timer_running[BSD_MAX_IP_SOCKET_COUNT + 1] = {0,};
static volatile bsd_timer_instance_t m_bsd_timers[8];

static void bsd_timer_reset(uint8_t index) {
    m_bsd_timers[index].timer   = m_timers[index];
    m_bsd_timers[index].context = INVALID_CONTEXT;
    m_bsd_timers[index].started = false;
    m_bsd_timers[index].timeout = false;
}

static volatile bsd_timer_instance_t * bsd_timer_find(uint32_t context) {
    for (uint8_t i = 0; i < MP_ARRAY_SIZE(m_bsd_timers); i++) {
        if (m_bsd_timers[i].context == context) {
            return &m_bsd_timers[i];
        }
    }
    return NULL;
}

static volatile bsd_timer_instance_t * bsd_timer_allocate(uint32_t context) {
    for (uint8_t i = 0; i < MP_ARRAY_SIZE(m_bsd_timers); i++) {
        if (m_bsd_timers[i].context == INVALID_CONTEXT) {
            m_bsd_timers[i].context = context;
            return &m_bsd_timers[i];
        }
    }
    return NULL;
}

static bool bsd_timer_free(uint32_t context) {
    for (uint8_t i = 0; i < MP_ARRAY_SIZE(m_bsd_timers); i++) {
        if (m_bsd_timers[i].context == context) {
            bsd_timer_reset(i);
            return true;
        }
    }
    return false;
}

static void iot_timer_tick_callback(void * p_context)
{
    uint32_t context = ((uint32_t)p_context);
    volatile bsd_timer_instance_t * p_timer = bsd_timer_find(context);
    if (p_timer != NULL)
    {
        p_timer->timeout = true;
    }
}
#endif // BSD_OS_TIMER_ENABLED

void read_task_create(void)
{
    // The read task is achieved using SW interrupt.
    NVIC_SetPriority(BSD_APPLICATION_IRQ, BSD_APPLICATION_IRQ_PRIORITY);
    NVIC_ClearPendingIRQ(BSD_APPLICATION_IRQ);
    NVIC_EnableIRQ(BSD_APPLICATION_IRQ);
}

void bsd_os_init(void)
{
    nrf_gpio_cfg_output(5);
    nrf_gpio_pin_set(5);
    read_task_create();

#if BSD_OS_TIMER_ENABLED
    app_timer_init();
    for (uint8_t i = 0; i < MP_ARRAY_SIZE(m_bsd_timers); i++) {
        (void)app_timer_create(&m_timers[i],
                               APP_TIMER_MODE_SINGLE_SHOT,
                               iot_timer_tick_callback);
    }

    for (uint8_t i = 0; i < MP_ARRAY_SIZE(m_bsd_timers); i++) {
        bsd_timer_reset(i);
    }

    (void)m_timer0;
    (void)m_timer1;
    (void)m_timer2;
    (void)m_timer3;
    (void)m_timer4;
    (void)m_timer5;
    (void)m_timer6;
    (void)m_timer7;

#endif // BSD_OS_TIMER_ENABLED
}

int32_t bsd_os_timedwait(uint32_t context, uint32_t timeout)
{
    NVIC_SetPendingIRQ(IPC_IRQn);
#if BSD_OS_TIMER_ENABLED
    if (timeout != 0)
    {
        // Check if we need to create a timer.
        volatile bsd_timer_instance_t * p_timer = bsd_timer_find(context);

        if (p_timer == NULL) {
            p_timer = bsd_timer_allocate(context);
            p_timer->started = false;
            p_timer->timeout = false;
        }

        if (p_timer != NULL) {
            if (p_timer->started == false && p_timer->timeout == false) {
                p_timer->started = true;
                (void)app_timer_start(p_timer->timer,
                                      APP_TIMER_TICKS(timeout),
                                      (void *)context);
            }

            if (p_timer->started == true && p_timer->timeout == true)
            {
                (void)app_timer_stop(p_timer->timer);
                bsd_timer_free(context);

                return NRF_ETIMEDOUT;
            }
        } else {
            // Could not allocate or find any timer instance.
            return -1;
        }
    }
//    __WFI();
#endif // BSD_OS_TIMER_ENABLED
    return 0;
}

int nrf_errno;

void bsd_os_errno_set(int errno_val)
{
    nrf_errno = errno_val;
}

void bsd_os_application_irq_set(void)
{
    NVIC_SetPendingIRQ(BSD_APPLICATION_IRQ);
    //printf("Set BSD IRQ\n");
//    bsd_os_application_irq_handler();
}


void bsd_os_application_irq_clear(void)
{
    //printf("clear BSD IRQ\n");
    NVIC_ClearPendingIRQ(BSD_APPLICATION_IRQ);
}

void BSD_APPLICATION_IRQ_HANDLER(void)
{
    //printf("handle BSD IRQ\n");
    bsd_os_application_irq_handler();
}

void bsd_os_trace_irq_set(void)
{
    bsd_os_trace_irq_handler();
}

void bsd_os_trace_irq_clear(void)
{
}

int32_t bsd_os_trace_put(const uint8_t * const p_buffer, uint32_t buf_len)
{
    return 0;
}
