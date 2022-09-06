#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>
#include <stdbool.h>

#include "sm3.h"

void start_timer(uint8_t timegap);
void stop_timer();

#endif
