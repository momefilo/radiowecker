// ili_9341 Bibliothek
#include "ili_9341.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "font12x16.h"
#include "font7x11.h"
#include "font10x16.h"
#include "font14x20.h"
#include <string.h>

/*
 * display spi_bus spi_1
 * display cs/csn GP_9
 * display reset GP_8
 * display DC GP_7
 * display SDIO/SPI_TX GP_11
 * display SCK/SPI_SCK GP_10
*/
#define SPI_PORT spi1
#define PIN_SCK	10
#define PIN_SDA	11
#define PIN_DC  7
#define PIN_CS  9
#define PIN_RST 8

#define TOUCH_SPI_PORT spi0
#define TOUCH_SCK_PIN 2
#define TOUCH_DIN_PIN 3
#define TOUCH_DOUT_PIN 4
#define TOUCH_CS_PIN 5

uint16_t FgColor = 0xFEE0;
uint16_t BgColor = 0x0000;
uint16_t SeColor = 0xE0FF;
uint16_t Width, Height;
uint16_t Pos[2] = {0xffff,0xffff};

static inline void cs_select() {
	asm volatile("nop \n nop \n nop");
	gpio_put(PIN_CS, 0);
	asm volatile("nop \n nop \n nop");
}
static inline void cs_deselect() {
	asm volatile("nop \n nop \n nop");
	gpio_put(PIN_CS, 1);
	asm volatile("nop \n nop \n nop");
}
void write_cmd(uint8_t *cmd, int len) {
	cs_select();
	spi_write_blocking(SPI_PORT, cmd, 1);
	if(len > 1){
		gpio_put(PIN_DC, 1);
		spi_write_blocking(SPI_PORT, cmd + (uint8_t)1, len - 1);
		gpio_put(PIN_DC, 0);
	}
	cs_deselect();
}
void set_col(uint16_t start, uint16_t end){
	uint8_t cmd[5];
	cmd[0] = 0x2A;
	cmd[2] = start & 0x00FF;
	cmd[1] = (start & 0xFF00)>>8;
	cmd[4] = end & 0x00FF;
	cmd[3] = (end & 0xFF00)>>8;
	write_cmd(cmd, 5);
}
void set_row(uint16_t start, uint16_t end){
	uint8_t cmd[5];
	cmd[0] = 0x2B;
	cmd[2] = start & 0x00FF;
	cmd[1] = (start & 0xFF00)>>8;
	cmd[4] = end & 0x00FF;
	cmd[3] = (end & 0xFF00)>>8;
	write_cmd(cmd, 5);
}

void write_font7x11(uint16_t *pos, uint8_t zeichen){
	uint8_t font_ofset = 0x20;// Zeichensatz-Anfang
	uint8_t font_width = 7;
	uint8_t font_height = 11;
	uint8_t leer_col = 1;// Die linke Spalte ist leer da nur 7 bit
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];// buffer fuer das Zeichen
	buf[0] = 0x2C;// Startbyte
	int z = 1; // Schleifen-Zaehlvariable
	//write font in buffer
	if(zeichen > 127 || zeichen < font_ofset) zeichen = font_ofset;
	for(int y=0; y<font_height; y=y+1){  // 11 Byte umfasst ein 7x11Bit Zeichen
		for(int x=leer_col; x<8; x=x+1){ // 7 Bits pro Byte
			if(FONT7x11[zeichen-font_ofset][y] & (0x01 << (x - 1))){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0], pos[0] + font_width - 1);
	set_row(pos[1], pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText7x11(uint16_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t len = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint16_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*7;
			mypos[1] = pos[1]*11;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			mypos[0] = mypos[0] + i*7;
		}
		write_font7x11(mypos, text[i]);
	}
	FgColor = tmp_color;
}

