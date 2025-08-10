/* momefilo_flash
 * verwaltet den Flash des Pico und stellt je 4kb Sektor
 * 63 uint32_t Speicherbereiche in einem Array bereit,
 * die komfortabel mit "flash_getData()" gelesen und "flash_setData()"
 * sowie "flash_setDataRow()" bescheieben werden können.
 * momefilo_flash verwaltet den Flash vom Adressende (sektor = 0)herab so,
 * das erst nach 16 Änderungen im zur verfügung gestellten uint32_t Array der
 * Sektor gelöscht wird und auch der Programmcode wachsen kann ohne das
 * dem feste Grenzen gesetzt sind
 **/

#ifndef my_momefilo_flash_h
#define my_momefilo_flash_h 1
#include "pico/stdlib.h"

/* Initalisiert den Sektor sektor
 * sektor >= 0 und sektor < (Flashgroesse-256kb)/4kb
 * und stellt die darin gespeicherten 63 uint32_t Werte im Array bereit*/
void flash_init(uint8_t sektor);

/* Gibt das Array des Flashspeichers zuruek*/
uint32_t *flash_getData();

 /* Speichert data an der Stelle id des uint32_t Arrays im Flash
  * id >=0 und <63
  * data: der zu speichernde uint32_t-Wert*/
void flash_setData(uint8_t id, uint32_t data);

/* Speichert das Array *data beginnend von
 * start >=0 und <62 bis
 * ende >=1 und <63 im uint32_t Array des Flash
 * Diese Funktion "verbraucht" nur eine Page und sollte immer benutzt werden
 * wenn mehrere uint32_t-Werte auf einmal gespeichert werden koennen*/
void flash_setDataRow(uint8_t start, uint8_t end, uint32_t *data);

#endif
