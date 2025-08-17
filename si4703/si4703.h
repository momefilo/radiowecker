// momefilo Desing
/* si4703 RDS-Tuner-Eval-Board
 * Diese Bibliothek bietet Komfortfunktionen die eine umfänliche Bedienung
 * mit RDS ohne Datenblatt und Bitverschieberei ermoeglicht
 *  + void si4703_init(i2c_bus, i2c_sda, i2c_scl, reset, gp2, ahizen_enable)
 *  + void si4703_germanDefault_setup(callbackPS, callbackRDS, callbackSTC)
 * muessen aufgerufen sein damit
 *  + void si4703_seeking(bool up)
 *  + void si4703_tuning(float_t mhz)
 *  + void si4703_tuneSpace(uint8_t steps, bool up)
 *  + void si4703_stepVolume(bool up)
 *  + void si4703_setSeekQuality(uint8_t quality)-Funktion.
 * 		quality=0=viele Stationen, 1=gute Qualitaet(default), 2=bessere Qualitaet
 * genutzt werden koennen
 * germanDefault_setup wird von 0 bis 2 folgend drei callback-Funktionen als Parameter übergeben
 * welche jeweils mit den nun folgenden Parametern in Nutzercode definiert sein muesssen.
 * 		0: (char[11] text, uint8_t flag) // der acht-stellige Sendername
 * 		1: (char[64] text, uint8_t flag) // der 64-stellige RDS-Text
 *  	2: (float_t mhz, uint8_t rssi, uint8_t flags)// das Statusregister
 * Die Flags sind unten, ueber der Funktion dokumentiert. Die beiden ersten Callbacks werden
 * wiederholt nach vollstaendigem Datenempfang ausgefuehrt, die letztere nach aufruf der folgenden
 * 		void si4703_seeking(bool up)
 * 		void si4703_tuning(float_t mhz)// mit Bandueberschreitung
 * 		void si4703_tuneSpace(uint8_t steps, bool up)// mit Loop über das Band-Ende/Anfang
 * Funktionen
 * Auch alle Register als einfache namentliche Setter-Funktionen mit Bitverschieberei sind geschrieben
 * und ausfuehrlich nach Datenblatt eindokumentiert so das eine einfache Nutzung gegeben ist.
 * Nach den settern muss "void si4703_write_register(uint8_t cnt)"// cnt=10 schreibt alle Control-Register
 * aufgerufen werden
 */
#ifndef my_si4703_h
#define my_si4703_h 1

#include "pico/stdlib.h"
#include "pico/double.h"
#include "hardware/i2c.h"
#include <string.h>

/*Powerup the si7403 with i2c-Interface, reset and GP02 Interrupt for STC and RDSR, and bool for output-offset
*/void si4703_init(uint8_t bus, uint8_t sda, uint8_t scl, uint8_t rst, uint8_t gp2, bool ahizen);

void si4703_powerDown();

void si4703_powerUp();

// Die Komfortfunktionen

/* Ein Deafault-Setup fuer Deutschland
 * *cbPS: Sendernamen Callbackfunktion.
 * flag: 0x01=most, 0x02=arti, 0x04=comp, 0x08=dpty, 0x10=M/S, 0x20=ta, 0x40=tp
 * *cbTEXT: RDSText Callbackfunktion. pty: PTY
 * *cbMHz: seek/tune Callbackfunktion
 * flags: 0x01=Stereo, 0x02= RDSS, 0x04=AFCRL, 0x08=SF/BL, 0x10=STC, 0x20=RDSR
*/void si4703_germanDefault_setup( \
		void (*cbPS)(char[11], uint8_t flag), \
		void (*cbTEXT)(char[129], uint8_t pty), \
		void (*cbMHz)(double_t mhz, uint8_t rssi, uint8_t flags));

/* Setzt die Seek_Sensibiltaet=0=most Stops, 1=more Stations (default), >1=good Quality
* Table 23. Summary of Seek Settings
* Configuration SEEKTH[7:0] SKSNR[3:0]  SKCNT[3:0]
* Default		0x19		0x00		0x00
* Recommended	0x19		0x4			0x8
* More Stations	0xC			0x4			0x8
* Good Quality	0xC			0x8			0xF
* Most Stations	0x0			0x4			0xF
*/void si4703_setSeekQuality(uint8_t quality);

