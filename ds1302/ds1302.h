#ifndef my_ds1302_h
#define my_ds1302_h 1

#include "pico/stdlib.h"
#include "pico/util/datetime.h"

void ds1302_init(uint8_t scl, uint8_t io, uint8_t cs);

datetime_t ds1302_getDate();

void ds1302_setDate(datetime_t datum);

#endif
