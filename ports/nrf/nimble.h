
#pragma once

#include <stdint.h>
#include <stdbool.h>

void nimble_init();

extern volatile bool in_critical;
extern uint8_t *nimble_current_task;
extern uint8_t nimble_started;

void nimble_schedule_callouts(void);

#if 1
#define bleprintf(...)
#else
#define bleprintf printf
#endif
