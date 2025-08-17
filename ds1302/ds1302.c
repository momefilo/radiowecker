// ds1302 Bibliothek
#include "ds1302.h"

#define BURSDT_READ 0xBF
#define BURST_WRITE 0xBE

/* Only for debug
#include "pico/printf.h"
const static char Tage[7][3] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const static char Monate[12][4] = {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", \
	 "jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
void printDatum(datetime_t datum){
	printf("%s %d %s %d %d:%d:%d\n", \
		Tage[datum.dotw - 1], datum.day, \
		Monate[datum.month - 1], datum.year, \
		datum.hour, datum.min, datum.sec);
}*/

/* Variabeln
*/
uint8_t Pin_scl, Pin_io, Pin_cs;
datetime_t aktDate;

/* Helfer-Funktionen
*/
uint8_t bcdToDec(uint8_t n){ return n - 6 * (n >> 4); }
uint8_t decToBcd(uint8_t n){ return ( ((n/10) << 4) | (n%10) ); }

void startBus(){
	gpio_put(Pin_scl, 0);
	gpio_put(Pin_cs, 1);
	busy_wait_us(2);
}
void stopBus(){
	gpio_put(Pin_cs, 0);
	busy_wait_us(2);
}

/* Schreib/lese- Funktionen
*/
void writeOut(uint8_t value, bool readAfter) {

	gpio_set_dir(Pin_io, true);// dir out
	for (int i = 0; i < 8; ++i){
		gpio_put(Pin_io, (value >> i) & 1);
		busy_wait_us(1);
		gpio_put(Pin_scl, 1);
		busy_wait_us(1);

		if (readAfter && i == 7) {
			// We're about to read data -- ensure the pin is back in input mode
			// before the clock is lowered.
			gpio_set_dir(Pin_io, false);// dir in
		}else{
			gpio_put(Pin_scl, 0);
			busy_wait_us(1);
		}
	}
}
uint8_t readIn() {
	uint8_t input_value = 0;
	uint8_t bit = 0;
	gpio_set_dir(Pin_io, false);// dir in

	// Bits from the DS1302 are output on the falling edge of the clock
	// cycle. This is called after readIn (which will leave the clock low) or
	// writeOut(..., true) (which will leave it high).
	for (int i = 0; i < 8; ++i) {
		gpio_put(Pin_scl, 1);
		busy_wait_us(1);
		gpio_put(Pin_scl, 0);
		busy_wait_us(1);

		bit = gpio_get(Pin_io);
		input_value |= (bit << i);  // Bits are read LSB first.
	}
	return input_value;
}

/* Getter/Setter-Funktionen
*/
datetime_t ds1302_getDate(){
	startBus();
	writeOut(BURSDT_READ, true);
	aktDate.sec = bcdToDec(readIn() & 0x7F);
	aktDate.min = bcdToDec(readIn());
	aktDate.hour = bcdToDec(readIn());
	aktDate.day = bcdToDec(readIn());
	aktDate.month = bcdToDec(readIn());
	aktDate.dotw = bcdToDec(readIn());
	aktDate.year = 2000 + bcdToDec(readIn());
	stopBus();
	//printDatum(aktDate);// Only for debug
	return aktDate;
}
void ds1302_setDate(datetime_t datum){

	startBus();
	// disable Write-Protection
	uint8_t cmd_byte = (0x80 | (7 << 1));
	writeOut(cmd_byte,false);
	writeOut(0, false);
	// write the date
	writeOut(BURST_WRITE, false);
	writeOut(decToBcd(datum.sec), false);
	writeOut(decToBcd(datum.min), false);
	writeOut(decToBcd(datum.hour), false);
	writeOut(decToBcd(datum.day), false);
	writeOut(decToBcd(datum.month), false);
	writeOut(decToBcd(datum.dotw), false);
	writeOut(decToBcd(datum.year - 2000), false);
	// All clock registers *and* the WP register have to be written for the time
	// to be set.
	writeOut(0x80, false);  // Write protection register.
	stopBus();
	//printDatum(datum);// Only for debug
}

/* Initialisierung
*/
void ds1302_init(uint8_t scl, uint8_t io, uint8_t cs){
	Pin_scl = scl; Pin_io = io; Pin_cs = cs;
	gpio_init(scl); gpio_init(io); gpio_init(cs);
	gpio_set_dir(scl, true);// dir out
	gpio_set_dir(io, true);// dir out
	gpio_set_dir(cs, true);// dir out
	gpio_put(scl, 0);
	gpio_put(io, 0);
	gpio_put(cs, 0);
}
