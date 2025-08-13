#ifndef my_uhr_h
#define my_uhr_h 1

#include "ili_9341/ili_9341.h"
#include "ili_9341/ziffer15x11.h"
#include "ili_9341/ziffer10x7.h"
#include "ili_9341/navi.h"
#include "si4703/si4703.h"
#include "momefilo_flash/momefilo_flash.h"
#include "buttons/buttons.h"
#include "pico/stdlib.h"
#include "pico/double.h"
#include "pico/malloc.h"
#include "pico/util/datetime.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"
#include <stdlib.h>
#include <stdio.h>

#define STD_GRAD 30
#define MIN_GRAD 6

uint8_t Adr = 0x68;
uint16_t StdColor = 0xF81F;
uint16_t MinColor = 0xF81E;
uint16_t ClrColor = 0x0000;
uint16_t TxtColor = 0x7C0F;
uint16_t ZifColor = 0xF81F;
uint16_t SekColor = 0xFFE0;
uint8_t Abstand = 5; // Abstand Zeischen Minutenzeiger und Ziffernblattstrichen
uint8_t ZifferWidth = 1;
uint8_t ZifferLen = 1;
uint8_t StdZeiger_width = 1;
uint8_t MinZeiger_width = 1;
uint8_t SekZeiger_width = 1;// Dicke des Sekundenwurms
uint8_t SekZeiger_len = 6;// länge des Sekundenwurms
uint8_t ZeigerDiff = 10;// Abstand zwischen Minuten- und Stundenzeigerspitze

void uhr_init(uint16_t x, uint16_t y, uint16_t width, uint8_t color, uint8_t bus, uint8_t sda, uint8_t scl);

#endif