void write_font10x16(uint16_t *pos, uint8_t zeichen){
	uint8_t font_ofset = 0x20;
	uint8_t font_width = 10;
	uint8_t font_height = 16;
	uint8_t leer_col = 6;
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];
	buf[0] = 0x2C;// Startbyte
	int z = 1;// Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<32; i=i+1){ // 32 Byte umfasst ein 10x16Bit Zeichen
		int anfang = 0;
		if(i % 2 == 0) anfang = 6;
		for(int k=anfang; k<8; k=k+1){ // zwei oder acht Bits pro Byte
//			if(zeichen < 0x21) zeichen = 0x21; // THIS IS A BUG!
			if(FONT10x16[zeichen-font_ofset][i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0] + font_width - 1);
	set_row(pos[1],pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText10x16(uint16_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t len = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint16_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*10;
			mypos[1] = pos[1]*16;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*10;
		}
		write_font10x16(mypos, text[i]);
	}
	FgColor = tmp_color;
}

void write_font12x16(uint16_t *pos, uint8_t zeichen){
	uint8_t font_width = 12;
	uint8_t font_height = 16;
	uint8_t leer_col = 4;
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];
	buf[0] = 0x2C;// Startbyte
	int z = 1;// Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<32; i=i+1){ // 32 Byte umfasst ein 16x16Bit Zeichen
		int ofset = i + 1;
		int ende = 8;
		if(ofset % 2 == 0) ende = 4;
		for(int k=0; k<ende; k=k+1){ // acht oder vier Bits pro Byte
			if(FONT16x16[zeichen*32+i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0] + font_width - 1);
	set_row(pos[1],pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText12x16(uint16_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t laenge = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<laenge; i++){
		uint16_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*12;
			mypos[1] = pos[1]*16;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*12;
		}
		write_font12x16(mypos, text[i]);
	}
	FgColor = tmp_color;
}

void write_font14x20(uint16_t *pos, uint8_t zeichen){
	uint8_t font_width = 14;
	uint8_t font_height = 20;
	uint8_t leer_col = 2;
	//create buffer for font
	int len = font_width * font_height * 2 + 1;
	uint8_t buf[len];
	buf[0] = 0x2C;// Startbyte
	int z = 1;// Schleifen-Zaehlvariable
	//write font in buffer
	for(int i=0; i<40; i=i+1){ // 40 Byte umfasst ein 14x20Bit Zeichen
		int anfang = 0;
		if(i % 2 == 0) anfang = leer_col;
		for(int k=anfang; k<8; k=k+1){ // zwei oder acht Bits pro Byte
			if(zeichen <0x20) zeichen = 0x20;
			if(FONT14x20[zeichen-0x20][i] & (0x80 >> k)){ // ist das Bit gesetzt
				buf[z] = (FgColor & 0xFF00)>>8;
				buf[z+1] = (FgColor & 0x00FF);
			}
			else{ // Das Bit ist nicht gesetzt
				buf[z] = (BgColor & 0xFF00)>>8;
				buf[z+1] = (BgColor & 0x00FF);
			}
			z=z+2;
		}
	}
	//write buffer to display
	set_col(pos[0],pos[0] + font_width - 1);
	set_row(pos[1],pos[1] + font_height - 1);
	write_cmd(buf,len);
}
void writeText14x20(uint16_t *pos, char *text, bool sel, bool matrix){
	uint16_t tmp_color = FgColor;
	uint8_t len = strlen(text);
	if(sel) FgColor = SeColor;
	for(int i=0; i<len; i++){
		uint16_t mypos[2];
		if(matrix){
			mypos[0] = (pos[0]+i)*14;
			mypos[1] = pos[1]*20;
		}
		else{
			mypos[0] = pos[0];
			mypos[1] = pos[1];
			if(i>0) mypos[0] = mypos[0] + i*14;
		}
		write_font14x20(mypos, text[i]);
	}
	FgColor = tmp_color;
}

void clearScreen(){
	//color = RRRRRGGG GGGBBBBB
	int len = Height * Width * 2 +1;
	uint8_t area[len];
	for(int i=1; i<len-1; i=i+2){
		area[i] = (BgColor & 0xFF00)>>8;
		area[i+1] = (BgColor & 0x00FF);
	}
	area[0] = 0x2C;
	set_col(0, Width -1);
	set_row(0, Height -1);
	write_cmd(area, len);
}

