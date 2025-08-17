**Der Radiowecker mit RDS, sechs Farbmodi und vier Seek-Empfindlichkeiten**\
*Hardware*
 * ili9341-Display_320x240, spi
 * si4703-FMRadio, i2c
 * ds1302-RTC, 3wire
 * 5-Wege-Button, COM on Ground
* Radio ON/OFF digital-out on GP_20/GP_2, for PowerOn/Off-switch an Amplifier
  
*Pins*
* PIN_RADIO_ON/OFF for org GP_20 / for zero GP_2 out for switch
 * RTC SCL for org GP_21 / for zero GP_3
 * RTC IO for org GP_2 / for zero GP_4
 * RTC CS for org GP_3 / for zero GP_5
 * BUTTON_M GP_6
 * display DC GP_7
 * display reset GP_8
 * display cs/csn GP_9
 * display SCK/SPI_SCK GP_10
 * display SDIO/SPI_TX GP_11
 * BUTTON_U GP_12
 * BUTTON_D GP_13
 * BUTTON_L GP_14
 * BUTTON_R GP_15
 * si4703 SDA GP_16 / for zero GP_26
 * si4703 SCL GP_17 / for zero GP_27
 * si4703 RES GP_18 / for zero GP_28
 * si4703 GP2 GP_19 / for zero GP_29
