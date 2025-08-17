// Analoguhr by momefilo mit Radiowecker
#include "uhr.h"

/* for zero
#define FM_i2c 1
#define FM_SDA 26
#define FM_SCL 27
#define FM_RES 28
#define FM_GP2 29

#define RTC_SCL 3
#define RTC_IO 4
#define RTC_CS 5

#define PIN_RADIO 2
*/
// for org
#define FM_i2c 0
#define FM_SDA 16
#define FM_SCL 17
#define FM_RES 18
#define FM_GP2 19

#define RTC_SCL 21
#define RTC_IO 2
#define RTC_CS 3

#define PIN_RADIO 20


volatile bool RadioON = false;
void radioMenu();

/* Die Uhr
 * * display spi_bus spi_1, kabel black1 und yellow2 = +3.3v, red1 = GND
 * * si4703 i2c bus i2c_0 / for zero i2c_0
 *
 * PIN_RADIO_ON GP_20 / for zero GP_2 für Verstärker-Spannung Ein/Aus
 * RTC SCL GP_21 / for zero GP_3, kabel2pol black
 * RTC IO GP_2 / for zero GP_4, kabel2pol black
 * RTC CS GP_3 / for zero GP_5, kabel2pol red
 * BUTTON_M GP_6, kabel2pol black
 * BUTTONS_COM, kabel2pol red
 * display DC GP_7, kabel black2
 * display reset GP_8 kabel gelb1
 * display cs/csn GP_9, kabel weis1
 * display SCK/SPI_SCK GP_10, kabel weis2
 * display SDIO/SPI_TX GP_11, kabel red2
 * BUTTON_U GP_12, kabel black
 * BUTTON_D GP_13, kabel red
 * BUTTON_L GP_14, kabel weis
 * BUTTON_R GP_15, kabel yellow
 * si4703 SDA GP_16 / for zero GP_26, kabel black
 * si4703 SCL GP_17 / for zero GP_27, kabel red
 * si4703 RES GP_18 / for zero GP_28, kabel weis
 * si4703 GP2 GP_19 / for zero GP_29, kabel yellow
 * */
