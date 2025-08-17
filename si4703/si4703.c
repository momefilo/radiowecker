// si4703 Bibliothek
#include "si4703.h"

uint8_t BUS, SDA, SCL, RST, GP2, ADDR = 0x10;
uint8_t REGISTER[32]; // Starts by PowerCnfReg. 02h upper-byte

void read_register(){
	uint8_t buf[32];
	int ret=i2c_read_blocking(I2C_INSTANCE(BUS), ADDR, buf, 32, false);
	for(int i=16; i<32; i++) REGISTER[i-16] = buf[i];
	for(int i=0; i<16; i++) REGISTER[i+16] = buf[i];
}
// for Debug
void print_register(){
	read_register();
	for(int i=0; i<32; i=i+2){
//		printf("Reg 0x%02X: 0x%02X%02X\n", i/2+2, REGISTER[i], REGISTER[i+1]);
	}
}

uint8_t *si4703_get_register(){
	read_register();
	return REGISTER;
}

void si4703_write_register(uint8_t cnt){
	i2c_write_blocking(I2C_INSTANCE(BUS), ADDR, REGISTER, cnt, false);
}

void si4703_enable(bool on){
	if(on) REGISTER[1] = 0x01;
	else REGISTER[1] = 0x41;
}

void si4703_dsmute(bool disable){
	if(disable) REGISTER[0] |= 0x80;
	else REGISTER[0] &= 0x7F;
}

void si4703_dmute(bool disable){
	if(disable) REGISTER[0] |= 0x40;
	else REGISTER[0] &= 0xBF;
}

void si4703_mono(bool mono){
	uint mask = 0x20;
	if(mono) REGISTER[0] |= 0x20;
	else REGISTER[0] &= 0xDF;
}

void si4703_rdsm(bool verbose){
	uint mask = 0x08;
	if(verbose) REGISTER[0] |= 0x08;
	else REGISTER[0] &= 0xF7;
}

void si4703_seekmode(bool stop){
	uint mask = 0x04;
	if(stop) REGISTER[0] |= 0x04;
	else REGISTER[0] &= 0xFB;
}

void si4703_seekup(bool up){
	if(up) REGISTER[0] |= 0x02;
	else REGISTER[0] &= 0xFD;
}

void si4703_seek(bool seek){
	if(seek) REGISTER[0] |= 0x01;
	else REGISTER[0] &= 0xFE;
}

void si4703_tune(bool tune){
	if(tune) REGISTER[2] |= 0x80;
	else REGISTER[2] &= 0x7F;
}

void si4703_chan(uint16_t chan){
	REGISTER[3] |= chan;
	REGISTER[2] |= (chan >> 8) & 0x03;
}

void si4703_rdsien(bool enable){
	if(enable) REGISTER[4] |= 0x80;
	else REGISTER[4] &= 0x7F;
}

void si4703_stcien(bool set){
	if(set) REGISTER[4] |= 0x40;
	else REGISTER[4] &= 0xBF;
}

void si4703_rds(bool on){
	if(on) REGISTER[4] |= 0x10;
	else REGISTER[4] &= 0xEF;
}

void si4703_de(bool de){
	if(de) REGISTER[4] |= 0x08;
	else REGISTER[4] &= 0xF7;
}

void si4703_agcd(bool disable){
	if(disable) REGISTER[4] |= 0x04;
	else REGISTER[4] &= 0xFB;
}

void si4703_blndadj(uint8_t adj){
	REGISTER[5] &= 0x3F;
	REGISTER[5] |= (adj & 0x03) << 6;
}

void si4703_gp3(uint8_t set){
	REGISTER[5] &= 0xCF;
	REGISTER[5] |= (set & 0x03) << 4;
}

void si4703_gp2(uint8_t set){
	REGISTER[5] &= 0xF3;
	REGISTER[5] |= (set & 0x03) << 2;
}

void si4703_gp1(uint8_t set){
	REGISTER[5] &= 0xFC;
	REGISTER[5] |= (set & 0x03);
}

void si4703_seekth(uint8_t th){
	REGISTER[6] = th;
}

void si4703_band(uint8_t band){
	REGISTER[7] &= 0x3F;
	REGISTER[7] |= (band & 0x03) << 6;
}

