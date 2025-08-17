/* Der Radiowecker mit RDS, sechs Farbmodi und vier Seek-Empfindlichkeiten
Hardware
 * * ili9341-Display_320x240, spi
 * * si4703-FMRadio, i2c
 * * ds1302-RTC, 3wire
 * * 5-Wege-Button, COM on Ground
 * * Radio ON/OFF digital-out on GP_20/GP_2, for PowerOn/Off-switch an Amplifier
Pins
 * PIN_RADIO_ON/OFF for org GP_20 / for zero GP_2 out for switch
 * RTC SCL for org GP_21 / for zero GP_3
 * RTC IO for org GP_2 / for zero GP_4
 * RTC CS for org GP_3 / for zero GP_5
 * BUTTON_M GP_6
 * display DC GP_7
 * display reset GP_8
 * display cs/csn GP_9
 * display SCK/SPI_SCK GP_10
 * display SDIO/SPI_TX GP_11
 * BUTTON_U GP_12
 * BUTTON_D GP_13
 * BUTTON_L GP_14
 * BUTTON_R GP_15
 * si4703 SDA GP_16 / for zero GP_26
 * si4703 SCL GP_17 / for zero GP_27
 * si4703 RES GP_18 / for zero GP_28
 * si4703 GP2 GP_19 / for zero GP_29
 * */

#ifndef my_uhr_h
#define my_uhr_h 1

#include "ili_9341/ili_9341.h"
#include "ili_9341/ziffer15x11.h"
#include "ili_9341/ziffer10x7.h"
#include "ili_9341/navi.h"
#include "si4703/si4703.h"
#include "momefilo_flash/momefilo_flash.h"
#include "ds1302/ds1302.h"
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

void uhr_init(uint16_t x, uint16_t y, uint16_t width, uint8_t color);

#endif
