#ifndef my_ili_9341_h
#define my_ili_9341_h 1
#include "pico/stdlib.h"

#define HORIZONTAL 0
#define VERTICAL 1

/* inits the display */
void ili9341_init();

/*inits the touchfunction of the diplay */
void ili9341_touch_init();

/*returns the position [0..240, 0..320] of the touch, or [0xffff, 0xffff] if no touch
 * unabhaengig von der orientation*/
uint16_t *ili9341_getTouch();

/* Setzt die Orientierung, Parameter ori sind HORIZONTAL und VERTICAL */
void setOrientation(uint8_t ori);

/* 565-Bit RGB-Farbwert
 * Alternativ-Textfarbe */
void setSeColor(uint16_t color);

/* 565-Bit RGB-Farbwert
 * Textfarbe */
void setFgColor(uint16_t color);

/* 565-Bit RGB-Farbwert
 * Hintergrundfarbe */
void setBgColor(uint16_t color);

/* Fuellt das Display mit der Hintergrundfarbe */
void clearScreen();

/* Schreibt char *text mit 7x11 Pixeln
 * pos[0]: ist die Spalte, pos[1] die Zeile in Pixel wenn matrix = false.
 * text: der Text der angezeigt wird
 * len: ist die anzahl der Zeichen des Textes
 * sel: wenn true wird der Text statt in FGground in SelColor angezeigt.
 * matrix: wenn true dann beziehen sich die Koordinatn in pos auf eine Matrix
 * deren Felder mit der x-Kantelänge von 11 Pixeln sind. */
void writeText7x11(uint16_t *pos, char *text, bool sel, bool matrix);
void writeText10x16(uint16_t *pos, char *text, bool sel, bool matrix);
void writeText12x16(uint16_t *pos, char *text, bool sel, bool matrix);
void writeText14x20(uint16_t *pos, char *text, bool sel, bool matrix);

/* zeichnet die 565-Bit RGB-Farbwert color in den
 * durch area bezeichneten Bereich
 * area[0]: position x.start, area[1] position y.start
 * area[12] position x.end, area[3] position y.end */
void paintRect(uint16_t *area, uint16_t color);

/* zeichnet die 565bit-farbcodierten Daten in den
 * durch area bezeichneten Bereich
 * area[0]: position x.start, area[1] position y.start
 * area[12] position x.end, area[3] position y.end
 * *data: die Daten die gezeichnet werden */
void drawRect(uint16_t *area, uint8_t *data);

/* 565-Bit RGB-Farbwert
 * zeichnet einen horizontalen Verlauf von color1 zu color2
 * durch "setOrientation(HORIZONTAL/VERTICAL) kann der Verlauf auch vertikal erfolgen*/
void paintRectGradient(uint16_t *area, uint16_t color1, uint16_t color2);

#endif