void si4703_seeking(bool up);

void si4703_tuning(double_t mhz);

/* Tune steps*space(50kHz Default) from the current Channel down/up
*/void si4703_tuneSpace(uint8_t steps, bool up);

/* ändert in einer von 16 Schritten die Laustaerke (Default 8)
*/void si4703_stepVolume(bool up);

/* Gibt die aktuelle Frequenz nach Band/Offset/Space in MHz zurueck
*/double_t si4703_getMHz();

/* Gibt das Statuseriger 0Ah zurueck
*/uint16_t si4703_getStatus();


// Helper-Functions

void print_register();

/*Return 32 Byte Register around from 0=02h  to 31=01h
* (Register[0] si4703-POWERCFG-Register 02h)
* (Register[31] si4703-CHIPID-Register 01h)
*/uint8_t *si4703_get_register();

/*Write the Setting from Register 02h to 02h+cnt to the si4703
* cnt = 10 write the full config
*/void si4703_write_register(uint8_t cnt);

// The Registersettings

/*Enable or disable the si4703
*/void si4703_enable(bool on);

/*Softmute Disable.
* 0 = Softmute enable (default).
* 1 = Softmute disable.
*/void si4703_dsmute(bool disable);

/*Mute Disable.
* 0 = Mute enable (default).
* 1 = Mute disable.
*/void si4703_dmute(bool disable);

/*Mono Select.
* 0 = Stereo (default).
* 1 = Force mono.
*/void si4703_mono(bool mono);

/*RDS Mode.
* 0 = Standard (default).
* 1 = Verbose.
* Refer to “4.4. RDS/RBDS Processor and Functionality”.
*/void si4703_rdsm(bool verbose);

/*Seek Mode.
* 0 = Wrap at the upper or lower band limit and continue seeking (default).
* 1 = Stop seeking at the upper or lower band limit.
*/void si4703_seekmode(bool stop);

/*Seek Direction.
* 0 = Seek down (default).
* 1 = Seek up.
*/void si4703_seekup(bool up);

/*Seek.
* 0 = Disable (default).
* 1 = Enable.
* Notes:
* 1. Seek begins at the current channel, and goes in the direction specified with the SEEKUP
* bit. Seek operation stops when a channel is qualified as valid according to the seek
* parameters, the entire band has been searched (SKMODE = 0), or the upper or lower
* band limit has been reached (SKMODE = 1).
* 2. The STC bit is set high when the seek operation completes and/or the SF/BL bit is set
* high if the seek operation was unable to find a channel qualified as valid according to the
* seek parameters. The STC and SF/BL bits must be set low by setting the SEEK bit low
* before the next seek or tune may begin.
* 3. Seek performance for 50 kHz channel spacing varies according to RCLK tolerance.
* Silicon Laboratories recommends ±50 ppm RCLK crystal tolerance for 50 kHz seek
* performance.
* 4. A seek operation may be aborted by setting SEEK = 0.
*/void si4703_seek(bool seek);

/*Tune.
* 0 = Disable (default).
* 1 = Enable.
* The tune operation begins when the TUNE bit is set high. The STC bit is set high
* when the tune operation completes. The STC bit must be set low by setting the TUNE
* bit low before the next tune or seek may begin.
* Freq (MHz) = Spacing(kHz) * Channel + Bottom of Band(MHz)
* The tuning operation begins by setting the TUNE bit. After tuning
* completes, the seek/tune complete (STC) bit will be set
* and the RSSI level is available by reading bits
* RSSI[7:0]. The TUNE bit must be set low after the STC
* bit is set high in order to complete the tune operation
* and clear the STC bit.
*/void si4703_tune(bool tune);

/*Channel Select.
* Channel value for tune operation.
* If BAND 05h[7:6] = 00, then Freq (MHz) = Spacing (MHz) x Channel + 87.5 MHz.
* If BAND 05h[7:6] = 01, BAND 05h[7:6] = 10, then
* Freq (MHz) = Spacing (MHz) x Channel + 76 MHz.
* CHAN[9:0] is not updated during a seek operation. READCHAN[9:0] provides the
* current tuned channel and is updated during a seek operation and after a seek or
* tune operation completes. Channel spacing is set with the bits SPACE 05h[5:4].
*/void si4703_chan(uint16_t chan);