void ili9341_init(){
	if(true){//spi setup
		int speed = spi_init(SPI_PORT, 50* 1000 * 1000);
		spi_set_format(SPI_PORT, 8, 0, 0, SPI_MSB_FIRST);
		gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
		gpio_set_function(PIN_SDA, GPIO_FUNC_SPI);

		gpio_init(PIN_CS);
		gpio_set_dir(PIN_CS, GPIO_OUT);
		gpio_put(PIN_CS, 1);

		gpio_init(PIN_DC);
		gpio_set_dir(PIN_DC, GPIO_OUT);
		gpio_put(PIN_DC, 0);

		gpio_init(PIN_RST);
		gpio_set_dir(PIN_RST, GPIO_OUT);
		gpio_put(PIN_RST, 1);
	}
	sleep_ms(200);
	if(true){//display init
		uint8_t cmd[2];
		cmd[1] = 0;

		//sleep out
		cmd[0] = 0x11;
		write_cmd(cmd,1);
		sleep_ms(200);

		//display on
		cmd[0]=0x29;
		write_cmd(cmd,1);

		//flip vertikal&hoizontal & col/row-exchange
		cmd[0] = 0x36;
		cmd[1] = 0x28;
		write_cmd(cmd,2);
		Width = 320; Height = 240;

		cmd[0] = 0x3A;	//pixfrm
		cmd[1] = 0x55;	// 565
		write_cmd(cmd,2);
	}
	clearScreen();
}
void setOrientation(uint8_t ori){
	Width = 320;
	Height= 240;
	uint8_t cmd[2];
	cmd[0] = 0x36;
	cmd[1] = 0x28;
	if(ori>0){
		cmd[1] = 0x88;
		Width = 240;
		Height= 320;
	}
	write_cmd(cmd,2);
}
void setSeColor(uint16_t color){ SeColor = color;}
void setFgColor(uint16_t color){ FgColor = color;}
void setBgColor(uint16_t color){ BgColor = color;}

void paintRect(uint16_t *area, uint16_t color){
	int len = (area[2] - area[0] + 1)*(area[3] - area[1] + 1)*2+1;
	uint8_t pixarea[len];
	pixarea[0] = 0x2C;
	for(int i=1; i<len-2; i=i+2){
		pixarea[i] = (color & 0xFF00)>>8;
		pixarea[i+1] = (color & 0x00FF);
	 }
	set_col(area[0], area[2]);
	set_row(area[1], area[3]);
	write_cmd(pixarea, (len));
}
void drawRect(uint16_t *area, uint8_t *data){
	int len = (area[2] - area[0] + 1)*(area[3] - area[1] + 1)*2+1;
	set_col(area[0], area[2]);
	set_row(area[1], area[3]);
	write_cmd(data, (len));
}
void paintRectGradient(uint16_t *area, uint16_t color1, uint16_t color2){
	int width = (area[2] - area[0]) + 1;
	int height = (area[3] - area[1]) + 1;
	int len = width*height*2+1;
	uint8_t pixarea[len];
	pixarea[0] = 0x2C;

	int8_t red_diff = (((color2 & 0xF800) >> 11) - ((color1 & 0xF800) >> 11));
	float red_summand = red_diff / (float)width;
	float red = ((color1 & 0xF800) >> 11);
	int8_t green_diff = (((color2 & 0x07E0) >> 5) - ((color1 & 0x07E0) >> 5));
	float green_summand = green_diff / (float)width;
	float green = ((color1 & 0x07E0) >> 5);
	int8_t blue_diff = ((color2 & 0x001F)-(color1 & 0x001F));
	float blue_summand = blue_diff / (float)width;
	float blue = ((color1 & 0x001F));

	uint count = 0;
	for(int i=1; i<len-2; i=i+2){
		uint16_t color = ((uint16_t)red << 11) | ((uint16_t)green << 5) | (uint16_t)blue;
		pixarea[i] = (color & 0xFF00)>>8;
		pixarea[i+1] = (color & 0x00FF);
		red = red + red_summand;
		green = green + green_summand;
		blue = blue + blue_summand;
		count++;
		if(count >= (width)){
			count = 0;
			red = (color1 & 0xF800) >> 11;
			green = (color1 & 0x07E0) >> 5;
			blue = (color1 & 0x001F);
		}
	 }
	set_col(area[0], area[2]);
	set_row(area[1], area[3]);
	write_cmd(pixarea, (len));
}

