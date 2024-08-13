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
#include "face2keymap.h"

#define I2C_ADDR		0x08						//	I2Cアドレス
#define I2C_SDA			18							//	SDAにはGPIO18を使用する (※I2C1)
#define I2C_SCL			19							//	SCLにはGPIO19を使用する (※I2C1)
#define KB_INTR			17
#define GPIO_LED		25
#define GPIO_LED_CAPS	21
#define GPIO_LED_KANA	20

static CF2KEY cf2key;

static CMSX0KBSCAN kbscan;
static uint8_t keymap[14]		= { 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t init_keymap[14]	= { 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t msx_keymap[13]	= { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t last_request[2]	= { 0, 0 };

static int count = 0;
static int mode = 0;		//	0: FaceII Keyboard compatible mode, 1: MSX Keyboard mode

static bool led_caps = false;
static bool led_kana = false;
static bool is_negosiate = true;

// --------------------------------------------------------------------
//	MSX0からの読み出し
static void on_request() {
	int i;
	uint8_t d;
	static CSEND_DATA send_data;

	switch( last_request[0] ) {
	case 0xF0:
		break;
	case 0xFE:
		//	デバイスタイプ要求
		Wire1.write( 0xA2 );		//	MSX0キーボード
		break;
	default:
	case 0xF1:
		//	FaceII Keyboard compatible mode -----------------------
		if( cf2key.get_send_data_count() <= 1 ) {
			gpio_put( KB_INTR, 1 );
		}
		mode = 0;
		is_negosiate = false;
		//	受信可能タイミング＋装飾キー状態通知
		if( (last_request[1] & 0x80) != 0 ) {
			//	装飾キー状態を取り込む
			led_caps	= ((last_request[1] & 0x10) != 0);
			led_kana	= cf2key.get_kana_mode();
		}
		send_data = cf2key.get_send_data();

		for( i = 0; i < 10; i++ ) {
			Wire1.write( send_data.data[i] );
			DEBUG( Serial.printf( "%02X,", send_data.data[i] ) );
		}
		DEBUG( Serial.printf( "[%d]\n", (int)cf2key.get_send_data_count() ) );
		break;
	case 0xF2:
		//	MSX Keyboard mode -------------------------------------
		gpio_put( KB_INTR, 1 );
		if( mode == 0 ) {
			cf2key.clear();
		}
		mode = 1;
		is_negosiate = false;
		//	受信可能タイミング＋装飾キー状態通知
		if( (last_request[1] & 0x80) != 0 ) {
			//	装飾キー状態を取り込む
			led_caps	= ((last_request[1] & 0x10) != 0);
			led_kana	= ((last_request[1] & 0x20) != 0);
		}
		for( i = 0; i < sizeof(keymap); i++ ) {
			Wire1.write( keymap[i] );
		}
		gpio_put( GPIO_LED, 0 );
		break;
	}
}

// --------------------------------------------------------------------
//	MSX0からの書き込み
static void on_receive( int len ) {
	int i;

	i = 0;
	//	書き込まれた値の先頭 2byte を last_request[] に保存する
	while( Wire1.available() ) {
		if( i < 2 ) {
			last_request[i] = Wire1.read();
			i++;
		}
	}
	if( last_request[0] == 0xF0 ) {
		//	最初の開始コマンドだった場合、こちらも初期化する
		DEBUG( Serial.printf( "Recv: 0xF0\n" ) );
		led_caps	= false;
		led_kana	= false;
		is_negosiate = true;
		std::memcpy( keymap, init_keymap, sizeof(keymap) );
		cf2key.clear();
		cf2key.set_initial_matrix();
		gpio_put( KB_INTR, 0 );
	}
}

// --------------------------------------------------------------------
static void led_control( void ) {

	count = (count + 1) & 31;
	
	if( is_negosiate && ((count & 4) == 0) ) {
		//	1で点灯
		gpio_put( GPIO_LED, 1 );
	}
	else if( (mode == 0) && ((count & 16) == 0) ) {
		//	1で点灯
		gpio_put( GPIO_LED, 1 );
	}
	else {
		//	0で消灯
		gpio_put( GPIO_LED, 0 );
	}

	if( led_caps ) {
		//	0で点灯
		gpio_put( GPIO_LED_CAPS, 0 );
	}
	else {
		//	1で消灯
		gpio_put( GPIO_LED_CAPS, 1 );
	}

	if( led_kana ) {
		//	0で点灯
		gpio_put( GPIO_LED_KANA, 0 );
	}
	else {
		//	1で消灯
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

	DEBUG( Serial.begin( 115200 ) );
	DEBUG( Serial.print( "Start MSX0 Keyboard\n" ) );

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

	//	LEDを制御する
	led_control();

	//	キーマトリクスを送信待ちであれば何もせずに戻る
	if( cf2key.get_send_data_count() ) {
		delay( 4 );
		return;
	}
	//	現時点のキーマトリクス状態を調べる
	if( !kbscan.update() ) {
		//	キーマトリクスが前回送信のマトリクスから変化していなければ何もしない
		delay( 4 );
		return;
	}
	DEBUG( Serial.printf( "Key scan changed\n" ) );
	//	更新したキーマトリクス(MSXタイプ)を取得
	p_key = kbscan.get();
	//	FaceII Keyboard compatible mode -----------------------
	cf2key.begin();
	std::memcpy( msx_keymap, p_key, sizeof(msx_keymap) );
	cf2key.regist_msx_key( msx_keymap );
	cf2key.end();
	//	MSX Keyboard mode -------------------------------------
	std::memcpy( keymap + 1, p_key, sizeof(keymap) - 1 );
	gpio_put( KB_INTR, 0 );
}
