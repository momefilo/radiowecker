// momefilo_flash Bibliothek
#include "momefilo_flash.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// die Anfangsadesse des Sektors
uint8_t *Flash_Content;

// offset zur berechnung von *Flash_Content
uint32_t Flash_Offset;

/*
 * Jeder Sektor ist in 16 Pages mit je 256 Byte unterteilt die einzeln
 * beschrieben, und nur der Sektor im ganzen gelöscht werden koennen.
 * Pagecount ist die aktuelle der 16 Pages welche das Data[]-Array beinhaltet
 * */
uint8_t Pagecount;

// Der zur Verfügung gestellte Speicherbereich
uint32_t Data[63];

/* Liest anhand Pagecount und Flash_content die Daten aus dem Flash
 * in das uint32_t Array*/
void get_Flash(){
	int addr;
	if(Pagecount < 1){
		addr = 15 * 256;
	}else{ addr = (Pagecount - 1) * 256;}
	uint8_t bufcount = 0;
	for(int i=0; i<63; i++){
		for(int k=0; k<4; k++){
			Data[i] |= (Flash_Content[addr + 2 + bufcount]) << (24 - 8*k);
			bufcount++;
		}
	}
}

/* Speichert die Anfangsadresse des Sektors in Flash_Content
 * Setzt den Pagecount
 * Ermittelt im Sktor vom Ende herab die aktuelle 256 byte umfassende
 * Page deren erste vier Byte 0xABBA als erkennungszeichen enthalten
 * die von unten beginnend genuntz sind.
 * */
void flash_init(uint8_t sektor){
	Flash_Offset = (PICO_FLASH_SIZE_BYTES - (sektor + 1) * FLASH_SECTOR_SIZE);
	Flash_Content = (uint8_t *) (XIP_BASE + Flash_Offset);
	for(uint8_t i=0; i<63; i++){ Data[i] = 0; }
	bool found = false;
	Pagecount = 15;

	for(int pc=Pagecount; pc>=0; pc--){
			uint16_t addr = pc * 256;
			if( (Flash_Content[addr] == 0xAB) && \
							(Flash_Content[addr+1] == 0xBA)){
				found = true;
				Pagecount = pc + 1;
				get_Flash();
				return;

		}
	}
	if(! found) Pagecount = 16;
}

/* Pueft anhand von Pagecount ob die letzte der 16 Pages erreicht ist
 * und loescht in diesem Falle den gesamten 4096Byte-Sektor bevor die
 * neue Page 0 geschrieben wird*/
void write_Flash(){
	uint32_t flags = save_and_disable_interrupts();
	uint8_t buf[FLASH_PAGE_SIZE];
	if(Pagecount > 15){
		flash_range_erase(Flash_Offset, FLASH_SECTOR_SIZE);
		Pagecount = 0;
	}
	buf[0] = 0xAB;
	buf[1] = 0xBA;
	uint8_t bufcount = 0;
	for(int i=0; i<63; i++){
		uint32_t mask = 0xFF000000;
		for(int k=0; k<4; k++){
			buf[2 + bufcount] = (Data[i] & mask) >> (24 - 8*k);
			mask = mask >> 8;
			bufcount++;
		}
	}
	flash_range_program(Flash_Offset + Pagecount * FLASH_PAGE_SIZE, buf, FLASH_PAGE_SIZE);
	restore_interrupts(flags);
	Pagecount++;
}

/* Gibt die Anfangsadresse des uint32_t Arrays im Flash zurueck*/
uint32_t *flash_getData(){
	return Data;
}

void flash_setData(uint8_t id, uint32_t data){
	Data[id] = data;
	write_Flash();
}

void flash_setDataRow(uint8_t start, uint8_t end, uint32_t *data){
	for(uint8_t	i=start; i<=end; i++){
		 Data[i] = data[i - start];
	}
	write_Flash();
}

