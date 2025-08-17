#ifndef my_buttons_h
#define my_buttons_h 1

#include "pico/stdlib.h"
/*
 * BUTTON_M GP_6
 * BUTTON_U GP_12
 * BUTTON_D GP_13
 * BUTTON_L GP_14
 * BUTTON_R GP_15
*/
#define BUTTON_U 12
#define BUTTON_D 13
#define BUTTON_L 14
#define BUTTON_R 15
#define BUTTON_M 6

/* Initialisiert gpio */
void buttons_init();

/* Gibt den aktuell gedrueckten Button zurück oder den Wert 100 */
uint8_t get_Button();

#endif