void si4703_space(uint8_t space){
	REGISTER[7] &= 0xCF;
	REGISTER[7] |= (space & 0x03) << 4;
}

void si4703_volume(uint8_t vol){
	REGISTER[7] &= 0xF0;
	REGISTER[7] |= (vol & 0x0F);
}

void si4703_smuter(uint8_t rate){
	REGISTER[8] &= 0x3F;
	REGISTER[8] |=  (rate & 0x03)<< 6;
}

void si4703_smutea(uint8_t att){
	REGISTER[8] &= 0xCF;
	REGISTER[8] |=  (att & 0x03) << 4;
}

void si4703_rdsprf(bool enable){
	if(enable) REGISTER[8] |= 0x02;
	else REGISTER[4] &= 0xFD;
}

void si4703_volext(bool enable){
	if(enable) REGISTER[8] |= 0x01;
	else REGISTER[4] &= 0xFE;
}

void si4703_sksnr(uint8_t th){
	REGISTER[9] &= 0x0F;
	REGISTER[9] |= (th & 0x0F) << 4;
}

void si4703_skcnt(uint8_t cnt){
	REGISTER[9] &= 0xF0;
	REGISTER[9] |= (cnt & 0x0F);
}

void si4703_xoscen(bool enable){
	if(enable) REGISTER[10] |= 0x80;
	else REGISTER[10] &= 0x7F;
}

void si4703_ahizen(bool enable){
	if(enable) REGISTER[10] |= 0x40;
	else REGISTER[10] &= 0xBF;
}

void si4703_powerDown(){
	si4703_dsmute(false);
	si4703_enable(false);
	si4703_write_register(2);
}

void si4703_powerUp(){
	si4703_dsmute(true);
	si4703_enable(true);
	si4703_write_register(2);
	busy_wait_ms(100);
}

void si4703_init(uint8_t bus, uint8_t sda, uint8_t scl, uint8_t rst, uint8_t gp2, bool ahizen){
	BUS = bus; SDA = sda; SCL = scl; RST = rst;GP2 = gp2;
	if(1){// init the ports for Startcondition
		gpio_init(rst);
		gpio_init(gp2);
		gpio_init(sda);
		gpio_init(scl);
		gpio_set_pulls(gp2, 0, 1);
		gpio_set_pulls(rst, 0, 1);
		gpio_set_pulls(sda, 0, 1);
		gpio_set_pulls(scl, 0, 1);
		gpio_set_dir(rst, 1);
		gpio_set_dir(sda, 1);
		gpio_set_dir(scl, 1);
		gpio_put(sda, 0);
		gpio_put(scl, 0);
		gpio_put(rst, 0);
		busy_wait_ms(1);
		gpio_put(rst, 1);
		busy_wait_ms(5);
		gpio_deinit(sda);
		gpio_deinit(scl);
		i2c_init(I2C_INSTANCE(bus), 400*1000);
		gpio_set_function(sda, GPIO_FUNC_I2C);
		gpio_set_function(scl, GPIO_FUNC_I2C);
		gpio_pull_up(sda);
		gpio_pull_up(scl);
		gpio_pull_up(rst);
		gpio_pull_up(gp2);
		read_register();// Wichtig, s kann das Board nicht funktionieren
		si4703_xoscen(true);
		si4703_write_register(12);
		busy_wait_ms(500);
		si4703_enable(true);
		si4703_dmute(true);
		si4703_write_register(2);
		busy_wait_ms(110);
		if(ahizen){
			si4703_enable(false);
			si4703_ahizen(true);
			si4703_write_register(2);
			busy_wait_ms(110);
			read_register();
			si4703_ahizen(ahizen);
			si4703_xoscen(true);
			si4703_enable(true);
			si4703_dmute(true);
			si4703_write_register(12);
			busy_wait_ms(110);
		}
	}
}

// Das Radio-RDS-System

// Flags: 1=most, 2=arti, 3=comp, 4=dpty, 5=M/S, 6=ta, 7=tp
static void (*callbackPS)(char[11], uint8_t flag);
static void (*callbackTEXT)(char[129], uint8_t pty);
// Flags: 1=Stereo, 2= RDSS, 3=AFCRL, 4=SF/BL, 5=STC, 6=RDSR
static void (*callbackMHz)(double_t mhz, uint8_t rssi, uint8_t flags);

