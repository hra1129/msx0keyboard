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
//  THE SOFTWARE IS PROVIDED “AS IS�?, WITHOUT WARRANTY OF ANY KIND, 
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

#define I2C_ADDR		0x08						//	I2Cアドレス
#define I2C_SDA			18							//	SDAにはGPIO18を使用する (※I2C1)
#define I2C_SCL			19							//	SCLにはGPIO19を使用する (※I2C1)
#define KB_INTR			17
#define GPIO_LED		25
#define GPIO_LED_CAPS	21
#define GPIO_LED_KANA	20

static CMSX0KBSCAN kbscan;
static uint8_t keymap[14] = { 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t init_keymap[14] = { 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t msx_keymap[13] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t last_request[2] = { 0, 0 };
static int count = 0;
static bool led_caps = false;
static bool led_kana = false;
static volatile bool is_sending = false;

// --------------------------------------------------------------------
//	MSX0からの読み出し
static void on_request() {
	int i;
	uint8_t d;

	switch( last_request[0] ) {
	case 0xF0:
		break;
	case 0xFE:
		//	デバイスタイプ要求
		Wire1.write( 0xA3 );		//	新プロトコル MSX0キーボード
		break;
	default:
	case 0xF1:
		//	受信可能タイミング＋装飾キー状態通知
		if( (last_request[1] & 0x80) != 0 ) {
			//	装飾キー状態を取り込む
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
//	MSX0からの書き込み
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
		//	通信開始要求
		is_sending	= true;
		led_caps	= false;
		led_kana	= false;
		std::memcpy( keymap, init_keymap, sizeof(keymap) );
		gpio_put( KB_INTR, 0 );
		gpio_put( GPIO_LED, 1 );
	}
}

// --------------------------------------------------------------------
static void led_control( void ) {

	if( led_caps ) {
		gpio_put( GPIO_LED_CAPS, 0 );
	}
	else {
		gpio_put( GPIO_LED_CAPS, 1 );
	}
	if( led_kana ) {
		gpio_put( GPIO_LED_KANA, 0 );
	}
	else {
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

	//I2Cスレーブ設定
	Wire1.setSDA( I2C_SDA );
	Wire1.setSCL( I2C_SCL );
	Wire1.onReceive( on_receive );
	Wire1.onRequest( on_request );
	Wire1.begin( I2C_ADDR );
}

// --------------------------------------------------------------------
void loop() {
	const uint8_t *p_key;
	
	led_control();

	//	キーマトリクスを送信待ちであれば何もせずに戻る
	if( is_sending ) {
		delay( 1 );
		return;
	}
	//	現時点のキーマトリクス状態を調べる
	if( !kbscan.update() ) {
		//	キーマトリクスが前回送信のマトリクスから変化していなければ何もしない
		delay( 4 );
		return;
	}
	//	更新したキーマトリクス(MSXタイプ)を取得
	p_key = kbscan.get();
	std::memcpy( keymap + 1, p_key, sizeof(keymap) - 1 );
	// 割り込み
	is_sending = true;
	gpio_put( KB_INTR, 0 );
	gpio_put( GPIO_LED, 1 );
	delay( 4 );
}