/*RDS Interrupt Enable.
* 0 = Disable Interrupt (default).
* 1 = Enable Interrupt.
* Setting RDSIEN = 1 and GPIO2[1:0] = 01 will generate a 5 ms low pulse on GPIO2 when
* the RDSR 0Ah[15] bit is set.
*/void si4703_rdsien(bool enable);

/*Seek/Tune Complete Interrupt Enable.
* 0 = Disable Interrupt (default).
* 1 = Enable Interrupt.
* Setting STCIEN = 1 and GPIO2[1:0] = 01 will generate a 5 ms low pulse on GPIO2 when
* the STC 0Ah[14] bit is set.
*/void si4703_stcien(bool set);

/*RDS Enable.
* 0 = Disable (default).
* 1 = Enable.
*/void si4703_rds(bool on);

/*De-emphasis.
* 0 = 75 µs. Used in USA (default).
* 1 = 50 µs. Used in Europe, Australia, Japan.
* To reduce the amount of high frequency noise in an FM system, the transmitting station boosts (pre-emphasis) the
* high frequency content expecting that the receiving radio will reduce (de-emphasis) the high frequency content by
* the same amount.
*/void si4703_de(bool de);

/*AGC Disable.
* 0 = AGC enable (default).
* 1 = AGC disable.
* An automatic gain control (AGC) circuit controls the gain of
* the LNA to optimize sensitivity and rejection of strong
* interferers. For testing purposes, the AGC can be
* disabled with the AGCD bit.
*/void si4703_agcd(bool disble);

/*Stereo/Mono Blend Level Adjustment.
* Sets the RSSI range for stereo/mono blend.
* 00 = 31–49 RSSI dBµV (default).
* 01 = 37–55 RSSI dBµV (+6 dB).
* 10 = 19–37 RSSI dBµV (–12 dB).
* 11 = 25–43 RSSI dBµV (–6 dB).
* ST bit set for RSSI values greater than low end of range.
*/void si4703_blndadj(uint8_t adj);

/*General Purpose I/O 3.
* 00 = High impedance (default).
* 01 = Mono/Stereo indicator (ST). The GPIO3 will output a logic high when the device is in
* stereo, otherwise the device will output a logic low for mono.
* 10 = Low.
* 11 = High.
*/void si4703_gp3(uint8_t set);

/*General Purpose I/O 2.
* 00 = High impedance (default).
* 01 = STC/RDS interrupt. A logic high will be output unless an interrupt occurs as
* described below.
* 10 = Low.
* 11 = High.
* Setting STCIEN = 1 will generate a 5 ms low pulse on GPIO2 when the STC 0Ah[14] bit is
* set. Setting RDSIEN = 1 will generate a 5 ms low pulse on GPIO2 when the RDSR
* 0Ah[15] bit is set.
*/void si4703_gp2(uint8_t set);

/*General Purpose I/O 1.
* 00 = High impedance (default).
* 01 = Reserved.
* 10 = Low.
* 11 = High.
*/void si4703_gp1(uint8_t set);

/*RSSI Seek Threshold. RSSI=Received Signal Strength Indicator
* 0x00 = min RSSI (default). most stops
* 0x7F = max RSSI. fewest stops
*/void si4703_seekth(uint8_t th);

/*Band Select.
* 00 = 87.5–108 MHz (USA,Europe) (Default).
* 01 = 76–108 MHz (Japan wide band).
* 10 = 76–90 MHz (Japan).
* 11 = Reserved.
*/void si4703_band(uint8_t band);

/*Channel Spacing.
* 00 = 200 kHz (USA, Australia) (default).
* 01 = 100 kHz (Europe, Japan).
* 10 = 50 kHz.
*/void si4703_space(uint8_t space);

/*Volume.
* Relative value of volume is shifted –30 dBFS with the VOLEXT 06h[8] bit.
* VOLEXT = 0 (default).
* 0000 = mute (default).
* 0001 = –28 dBFS.
* 1110 = –2 dBFS.
* 1111 = 0 dBFS.
*
* VOLEXT = 1.
* 0000 = mute.
* 0001 = –58 dBFS.
* 1110 = –32 dBFS.
* 1111 = –30 dBFS.
* FS = full scale.
* Volume scale is logarithmic.
*/void si4703_volume(uint8_t vol);

