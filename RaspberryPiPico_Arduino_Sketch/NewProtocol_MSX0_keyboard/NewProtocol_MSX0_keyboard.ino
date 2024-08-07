// --------------------------------------------------------------------
//	MSX0 Keyboard
// ====================================================================
// Copyright 2024 Takayuki Hara
//
//  Permission is hereby granted, free of charge, to any person obtaining 
// a copy of this software and associated documentation files face2keymap, 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED âAS ISâ?, WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// --------------------------------------------------------------------

#include <hardware/gpio.h>
#include <Wire.h>
#include <cstring>

#include "msx0kbscan.h"
#include "face2keymap.h"

#define I2C_ADDR		0x08						//	I2Cã¢ãã¬ã¹
#define I2C_SDA			18							//	SDAã«ã¯GPIO18ãä½¿ç¨ãã (â»I2C1)
#define I2C_SCL			19							//	SCLã«ã¯GPIO19ãä½¿ç¨ãã (â»I2C1)
#define KB_INTR			17
#define GPIO_LED		25
#define GPIO_LED_CAPS	21
#define GPIO_LED_KANA	20

static CF2KEY cf2key;
static uint8_t f2k_keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0xFF };
//static uint8_t f2k_init_keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0x71 };		//	æ¨æº
static uint8_t f2k_init_keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB1, 0x1F, 0x70 };		//	æ°ãã­ãã³ã«å¯¾å¿
static uint8_t f2k_next_keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0xFF };