uint8_t RDSText[65];
uint16_t RDSText_flag = 0;
uint8_t RDSText_ABflag = 0;
uint8_t StationName[8];
uint8_t StationName_flag = 0;

void clearRDSData(){// wird von seeking und tuning aufgeerufen
	for(int i=0; i<65; i++){
		if(i < 8) StationName[i] = 0;
		RDSText[i]=0;
	}
}

// Die RDS-Decodierung. Siehe GNURadio
void RDS(uint gpio, uint32_t events){
	if(gpio == GP2){
		uint8_t *reg = si4703_get_register();
		if((reg[16] & 0x80)){// RDSR bit is set
			uint8_t group = ((reg[22] & 0xF0) >> 4);
			char version = 'A';
			if(reg[22] & 0x08) version = 'B';
			if(group == 0){// PS Stationname
				bool tp = (reg[22] & 0x04);
				bool ta = (reg[23] >>  4) & 0x01;
				bool ms = (reg[23] >>  3) & 0x01;
				bool di = (reg[23] >>  2) & 0x01;
				bool dpty, comp, arti, most;
				uint8_t segment = (reg[23] & 0x03);
				char ps_1 = reg[26];
				char ps_2 = reg[27];
				if(segment == 0 && di) dpty = true;
				else if(segment == 1 && di) comp = true;
				else if(segment == 2 && di) arti = true;
				else if(segment == 3 && di) most = true;
				uint8_t psFlags = (tp << 6);
				psFlags |= (ta << 5); psFlags |= (ms << 4);
				psFlags |= (dpty << 3); psFlags |= (comp << 2); psFlags |= (arti << 1); psFlags != most;
				if(StationName_flag & (1 << segment)){// das Segment wurde bereits empfangen
					if((StationName[segment * 2] != ps_1) || (StationName[segment * 2 + 1] != ps_2)){
						StationName[segment * 2] = ps_1;
						StationName[segment * 2 + 1] = ps_2;
						StationName_flag = (1 << segment);
					}
				}
				else {
					//printf("%d, %x\n", segment, StationName_flag);
					StationName[segment * 2] = ps_1;
					StationName[segment * 2 + 1] = ps_2;
					StationName_flag |= (1 << segment);
				}
				if(StationName_flag == 0xF) callbackPS(StationName, psFlags);
			}
			else if(group == 2){// rds-text
				uint8_t pty = ((reg[22] & 0x03) << 3) | ((reg[23] & 0xE0) >> 5);
				uint8_t segment = (reg[23] & 0x0f);
				uint8_t seg_width = 4;
				uint8_t seg_valid = 0;
				bool abflag = (reg[23] >>  4) & 0x01;
				if(RDSText_ABflag != abflag) RDSText_flag = 0;// neuer Inhalt
				RDSText_ABflag = abflag;
				if(version == 'A'){
					RDSText[segment * 4] = reg[24];
					RDSText[segment * 4 + 1] = reg[25];
					RDSText[segment * 4 + 2] = reg[26];
					RDSText[segment * 4 + 3] = reg[27];
				}else{
					seg_width = 2;
					RDSText[segment * 2] = reg[26];
					RDSText[segment * 2 + 1] = reg[27];
				}
				RDSText_flag |= (1 << segment);
				for(int i=0; i<16; i++){ if(RDSText_flag & (1 << i)) seg_valid++; else break; }
				char *tail = (char *)memchr(RDSText, '\r', seg_valid * seg_width);
				if(tail || seg_valid == 16) callbackTEXT(RDSText, pty);
			}
		}
	}
}

// quality=0 most Stops, 1 better Qality, >1 best Quality
void si4703_setSeekQuality(uint8_t quality){
	if(quality <1){
		si4703_seekth(0);
		si4703_sksnr(0x04);
		si4703_skcnt(0x04);
	}else if(quality < 2){
		si4703_seekth(0x19);
		si4703_sksnr(0x00);
		si4703_skcnt(0x00);
	}else if(quality < 3){
		si4703_seekth(0x0C);
		si4703_sksnr(0x0F);
		si4703_skcnt(0x0F);
	}else{
		si4703_seekth(0x19);
		si4703_sksnr(0x0F);
		si4703_skcnt(0x0F);
	}
	si4703_write_register(10);
}