/*Softmute Attack/Recover Rate.
* 00 = fastest (default).
* 01 = fast.
* 10 = slow.
* 11 = slowest.
* To improve the listening experience when tuned to a non-existent station or one with poor SNR, the device
* provides a softmute feature which automatically reduces the volume significantly when the tuner detects that it isn't
* on a valid station. This feature can be disabled entirely by setting the DSMUTE bit. Additionally, this feature can be
* adjusted for how much it attenuates the volume (SMUTEA) as well as how quickly the attenuation is applied and
* removed (SMUTER).
*/void si4703_smuter(uint8_t rate);

/*Softmute Attenuation.
* 00 = 16 dB (default).
* 01 = 14 dB.
* 10 = 12 dB.
* 11 = 10 dB.
*/void si4703_smutea(uint8_t att);

/*RDS High-Performance Mode.
* 0 = disabled (default).
* 1 = enabled.
* The Si4703 offers an RDS high-performance mode for
* RDS-only applications such as TMC (traffic message
* channel) coupled with a GPS device. The RDS
* performance bit RDSPRF 06h[9] is disabled by default
* for backwards compatibility with previous RDS firmware
* releases. When RDSPRF is enabled the device
* increases power to the LNA, sets RDS to
* unconditionally remain enabled, and disables FM
* impulse detection, thereby avoiding RDS shutdown and
* allowing the device to continue to track and decode
* RDS in very poor SNR environments.
* Refer to "AN281: Si4700/01/02/03 Firmware Change List."
*/void si4703_rdsprf(bool enable);

/*Extended Volume Range.
* 0 = disabled (default).
* 1 = enabled.
* This bit attenuates the output by 30 dB. With the bit set to 0, the 15 volume settings
* adjust the volume between 0 and –28 dBFS. With the bit set to 1, the 15 volume set-
* tings adjust the volume between –30 and –58 dBFS.
* Refer to 4.5. "Stereo Audio Processing" on page 15.
* Refer to "AN281: Si4700/01/02/03 Firmware Change List."
*/void si4703_volext(bool enable);

/*Seek SNR Threshold. The Signal to Noise Ratio threshold for the seek operation
* 0000 = disabled (default).
* 0001 = min (most stops).
* 1111 = max (fewest stops).
* Required channel SNR for a valid seek channel.
*/void si4703_sksnr(uint8_t th);

/*Seek FM Impulse Detection Threshold.
* 0000 = disabled (default).
* 0001 = max (most stops).
* 1111 = min (fewest stops).
* Allowable number of FM impulses for a valid seek channel.
* FM Impulse noise occurs in all FM detectors when the SNR of a received station becomes very low and the
* received noise causes the FM detector to make instantaneous phase jumps, resulting in audible “clicks.” SKCNT
* sets the threshold for the number of FM impulses allowed on a tuned channel within a defined period.
*/void si4703_skcnt(uint8_t cnt);

/*Crystal Oscillator Enable.
* 0 = Disable (default).
* 1 = Enable.
* The internal crystal oscillator requires an external 32.768 kHz crystal as shown in
* "Typical Application Schematic" on page 13. The oscillator must be enabled before
* powerup (ENABLE = 1) as shown in Figure 9, “Initialization Sequence,” on page 20. It
* should only be disabled after powerdown (ENABLE = 0). Bits 13:0 of register 07h
* must be preserved as 0x0100 while in powerdown and as 0x3C04 while in powerup.
* Refer to Si4702/03 Internal Crystal Oscillator Errata.
*/void si4703_xoscen(bool enable);

/*Audio High-Z Enable.
* 0 = Disable (default).
* 1 = Enable.
* Setting AHIZEN maintains a dc bias of 0.5 x VIO on the LOUT and ROUT pins to pre-
* vent the ESD diodes from clamping to the VIO or GND rail in response to the output
* swing of another device. Register 07h containing the AHIZEN bit must not be written
* during the powerup sequence and high-Z only takes effect when in powerdown and
* VIO is supplied. Bits 13:0 of register 07h must be preserved as 0x0100 while in pow-
* erdown and as 0x3C04 while in powerup
*/void si4703_ahizen(bool enable);

#endif