static CMSX0KBSCAN kbscan;
static uint8_t keymap[14] = { 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t init_keymap[14] = { 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t msx_keymap[13] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t last_request[2] = { 0, 0 };

static int count = 0;
static int mode = 0;		//	0: FaceII Keyboard compatible mode, 1: MSX Keyboard mode

static bool led_caps = false;
static bool led_kana = false;
static volatile bool is_shift = false;
static volatile bool is_first = true;
static volatile bool is_sending = false;

// --------------------------------------------------------------------
//	MSX0ããã®èª­ã¿åºã
static void on_request() {
	int i;
	uint8_t d;

	switch( last_request[0] ) {
	case 0xF0:
		break;
	case 0xFE:
		//	ããã¤ã¹ã¿ã¤ãè¦æ±
		Wire1.write( 0xA2 );		//	æ°ãã­ãã³ã« MSX0ã­ã¼ãã¼ã
		break;
	default:
	case 0xF1:
		//	FaceII Keyboard compatible mode -----------------------
		mode = 0;
		//	åä¿¡å¯è½ã¿ã¤ãã³ã°ï¼è£é£¾ã­ã¼ç¶æéç¥
		if( (last_request[1] & 0x80) != 0 ) {
			//	è£é£¾ã­ã¼ç¶æãåãè¾¼ã
			led_caps	= ((last_request[1] & 0x10) != 0);
			led_kana	= ((last_request[1] & 0x20) != 0);
		}
		gpio_put( KB_INTR, 1 );
		for( i = 0; i < sizeof(f2k_keymap); i++ ) {
			Wire1.write( f2k_keymap[i] );
		}
		if( is_shift ) {
			std::memcpy( f2k_keymap, f2k_next_keymap, sizeof(f2k_keymap) );
			is_shift = false;
		}
		else {
			is_sending	= false;
			is_first = false;
		}
		break;
	case 0xF2:
		//	MSX Keyboard mode -------------------------------------
		mode = 1;
		//	åä¿¡å¯è½ã¿ã¤ãã³ã°ï¼è£é£¾ã­ã¼ç¶æéç¥
		if( (last_request[1] & 0x80) != 0 ) {
			//	è£é£¾ã­ã¼ç¶æãåãè¾¼ã
			led_caps	= ((last_request[1] & 0x10) != 0);
			led_kana	= ((last_request[1] & 0x20) != 0);
		}
		for( i = 0; i < sizeof(keymap); i++ ) {
			Wire1.write( keymap[i] );
		}
		is_sending	= false;
		gpio_put( KB_INTR, 1 );
		gpio_put( GPIO_LED, 0 );
		break;
	}
}

// --------------------------------------------------------------------
//	MSX0ããã®æ¸ãè¾¼ã¿
static void on_receive( int len ) {
	int i;

	i = 0;
	while( Wire1.available() ) {
		if( i < 2 ) {
			last_request[i] = Wire1.read();
			i++;
		}
	}
	if( last_request[0] == 0xF0 ) {
		//	éä¿¡éå§è¦æ±
		is_first	= true;
		is_shift	= false;
		is_sending	= true;
		led_caps	= false;
		led_kana	= false;
		std::memcpy( keymap, init_keymap, sizeof(keymap) );
		std::memcpy( f2k_keymap, f2k_init_keymap, sizeof(f2k_keymap) );
		gpio_put( KB_INTR, 0 );
		gpio_put( GPIO_LED, 1 );
	}
}

// --------------------------------------------------------------------
static void led_control( void ) {

	count = (count + 1) & 31;
	
	if( is_first && ((count & 4) == 0) ) {
		//	1ã§ç¹ç¯
		gpio_put( GPIO_LED, 1 );
	}
	else if( (mode == 0) && ((count & 16) == 0) ) {
		//	1ã§ç¹ç¯
		gpio_put( GPIO_LED, 1 );
	}
	else {
		//	0ã§æ¶ç¯
		gpio_put( GPIO_LED, 0 );
	}

	if( led_caps ) {
		//	0ã§ç¹ç¯
		gpio_put( GPIO_LED_CAPS, 0 );
	}
	else {
		//	1ã§æ¶ç¯
		gpio_put( GPIO_LED_CAPS, 1 );
	}

	if( led_kana ) {
		//	0ã§ç¹ç¯
		gpio_put( GPIO_LED_KANA, 0 );
	}
	else {
		//	1ã§æ¶ç¯
		gpio_put( GPIO_LED_KANA, 1 );
	}
}

// --------------------------------------------------------------------
void setup() {

	gpio_init( KB_INTR );
	gpio_init( GPIO_LED );
	gpio_init( GPIO_LED_CAPS );
	gpio_init( GPIO_LED_KANA );

	gpio_set_dir( KB_INTR, true );
	gpio_set_dir( GPIO_LED, true );
	gpio_set_dir( GPIO_LED_CAPS, true );
	gpio_set_dir( GPIO_LED_KANA, true );

	gpio_put( KB_INTR, 1 );
	gpio_put( GPIO_LED, 0 );
	gpio_put( GPIO_LED_CAPS, 1 );
	gpio_put( GPIO_LED_KANA, 1 );

	kbscan.begin();

	//I2Cã¹ã¬ã¼ãè¨­å®
	Wire1.setSDA( I2C_SDA );
	Wire1.setSCL( I2C_SCL );
	Wire1.onReceive( on_receive );
	Wire1.onRequest( on_request );
	Wire1.begin( I2C_ADDR );
}

// --------------------------------------------------------------------
void loop() {
	const uint8_t *p_key, *p_key1, *p_key2;
	
	led_control();

	//	ã­ã¼ãããªã¯ã¹ãéä¿¡å¾ã¡ã§ããã°ä½ãããã«æ»ã
	if( is_sending ) {
		delay( 1 );
		gpio_put( KB_INTR, 0 );
		return;
	}
	//	ç¾æç¹ã®ã­ã¼ãããªã¯ã¹ç¶æãèª¿ã¹ã
	if( !kbscan.update() ) {
		//	ã­ã¼ãããªã¯ã¹ãååéä¿¡ã®ãããªã¯ã¹ããå¤åãã¦ããªããã°ä½ãããªã
		delay( 4 );
		return;
	}
	//	æ´æ°ããã­ã¼ãããªã¯ã¹(MSXã¿ã¤ã)ãåå¾
	p_key = kbscan.get();
	std::memcpy( keymap + 1, p_key, sizeof(keymap) - 1 );
	std::memcpy( msx_keymap, p_key, sizeof(msx_keymap) );
	//	FaceIIã¿ã¤ãã­ã¼ãããªã¯ã¹ã¸å¤æ
	cf2key.begin();
	cf2key.regist_msx_key( msx_keymap );
	p_key1 = cf2key.end();
	//	ã·ããã­ã¼ç¶æã«å¤æ´ããã£ããèª¿ã¹ã
	p_key2 = cf2key.get_shift_key();
	if( p_key2 == NULL ) {
		//	å¤æ´ãç¡ãã£ãå ´åã¯ãæ°ãã FaceIIã¿ã¤ãã­ã¼ãããªã¯ã¹ãæ¡ç¨
		std::memcpy( f2k_keymap, p_key1, sizeof(f2k_keymap) );
	}
	else {
		//	å¤æ´ããã£ãå ´åã¯ãã¾ãã·ããã­ã¼ã®å¤æ´ãéç¥ããæ°ãã FaceIIã¿ã¤ããããªã¯ã¹ã¯ããã¯ã¢ãããã¨ã£ã¦ãã
		is_shift = true;
		std::memcpy( f2k_keymap, p_key2, sizeof(f2k_keymap) );
		std::memcpy( f2k_next_keymap, p_key1, sizeof(f2k_next_keymap) );
	}
	//	å²ãè¾¼ã¿
	is_sending = true;
	gpio_put( KB_INTR, 0 );
	gpio_put( GPIO_LED, 1 );
	delay( 4 );
}