void si4703_germanDefault_setup( \
		void (*cbPS)(char[11], uint8_t flag), \
		void (*cbTEXT)(char[129], uint8_t pty), \
		void (*cbMHz)(double_t mhz, uint8_t rssi, uint8_t flags)){

	si4703_gp2(0x01);// set GP2 irq enable
	si4703_stcien(true);// seek-complete irq on GP2
	si4703_rdsien(true);// RDS-Data-complete irq on GP2
	si4703_band(0x0);// 87MHz - 108MHz
	si4703_space(0x01);// 100kHz Kanalbreite
	si4703_de(true);// deemphasis 50µs
	si4703_rds(true);
	si4703_volume(0x8);
	si4703_setSeekQuality(1);

	callbackPS = cbPS;
	callbackTEXT = cbTEXT;
	callbackMHz = cbMHz;
	gpio_set_irq_enabled_with_callback(GP2, GPIO_IRQ_EDGE_RISE, true, RDS);
}

uint8_t getFlag(){// Helper for seeking/tuning
	uint8_t flags = 0x00 | (REGISTER[16] & 0x80) >> 2;// RDSR bit 6
	flags |= (REGISTER[16] & 0x40) >> 2;// STC bit 4
	flags |= (REGISTER[16] & 0x20) >> 2;// SF/BL bit 3
	flags |= (REGISTER[16] & 0x10) >> 2;// AFCRL bit 2
	flags |= (REGISTER[16] & 0x08) >> 2;// RDSS bit 1
	flags |= (REGISTER[16] & 0x01);// Stereo bit 0
	return flags;
}
void si4703_seeking(bool up){
	si4703_seekup(up);
	si4703_seek(true);
	clearRDSData();
	si4703_write_register(2);
	//wait for gpio ready seek
	while(gpio_get(GP2));
	busy_wait_ms(50);
	double_t mhz = si4703_getMHz();// reads the Register
	uint8_t rssi = REGISTER[17];
	uint8_t flags = getFlag();
	si4703_seek(false);
	si4703_write_register(2);
	//while(si4703_get_register()[16] & 0x40){busy_wait_ms(50);}

	callbackMHz(mhz, rssi, flags);
}

void si4703_tuning(double_t mhz){
	double_t space = 0.2;
	if(REGISTER[7] & 0x10) space = 0.1;
	if(REGISTER[7] & 0x20) space = 0.05;
	double_t ofset = 76.0;
	if( (REGISTER[7] & 0xC0) == 0) ofset = 87.5;
	double_t f_chan = (mhz - ofset) / space;
	uint16_t chan = f_chan;
	si4703_chan(chan);
	si4703_tune(true);
	si4703_write_register(4);
	while(gpio_get(GP2));
	busy_wait_ms(60);
	double_t real_mhz = si4703_getMHz();// reads the Register
	uint8_t rssi = REGISTER[17];
	uint8_t flags = getFlag();
	si4703_tune(false);
	si4703_write_register(4);
	//while(si4703_get_register()[16] & 0x40){busy_wait_ms(50);}
	clearRDSData();
	callbackMHz(real_mhz, rssi, flags);
}

void si4703_tuneSpace(uint8_t steps, bool up){
	double_t space = 0.2;
	if(REGISTER[7] & 0x10) space = 0.1;
	if(REGISTER[7] & 0x20) space = 0.05;
	double_t ende = 108.0;
	if( (REGISTER[7] & 0xC0) == 3) ende = 90.0;
	double_t ofset = 76.0;
	if( (REGISTER[7] & 0xC0) == 0) ofset = 87.5;
	uint16_t chan = (REGISTER[19] | ((REGISTER[18] & 0x03)<<8)) - steps * space;
	if(chan < ofset) chan = ende - (ofset-chan);
	if(up) chan = (REGISTER[19] | ((REGISTER[18] & 0x03)<<8)) + steps * space;
	if(chan > ende) chan = ofset + (chan-ende);
	si4703_tuning(chan * space + ofset);
}