// Uhr-Variablen
uint16_t Center_x, Center_y, MinZeiger_len, StdZeiger_len, UhrWidth;// Zeigerdarstellung
uint16_t Colors[] = {  0x47FF, 0x7FF0, 0xF81F };// Farbmodi
const static char Tage[7][3] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const static char Monate[12][4] = {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
struct repeating_timer Timer;// wird jede 1/6 Sekunde auferufen
uint16_t OldMinLen, OldStdLen, SekGrad;// zur Zeigerdarstellung
uint8_t OldMin = 0, OldStd = 0, SekCnt=0;// zur Zeigerdarstellung
int16_t **OldStdVec, **OldMinVec, SekVec[360][2], **Vektor;// zur Zeigerdarstellung
datetime_t AktDate, UserDate;// AktDate wird jede 1/6 Sekunde aktualisiert, Zeit aendern variable
bool AlarmAktiv = false, IsFlip = false, AlarmMenu = false, ColorInv = false;// Alarm an/aus, Display Hori.spiegeln, Alarm/Zeit einstellen
uint8_t AlarmHour = 1, AlarmMin = 0, MinutCount = 0, UhrSel = 0, ColorIndex = 0;// Alarmvariablen, ds1302-synchron, Uhrmenue, Falbmodi
// Zeigerberechnung
int16_t getVektor(uint16_t grad, uint16_t begin, uint16_t end){
	int8_t fak = 1; if(grad > 179){ grad -= 180; fak = -1;}
	float_t sinus = sin(grad * (2 * 3.14) / 360);
	float_t cosin = cos(grad * (2 * 3.14) / 360);
	int cnt = 0;
	for(int i=begin; i<end; i++){
		if(grad == 90){
			Vektor[cnt][0] = fak * i;
			Vektor[cnt][1] = 0;
			cnt++;
		}
		else if(grad == 0){
			Vektor[cnt][0] = 0;
			Vektor[cnt][1] = fak * -i;
			cnt++;
		}
		else{
			float_t cos_ = -i * cosin;
			float_t sin_ = i * sinus;
			int16_t y = cos_*fak, x = sin_*fak;
			if(i < begin + 1){
				Vektor[cnt][0] = x;
				Vektor[cnt][1] = y;
				cnt++;
			}
			else if(i > begin && ((Vektor[cnt-1][0] != x) || (Vektor[cnt-1][1] != y))){
				Vektor[cnt][0] = x;
				Vektor[cnt][1] = y;
				cnt++;
			}
		}
	}
	return cnt;
}

// Uhr-Malfunktionen
void clearSekunde(){
	int len = SekZeiger_len;
	uint16_t grad = SekGrad;
	if(grad < len) grad = 360 + grad;
	uint16_t startx = SekVec[grad-len][0] + Center_x;
	uint16_t endx = startx;
	uint16_t starty = SekVec[grad-len][1] + Center_y;
	uint16_t endy = starty;
	if(((grad-len) > 45 && (grad-len) < 135) || ((grad-len) > 225 && (grad-len) < 315)){
		endx+=(SekZeiger_width-0); startx-=(SekZeiger_width+0);
		if((grad-len) < 135){ endy++;starty--; }
		else{ starty-=2; endy+=2; }
	}
	else{
		endy+=(SekZeiger_width+0);
		starty-=(SekZeiger_width+0);
		startx--; endx++;}
	uint16_t area[] = { startx, starty, endx, endy};
	paintRect(area, ClrColor);
}
void clearZeiger(){
	// Minuten
	uint8_t mid  = (MinZeiger_width - 1) / 2;
	for(int i=0; i<OldMinLen; i++){
		uint16_t startx = OldMinVec[i][0] + Center_x;
		uint16_t  endx = startx;
		uint16_t starty = OldMinVec[i][1] + Center_y;
		uint16_t endy = starty;
		if((OldMin > 7 && OldMin < 23) || (OldMin > 37 && OldMin < 53)){
			starty = OldMinVec[i][1] + Center_y - MinZeiger_width + mid;
			endy = OldMinVec[i][1] + Center_y + MinZeiger_width - mid;
		}
		else{
			startx = OldMinVec[i][0] + Center_x - MinZeiger_width + mid;
			endx = OldMinVec[i][0] + Center_x + MinZeiger_width - mid;
		}
		uint16_t area[] = { startx, starty, endx, endy};
		paintRect(area, ClrColor);
	}
	// Stunden
	mid  = (StdZeiger_width - 1) / 2;
	for(int i=0; i<OldStdLen; i++){
		uint16_t startx = OldStdVec[i][0] + Center_x;
		uint16_t endx = startx;
		uint16_t starty = OldStdVec[i][1] + Center_y;
		uint16_t endy = starty;
		if((OldStd > 1 && OldStd < 5) || (OldStd > 7 && OldStd < 11)){
			starty = OldStdVec[i][1] + Center_y - StdZeiger_width + mid;
			endy = OldStdVec[i][1] + Center_y + StdZeiger_width - mid;
		}
		else{
			startx = OldStdVec[i][0] + Center_x - StdZeiger_width + mid;
			endx = OldStdVec[i][0] + Center_x + StdZeiger_width - mid;
		}
		uint16_t area[] = { startx, starty, endx, endy};
		paintRect(area, ClrColor);
	}
}
void paintBlatt(){
	uint16_t sx = Center_x - MinZeiger_len - Abstand;
	uint16_t ex = Center_x + MinZeiger_len + Abstand;
	uint16_t sy = Center_y - MinZeiger_len - Abstand;
	uint16_t ey = Center_y + MinZeiger_len + Abstand;
	uint16_t area[] = {sx, sy, ex, ey};
	paintRect(area, ClrColor);// alten Zeiger löschen
	for(int i=0; i<60; i += 5){
		uint16_t len = getVektor(i * MIN_GRAD, MinZeiger_len + Abstand, MinZeiger_len + Abstand + ZifferLen);
		if(UhrWidth < 100){// paint Striche
			for(int p=0; p<len; p++){
				uint16_t startx = Vektor[p][0] + Center_x;
				uint16_t starty = Vektor[p][1] + Center_y;
				uint16_t endy = starty;
				uint16_t endx = startx;
				if((i > 7 && i < 23) || (i > 37 && i < 53)){ endy+=ZifferWidth; starty-=ZifferWidth; }
				else if((i > 22 && i < 38) || (i > 52 || i < 8)){ endx+=ZifferWidth; startx-=ZifferWidth; }
				uint16_t area[] = {startx, starty, endx, endy};
				paintRect(area, Colors[ColorIndex]);
			}
		}
		else{// paint Zahl
			uint8_t ziffer_ofset = 0;
			if(AktDate.hour > 11) ziffer_ofset = 12;
			uint8_t height = 7, width = 10;
			if(UhrWidth > 149){ height = 11; width = 15; }
			uint16_t sx = Vektor[0][0] + Center_x, sy = Vektor[0][1] + Center_y;
			if(i == 0){ sy-= (height + 2); sx -= width / 2; }
			else if(i == 5){ sy -= height; }
			else if(i == 10){ sy -= (height - 2); }
			else if(i == 15){  sy -= (height / 2 + 1); sx += 1; }
			else if(i == 25){  sy += 3; sx -= 2; }
			else if(i == 30){  sx -= width / 2; sy += 2;}
			else if(i > 30)  sx -= width;
			if(i == 45){ sy -= height / 2; sx -= 1; }
			else if(i == 50)  sy -= height;
			else if(i == 55){ sy -= (height +1); sx += 1; }
			uint16_t ex = sx + width - 1, ey = sy + height - 1;
			uint16_t area[] = { sx, sy, ex, ey};
			if(UhrWidth < 150){
				drawRect(area, ZIFFER[i / 5 + ziffer_ofset][ColorIndex]);
			}
			else{
				drawRect(area, ZIFFER_B[i / 5 + ziffer_ofset][ColorIndex]);
			}
		}
	}
	//Datum
	char text[16];
	uint16_t pos[] = {319 - 10 * 9, 5};
	uint16_t color = Colors[ColorIndex];
	if(ColorIndex == 2) color = Colors[0];
	setFgColor(color);
	if(UhrWidth > 219){
		if(AktDate.day < 10) pos[0] = (319 - 10 * 7);
		sprintf(text, "%s %d %s", Tage[AktDate.dotw - 1], AktDate.day, Monate[AktDate.month - 1]);
		if(AktDate.day < 10) pos[0] -= 10;
		writeText10x16(pos, text, false, false);
		sprintf(text, "%d", AktDate.year);
		pos[1] += 20;
		pos[0] = (319 - 10 * 4);
		writeText10x16(pos, text, false, false);
		if(AlarmAktiv){
			sprintf(text, "AL");
			pos[1] += 25;
			pos[0] += 16;
			writeText10x16(pos, text, true, false);
		}
		uint16_t area[] = {239, 170, 318, 211};
		drawRect(area, BILD);
	}
	else{
		sprintf(text, "%s %d %s %d", Tage[AktDate.dotw -1], AktDate.day, Monate[AktDate.month - 1], AktDate.year);
		pos[0] = (319 - 10 * 14);
		if(AktDate.day < 10) pos[0] += 1;
		writeText10x16(pos, text, false, false);
	}
}
void paintZeiger(datetime_t now){
	uint8_t std = now.hour, min = now.min;
	clearZeiger();
	// Minuten
	uint16_t len = getVektor(min * MIN_GRAD, 0, MinZeiger_len);
	OldMin = min; OldMinLen = len;
	uint8_t mid  = (MinZeiger_width - 1) / 2;
	for(int i=0; i<len; i++){
		OldMinVec[i][0] = Vektor[i][0]; OldMinVec[i][1] = Vektor[i][1];
		uint16_t startx = Vektor[i][0] + Center_x;
		uint16_t endx = startx;
		uint16_t starty = Vektor[i][1] + Center_y;
		uint16_t endy = starty;
		if((min > 7 && min < 23) || (min > 37 && min < 53)){
			starty = Vektor[i][1] + Center_y - MinZeiger_width + mid;
			endy = Vektor[i][1] + Center_y + MinZeiger_width - mid;
		}
		else{
			startx = Vektor[i][0] + Center_x - MinZeiger_width + mid;
			endx = Vektor[i][0] + Center_x + MinZeiger_width - mid;
		}
		uint16_t area[] = { startx, starty, endx, endy};
		paintRect(area, Colors[ColorIndex]);
	}
	// Stunden
	uint16_t grad = std * STD_GRAD + min * MIN_GRAD / 12;
	len = getVektor(grad, 0, StdZeiger_len);
	mid  = (StdZeiger_width - 1) / 2;
	if(std > 11) std -= 12;
	OldStd = std; OldStdLen = len;
	for(int i=0; i<len; i++){
		OldStdVec[i][0] = Vektor[i][0]; OldStdVec[i][1] = Vektor[i][1];
		uint16_t startx = Vektor[i][0] + Center_x;
		uint16_t endx = startx;
		uint16_t starty = Vektor[i][1] + Center_y;
		uint16_t endy = starty;
		if((std > 1 && std < 5) || (std > 7 && std < 11)){
			starty = Vektor[i][1] + Center_y - StdZeiger_width + mid;
			endy = Vektor[i][1] + Center_y + StdZeiger_width - mid;
		}
		else{
			startx = Vektor[i][0] + Center_x - StdZeiger_width + mid;
			endx = Vektor[i][0] + Center_x + StdZeiger_width - mid;
		}
		uint16_t area[] = { startx, starty, endx, endy};
		paintRect(area, Colors[ColorIndex]);
	}
}
void paintSekunde(uint16_t grad){
	uint16_t startx = SekVec[grad][0] + Center_x;
	uint16_t endx = startx;
	uint16_t starty = SekVec[grad][1] + Center_y;
	uint16_t endy = starty;
	if((grad > 45 && grad < 135) || (grad > 225 && grad < 315)){
		endx+=SekZeiger_width; startx-=SekZeiger_width;
	}
	else{ endy+=SekZeiger_width; starty-=SekZeiger_width;}
	uint16_t area[] = { startx, starty, endx, endy};
	uint16_t color = Colors[1];
	if(ColorIndex == 1) color = Colors[0];
	else if(ColorIndex == 2) color = Colors[1];
	paintRect(area, color);
}

// Uhl-Logikfunktionen
bool timer_callback(struct repeating_timer *t) {// wird jede 1/6 Sekunde aufgerufen
	//if(SekGrad % 6 == 0) busy_wait_ms(4);// 6 * 166ms = 1s - 4ms
	SekGrad++;
	if(SekGrad == 360){
		MinutCount++;
		if(MinutCount > 29){
			AktDate = ds1302_getDate();
			rtc_set_datetime(&AktDate);
			MinutCount = 0;
		}
		else rtc_get_datetime(&AktDate);
		paintBlatt();
		paintZeiger(AktDate);
		SekGrad = (AktDate.sec) * 6;
	}
	clearSekunde();
	paintSekunde(SekGrad);
    return true;
}
void ds_init(){

    AktDate = ds1302_getDate();
    SekGrad = AktDate.sec * 6;
    rtc_init();
    rtc_set_datetime(&AktDate);
    add_repeating_timer_us(-166666, timer_callback, NULL, &Timer);
    paintBlatt();
    paintZeiger(AktDate);
}
void allocMemory(){
	OldMinVec = (int16_t **)malloc((MinZeiger_len) * sizeof(int16_t*));
	for(int i=0; i<(MinZeiger_len); i++) OldMinVec[i] = (int16_t *)malloc(2 * sizeof(int16_t));
	OldStdVec = (int16_t **)malloc((StdZeiger_len) * sizeof(int16_t*));
	for(int i=0; i<(StdZeiger_len); i++) OldStdVec[i] = (int16_t *)malloc(2 * sizeof(int16_t));
	Vektor = (int16_t**)malloc(MinZeiger_len * sizeof(int16_t*));
	for(int i=0; i<(MinZeiger_len); i++) Vektor[i] = (int16_t*)malloc(sizeof(int16_t));
	int grad;
	int len = MinZeiger_len + 2;
	for(int i=0; i<360; i++){
		getVektor(i, len, len+1);
		SekVec[i][0] = Vektor[0][0];
		SekVec[i][1] = Vektor[0][1];
	}
}
void uhr_deinit(){
	cancel_repeating_timer(&Timer);
	OldMinLen = 0; OldStdLen = 0; SekGrad = 0;
	OldMin = 0; OldStd = 0; SekCnt=0;
	free(OldMinVec);
	free(OldStdVec);
	free(Vektor);
	clearScreen();
	busy_wait_ms(100);
}
void uhr_init(uint16_t x, uint16_t y, uint16_t width, uint8_t color){
	ColorIndex = color;
	UhrWidth = width;
	if(UhrWidth < 50) UhrWidth = 50;
	Center_x = x + UhrWidth / 2;
	Center_y = y + UhrWidth / 2;
	if(UhrWidth > 99){
		ZifferLen = 11;
		SekZeiger_width = 2;
		if(UhrWidth > 124){
			StdZeiger_width = 2;
			ZeigerDiff = 15;
			if(UhrWidth > 149){
				ZifferLen = 16;
				if(UhrWidth > 179){
					ZeigerDiff = 20;
				}
				else if(UhrWidth > 199){
					ZeigerDiff = 25;
				}
			}
		}
	}
	MinZeiger_len = UhrWidth / 2 - Abstand - ZifferLen;
	StdZeiger_len = MinZeiger_len - ZeigerDiff;
	allocMemory();
	setBgColor(ClrColor);
	setSeColor(0x07E0);
	ds_init();
}

// Uhr-Menu (Weckzeit / Uhrzeit stellen)
//uint8_t UhrSel = 0;
void paintUhrMenu(){
	setFgColor(0xFFFF);
	char text[7];
	uint16_t pos[2];
	pos[1] = 80;
	if(!AlarmMenu){
		// Wochentag
		sprintf(text, "%s ", Tage[UserDate.dotw - 1]);
		bool sel = false; if(UhrSel == 5) sel = true;
		pos[0] = (319 - 12 * 12);
		writeText10x16(pos, text, sel, false);
		// Tag
		sprintf(text, "%d ", UserDate.day);
		sel = false; if(UhrSel == 0) sel = true;
		pos[0] = (319 - 10 * 12);
		if(UserDate.day < 10) sprintf(text, " %d ", UserDate.day);
		writeText10x16(pos, text, sel, false);
		// Monat
		sprintf(text, "%s ", Monate[UserDate.month - 1]);
		sel = false; if(UhrSel == 1) sel = true;
		pos[0] += (10 * 3);
		writeText10x16(pos, text, sel, false);
		// Jahr
		sprintf(text, "%d ", UserDate.year);
		sel = false; if(UhrSel == 2) sel = true;
		pos[0] = (319 - 10 * 5);
		writeText10x16(pos, text, sel, false);
		// Stunde
		pos[1] += 22;
		sprintf(text, "%d:", UserDate.hour);
		sel = false; if(UhrSel == 3) sel = true;
		pos[0] = (319 - 10 * 10);
		if(UserDate.hour < 10) sprintf(text, "0%d:", UserDate.hour);
		writeText10x16(pos, text, sel, false);
		// Minute
		sprintf(text, "%d", UserDate.min);
		sel = false; if(UhrSel == 4) sel = true;
		pos[0] = (319 - 10 * 7);
		if(UserDate.min < 10) sprintf(text, "0%d:", UserDate.min);
		writeText10x16(pos, text, sel, false);
	}
	else{// Alarm
		uint8_t al = AlarmHour;
		pos[1] += 22;
		pos[0] = (319 - 10 * 16);
		sprintf(text, "Alarm ");
		writeText10x16(pos, text, false, false);
		sprintf(text, "%d:", al);
		bool sel = false; if(UhrSel == 6) sel = true;
		pos[0] = (319 - 10 * 10);
		if(al < 10) sprintf(text, "0%d:", al);
		writeText10x16(pos, text, sel, false);
		// Alarm Minute
		al = AlarmMin;
		sprintf(text, "%d", al);
		sel = false; if(UhrSel == 7) sel = true;
		pos[0] = (319 - 10 * 7);
		if(al < 10) sprintf(text, "0%d", al);
		writeText10x16(pos, text, sel, false);
		// Alarm An/Aus
		sprintf(text, "An ");
		sel = false; if(UhrSel == 8) sel = true;
		pos[0] = (319 - 10 * 4);
		if(! AlarmAktiv) sprintf(text, "%s", "Aus");
		writeText10x16(pos, text, sel, false);
	}

}
void uhrChnSel(bool up){
	if(UhrSel == 0){
		int8_t wert = UserDate.day;
		if(up) wert++; else wert--;
		if(wert > 31) wert = 1;
		else if(wert < 1) wert = 31;
		UserDate.day = wert;
	}
	else if(UhrSel == 1){
		int8_t wert = UserDate.month;
		if(up) wert++; else wert--;
		if(wert > 12) wert = 1;
		else if(wert < 1) wert = 12;
		UserDate.month = wert;
	}
	else if(UhrSel == 2){
		int16_t wert = UserDate.year;
		if(up) wert++; else wert--;
		if(wert < 0) wert = 2025;
		UserDate.year = wert;
	}
	else if(UhrSel == 3){
		int8_t wert = UserDate.hour;
		if(up) wert++; else wert--;
		if(wert > 23) wert = 0;
		else if(wert < 0) wert = 23;
		UserDate.hour = wert;
	}
	else if(UhrSel == 4){
		int8_t wert = UserDate.min;
		if(up) wert++; else wert--;
		if(wert > 59) wert = 0;
		else if(wert < 0) wert = 59;
		UserDate.min = wert;
	}
	else if(UhrSel == 5){
		int8_t wert = UserDate.dotw;
		if(up) wert++; else wert--;
		if(wert > 7) wert = 1;
		else if(wert < 1) wert = 7;
		UserDate.dotw = wert;
	}
	else if(UhrSel == 6){// Alarm Stunden
		int8_t wert = AlarmHour;
		if(up) wert++; else wert--;
		if(wert > 23) wert = 0;
		else if(wert < 0) wert = 23;
		AlarmHour = wert;
	}
	else if(UhrSel == 7){// Alarm Minuten
		int8_t wert = AlarmMin;
		if(up) wert++; else wert--;
		if(wert > 59) wert = 0;
		else if(wert < 0) wert = 59;
		AlarmMin = wert;
	}
	else if(UhrSel == 8){// Alarm An/Aus
		AlarmAktiv = ! AlarmAktiv;
	}
	paintUhrMenu();
}
void uhrMenu(bool alarm){
	uint8_t sel_min = 0, sel_max = 5; AlarmMenu = false;
	if(alarm) { sel_min = 6; sel_max = 8; AlarmMenu = true;}
	UhrSel = sel_min;
	uhr_deinit();
	uhr_init(0, 0, 150, ColorIndex);
	UserDate = ds1302_getDate();
	UserDate.sec = 1;
	paintUhrMenu();
	while (true) {
		uint8_t button = get_Button();
		if(button < 100){
			if(button == BUTTON_U){ uhrChnSel(true); }
			else if(button == BUTTON_D){ uhrChnSel(false);  }
			else if(button == BUTTON_L){
				if(UhrSel > sel_min) UhrSel--; else UhrSel = sel_max;
				paintUhrMenu();
			}
			else if(button == BUTTON_R){
				if(UhrSel < sel_max) UhrSel++; else UhrSel = sel_min;
				paintUhrMenu();
			}
			else if(button == BUTTON_M){
				if(AlarmMenu) {}
				else{ds1302_setDate(UserDate); }
				break;
			}
		}
		busy_wait_ms(10);
	}
	uhr_deinit();
	uhr_init(0, 0, 240, ColorIndex);
}

/* Das Radio
 * Die calback-Funktionen zeichnen Tune, Sendernamen und RDS-Text
*/
uint8_t SeekQuali = 3;
double_t MHz = 95.90;
void paintQuali(){
	char text[7];
	sprintf(text, "EMPF %d", SeekQuali + 1);
	uint16_t pos[] = {319 - (7 * 7 + 0), 114};
	setSeColor(0xFFE0);
	writeText7x11(pos, text, true, false);
}
void paint_rssi(uint8_t rssi){
	uint8_t max = 75;
	uint8_t top = 78;// Startzeile
	uint16_t left = 319 - 120;
	uint16_t area0[] = {left, top, left+114, top+16};
	paintRect(area0, 0x0000);
	area0[3] = top;
	//paintRect(area0, 0xFFE0);
	area0[2] = left; area0[3] = top+16;
	//paintRect(area0, 0xFFE0);
	area0[2] = left+114; area0[1] = top+16;
	//paintRect(area0, 0xFFE0);
	area0[0] = left+114; area0[1] = top;
	//paintRect(area0, 0xFFE0);
	uint16_t area[] = {left+1, top+5, left+1, top+10}, color = 0x0000;
	float_t fak = rssi/3 * 4;
	uint16_t red = 0x3F;
	uint16_t green = 0x3F;
	color = (red << 12) | (green << 6);
	for(int i=0; i<fak; i++){
		paintRect(area, color);
		area[0] += 1;
		area[2] += 1;
		if(i > 5 && red > 0 && i % 2 == 0) red--;
		color = (red << 11) | (green << 5);
	}
}
void cbPS(char *name, uint8_t flags){
// Flags: 1=most, 2=arti, 3=comp, 4=dpty, 5=M/S, 6=ta, 7=tp
	if(RadioON){
		uint16_t pos[] = {318 - (8 * 14 + 11), 146};
		writeText14x20(pos, name, false, false);
		char text[9];
		sprintf(text,"Sprache");
		if(flags & 0x10)sprintf(text,"Musik  ");
		pos[1] = 68;
		pos[0] = 239;
		setSeColor(0x07E0);
		writeText7x11(pos, text, true, false);
		pos[0] += 51;
		sprintf(text,"  ");
		if(flags & 0x20) sprintf(text,"TA");
		writeText7x11(pos, text, true, false);
		sprintf(text,"  ");
		if(flags & 0x40) sprintf(text,"TP");
		pos[0] += 14;
		writeText7x11(pos, text, true, false);
	}
}
void cbRDS(char *text, uint8_t pty){
	if(RadioON){
		char text1[33], text2[33];
		for(int i=0; i<32; i++){
			if(text[i] < 128) text1[i] = text[i];
			else text1[i] = 32;
			if(text[i+32] < 128) text2[i] = text[i+32];
			else text2[i] = 32;
		}
		uint16_t pos[] = {0, 239 - 50};
		setSeColor(0xFFFF);
		writeText10x16(pos, text1, true, false);
		pos[1] += 18;
		writeText10x16(pos, text2, true, false);
	}
}
void cbTune(double_t mhz, uint8_t rssi, uint8_t flags){
// Flags: 1=Stereo, 2= RDSS, 3=AFCRL, 4=SF/BL, 5=STC, 6=RDSR
	if(RadioON){
		char freq[12];
		sprintf(freq, "%.2f MHz", mhz);
		if(mhz < 100) sprintf(freq, " %.2f MHz", mhz);
		uint16_t pos[] = {319 - 126, 95};
		setSeColor(0xFFE0);
		writeText12x16(pos, freq, true, false);
		paint_rssi(rssi);
		pos[0] = 319 - 119 -4;
		pos[1] = 69;

		sprintf(freq, "  ");
		if(flags & 0x01) sprintf(freq, "ST");
		setSeColor(0x07E0);
		writeText7x11(pos, freq, true, false);

		pos[0] += 18;
		sprintf(freq, "   ");
		if(flags & 0x20) sprintf(freq, "RDS");
		writeText7x11(pos, freq, true, false);

		pos[0] -= 18;
		pos[1] -= 13;
		sprintf(freq, "     ");
		if(flags & 0x08) sprintf(freq, "SF/BL");
		setSeColor(0xF800);
		writeText7x11(pos, freq, true, false);

		pos[0] += 39;
		sprintf(freq, "   ");
		if(flags & 0x04) sprintf(freq, "AFC");
		writeText7x11(pos, freq, true, false);

		MHz = mhz;
	}
}
void clearRDS(){
	char ps[9];
	char txt[68];
	for(int i=0; i<68; i++){
		if(i<9) ps[i] = 32;
		txt[i] = 32;
	}
	cbPS(ps, 0);
	cbRDS(txt, 0);
}
void initFlash(){
	flash_init(0);
	uint32_t *flashdata = flash_getData();
	if(flashdata[0] != 0xBAAB){
		flashdata[0] = 0xBAAB;
		flashdata[1] = ColorIndex;
		flashdata[2] = MHz;
		flashdata[3] = AlarmAktiv;
		flashdata[4] = AlarmHour;
		flashdata[5] = AlarmMin;
		flashdata[6] = SeekQuali;
		flashdata[7] = 0;// Kommastellen von MHz
		flashdata[8] = 0;// Display-IsFlip
		flashdata[9] = 0;// ColorInv
		flash_setDataRow(0, 9, flashdata);
	}else{

		ColorIndex = flashdata[1];
		MHz = flashdata[2] + flashdata[7] / 100.0;
		AlarmAktiv = flashdata[3];
		AlarmHour = flashdata[4];
		AlarmMin = flashdata[5];
		SeekQuali = flashdata[6];
		IsFlip = flashdata[8];
		ColorInv = flashdata[9];
	}
}
void saveFlash(){
	uint32_t mhz_int = MHz;
	double_t tmp = MHz - mhz_int;
	uint32_t mhz_frac = tmp * 100;
	uint32_t flashdata[9];
	flashdata[0] = ColorIndex;
	flashdata[1] = mhz_int;
	flashdata[2] = AlarmAktiv;
	flashdata[3] = AlarmHour;
	flashdata[4] = AlarmMin;
	flashdata[5] = SeekQuali;
	flashdata[6] = mhz_frac;
	flashdata[7] = IsFlip;
	flashdata[8] = ColorInv;
	flash_setDataRow(1, 9, flashdata);
}
void radioMenu(){
	RadioON = true;
	uhr_deinit();
	uhr_init(0, 0, 170, ColorIndex);
	gpio_put(PIN_RADIO, 1);
	si4703_powerUp();
	busy_wait_ms(110);
	si4703_setSeekQuality(SeekQuali);
	paintQuali();
	si4703_tuning(MHz);
	while(true){
		uint8_t button = get_Button();
		if(button == BUTTON_L) break;
		else if(button == BUTTON_U){ si4703_seeking(true); clearRDS(); }
		else if(button == BUTTON_D){ si4703_seeking(false); clearRDS(); }
		else if(button == BUTTON_R){
			SeekQuali++;
			if(SeekQuali > 3) SeekQuali = 0;
			si4703_setSeekQuality(SeekQuali);
			paintQuali();
		}
		busy_wait_ms(10);
	}
	si4703_powerDown();
	gpio_put(PIN_RADIO, 0);
	RadioON = false;
	uhr_deinit();
	clearScreen();
	saveFlash();
	uhr_init(0, 0, 240, ColorIndex);
}

int main() {
	stdio_init_all();
	busy_wait_ms(50);
	gpio_init(PIN_RADIO);
	gpio_set_dir(PIN_RADIO, GPIO_OUT);
	gpio_put(PIN_RADIO, 0);
	initFlash();
	buttons_init();
	ili9341_init();
	setOrientation(HORIZONTAL);
	flipDisplay(IsFlip);
	setInvert(ColorInv);
	ds1302_init(RTC_SCL, RTC_IO, RTC_CS);
	uhr_init(0, 0, 240, ColorIndex);
	si4703_init(FM_i2c, FM_SDA, FM_SCL, FM_RES, FM_GP2, false);
	si4703_germanDefault_setup(cbPS, cbRDS, cbTune);
	si4703_volume(0x0F);
	si4703_powerDown();
	busy_wait_ms(10);
	//radioMenu();
	while (true) {
		uint8_t button = get_Button();
		if(button < 100){
			if(button == BUTTON_U){ uhrMenu(true); saveFlash(); }
			else if(button == BUTTON_D){ uhrMenu(false);}
			else if(button == BUTTON_L){
				if(ColorIndex > 0) ColorIndex--;
				else{
					ColorIndex = 2;
					ColorInv = !ColorInv;
					setInvert(ColorInv);
				}
				paintBlatt();
				paintZeiger(AktDate);
				saveFlash();
			}
			else if(button == BUTTON_R){
				radioMenu();
			}
			else if(button == BUTTON_M){
				IsFlip = !IsFlip;
				flipDisplay(IsFlip); uhr_deinit();
				uhr_init(0, 0, 240, ColorIndex);
			}
		}
		if(AlarmAktiv && ! RadioON){
			rtc_get_datetime(&AktDate);
			if( AlarmHour == AktDate.hour \
				&& AlarmMin == AktDate.min \
				&& AktDate.sec < 2 )
					radioMenu();
		}
	}
}

