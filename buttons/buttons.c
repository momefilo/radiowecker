// buttons Bibliothek
#include "buttons.h"
#include "pico/stdlib.h"
#include "pico/printf.h"

/* Variablen zur Entprellung */
bool Button_pressed = false;
int WaitTime = 5;
int HoldTime = 200;//70;

void buttons_init(){
	gpio_init(BUTTON_R);
	gpio_set_pulls(BUTTON_R, 1, 0);
	gpio_init(BUTTON_L);
	gpio_set_pulls(BUTTON_L, 1, 0);
	gpio_init(BUTTON_U);
	gpio_set_pulls(BUTTON_U, 1, 0);
	gpio_init(BUTTON_D);
	gpio_set_pulls(BUTTON_D, 1, 0);
	gpio_init(BUTTON_M);
	gpio_set_pulls(BUTTON_M, 1, 0);
}

uint8_t get_Button(){
	uint8_t button = 100;
	if(Button_pressed){
		if(! gpio_get(BUTTON_U)) button = BUTTON_U;
		else if(! gpio_get(BUTTON_D)) button = BUTTON_D;
		else if(! gpio_get(BUTTON_L)) button = BUTTON_L;
		else if(! gpio_get(BUTTON_R)) button = BUTTON_R;
		else if(! gpio_get(BUTTON_M)) button = BUTTON_M;
		Button_pressed = false;
		sleep_ms(HoldTime);
	}
	else{
		Button_pressed = true;
		sleep_ms(WaitTime);
	}
	return button;
}