void ili9341_touch_init(){
	int speed = spi_init(TOUCH_SPI_PORT, 200*1000);
	spi_set_format(TOUCH_SPI_PORT, 8, 0, 0, SPI_MSB_FIRST);
	gpio_set_function(TOUCH_SCK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(TOUCH_DIN_PIN, GPIO_FUNC_SPI);
	gpio_set_function(TOUCH_DOUT_PIN, GPIO_FUNC_SPI);
	gpio_init(TOUCH_CS_PIN);
	gpio_set_dir(TOUCH_CS_PIN, GPIO_OUT);
	gpio_put(TOUCH_CS_PIN, 1);
}

void touch_select() {
	asm volatile("nop \n nop \n nop");
	gpio_put(TOUCH_CS_PIN, 0);
	asm volatile("nop \n nop \n nop");
}
void touch_deselect() {
	asm volatile("nop \n nop \n nop");
	gpio_put(TOUCH_CS_PIN, 1);
	asm volatile("nop \n nop \n nop");
}

/*returns the position of the touch or [0xffff,0xffff] if no touch */
uint16_t *ili9341_getTouch(){
	uint8_t msb = 0, lsb = 0;
	Pos[0]=0xffff;
	Pos[1]=0xffff;
	touch_select();
	uint8_t cmd = 0xB0;
	spi_write_blocking(TOUCH_SPI_PORT, &cmd, 1);
	spi_read_blocking(TOUCH_SPI_PORT, 0, &msb, 1);
	spi_read_blocking(TOUCH_SPI_PORT, 0, &lsb, 1);
	uint16_t z1 = ((((msb & 0x7ff) << 8) + lsb) >> 3);
	cmd = 0xC0;
	spi_write_blocking(TOUCH_SPI_PORT, &cmd, 1);
	spi_read_blocking(TOUCH_SPI_PORT, 0, &msb, 1);
	spi_read_blocking(TOUCH_SPI_PORT, 0, &lsb, 1);
	uint16_t z2 = ((((msb & 0x7ff) << 8) + lsb) >> 3);
	if((z1 > 100) || (z2 < 3500)){
		uint16_t xoffset = (3830-230)/240;
		uint16_t yoffset = (3870-350)/320;
		cmd = 0xD0;
		spi_write_blocking(TOUCH_SPI_PORT, &cmd, 1);
		spi_read_blocking(TOUCH_SPI_PORT, 0, &msb, 1);
		spi_read_blocking(TOUCH_SPI_PORT, 0, &lsb, 1);
		uint16_t xypos = ((((msb & 0x7ff) << 8) +lsb) >> 3);
		if(xypos < 230) xypos = 230;
		if(xypos > 3830) xypos = 3830;
		Pos[0] = (xypos-230)/xoffset;
		Pos[0] = 240 - Pos[0];
		cmd = 0x90;
		spi_write_blocking(TOUCH_SPI_PORT, &cmd, 1);
		spi_read_blocking(TOUCH_SPI_PORT, 0, &msb, 1);
		spi_read_blocking(TOUCH_SPI_PORT, 0, &lsb, 1);
		uint16_t y = (((msb & 0x7ff) << 8) +lsb) >> 3;
		if(y > 3870) y = 3870;
		if(y < 350) y = 350;
		Pos[1] = 320 - (3870 - (y))/yoffset;
	}
	touch_deselect();
	return Pos;
}
