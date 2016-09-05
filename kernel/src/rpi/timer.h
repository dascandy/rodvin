#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <stdint.h>

void wait_us(size_t us);
uint64_t get_time_us();

#endif