void si4703_stepVolume(bool up){
	uint8_t vol = REGISTER[7] & 0x0F;
	if(up){ if(vol < 0x0F) vol++; }
	else { if(vol > 0x00) vol--; }
	si4703_volume(vol);
	si4703_write_register(8);
}

double_t si4703_getMHz(){
	read_register();
	uint16_t chan = (REGISTER[19] | ((REGISTER[18] & 0x03)<<8));
	double_t space = 0.2;
	if(REGISTER[7] & 0x10) space = 0.1;
	if(REGISTER[7] & 0x20) space = 0.05;
	double_t ofset = 76.0;
	if( (REGISTER[7] & 0xC0) == 0) ofset = 87.5;
	return chan * space + ofset;
}

/* Gibt Das Statusregister 0Ah zurück
 * RDSR,STC,SF/BL,AFCRL,RDSS,BLERA[2:1],ST,RSSI[7:0]
*/uint16_t si4703_getStatus(){
	read_register();
	return (REGISTER[16] << 8) | (REGISTER[17]);
}

//##############   Beispielanwendung   #####################
/*
#include "../buttons/buttons.h"

#define I2C_BUS 0
#define I2C_SDA 4
#define I2C_SCL 5
#define RESET 6
#define GP2 7

// Print RDS-Data

char PSName[11], PSFlags[20], RDSTx1[34], RDSTx2[34], STCText[25], STCFlags[45];
uint8_t Sensivity = 1;
void printMsg(){
	printf("%s ", PSName);
	printf("%s\n", STCText);
	printf("%s\n", STCFlags);
	printf("%s\n", PSFlags);
	printf("%s\n", RDSTx1);
	printf("%s\n", RDSTx2);
	printf("%s\n", " ");
}

// flag: 0x01=most, 0x02=arti, 0x04=comp, 0x08=dpty, 0x10=M/S, 0x20=ta, 0x40=tp
void printPS(char psname[11], uint8_t flag){
	sprintf(PSName, "%s", psname);
	sprintf(PSFlags, "M/S:%d TA:%d TP:%d",
		(flag & 0x10)>>4, (flag & 0x20)>>5, (flag & 0x40)>>6 );
	printMsg();
}

void printRDSText(char text[129], uint8_t pty){
	char zeile1[33], zeile2[33];
	for(int i=0; i<32; i++){
		zeile1[i] = text[i];
		zeile2[i] = text[i+32];
	}
	sprintf(RDSTx1, "%s", zeile1);
	sprintf(RDSTx2, "%s", zeile2);
	printMsg();
}

// flags: 0x01=Stereo, 0x02= RDSS, 0x04=AFCRL, 0x08=SF/BL, 0x10=STC, 0x20=RDSR
void printMHz(double_t mhz, uint8_t rssi, uint8_t flags){
	sprintf(STCText, "%.2fMHz %d dBuV", mhz, rssi);
	sprintf(STCFlags, "ST:%d RDSS:%d AFCRL:%d\nSF/BL:%d STC:%d RDSR:%d", \
			(flags & 0x01), (flags & 0x02)>>1, (flags & 0x04)>>2,
			(flags & 0x08)>>3, (flags & 0x10)>>4, (flags & 0x20)>>5 );
	printMsg();
}

void loop() {
	buttons_init();
	si4703_init(I2C_BUS, I2C_SDA, I2C_SCL, RESET, GP2, false);
	si4703_germanDefault_setup(printPS, printRDSText, printMHz);
	si4703_setSeekQuality(2);
	si4703_tuning(88.8);
	while(true){
		uint8_t button = get_Button();
		if(button < 100){
			printf("button\n");
			if(button == BUTTON_U){ si4703_seeking(true); }
			else if(button == BUTTON_D){ si4703_seeking(false); }
			else if(button == BUTTON_L){ si4703_stepVolume(false); }
			else if(button == BUTTON_R){ si4703_stepVolume(true); }
			else if(button == BUTTON_M){
				Sensivity ++;
				if(Sensivity > 3) Sensivity = 0;
				si4703_setSeekQuality(Sensivity);
			}
		}
	}
}
*/
