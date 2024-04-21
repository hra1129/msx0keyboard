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

#include "msx0kbscan.h"
#include "face2keymap.h"

#define I2C_ADDR	0x08							//	I2Cアドレス
#define I2C_SDA		18								//	SDAにはGPIO18を使用する (※I2C1)
#define I2C_SCL		19								//	SCLにはGPIO19を使用する (※I2C1)
#define KB_INTR		17
#define GPIO_LED	25

static CMSX0KBSCAN kbscan;
static CF2KEY cf2key;
static uint8_t keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0xFF };
static uint8_t init_keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0x71 };
static uint8_t next_keymap[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0xFF };
static uint8_t msx_keymap[11] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t last_request[2] = { 0, 0 };
static int count = 0;
static bool led_shift = false;
static bool led_sym = false;
static volatile bool is_shift = false;
static volatile bool is_first = true;
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
		Wire1.write( 0xA2 );		//	FaceIIキーボード (MSX0クラファン版付属バージョン)
		break;
	default:
	case 0xF1:
		//	受信可能タイミング＋装飾キー状態通知
		if( (last_request[1] & 0x80) != 0 ) {
			//	装飾キー状態を取り込む
			led_shift	= ((last_request[1] & 0x10) != 0);
			led_sym		= ((last_request[1] & 0x20) != 0);
		}
		if( !is_shift ) {
			gpio_put( KB_INTR, 1 );
		}
		for( i = 0; i < sizeof(keymap); i++ ) {
			Wire1.write( keymap[i] );
		}
		if( is_shift ) {
			std::memcpy( keymap, next_keymap, sizeof(keymap) );
			is_shift = false;
		}
		else {
			is_sending	= false;
			is_first = false;
		}
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
		is_first	= true;
		is_shift	= false;
		is_sending	= true;
		led_shift	= false;
		led_sym		= false;
		std::memcpy( keymap, init_keymap, sizeof(keymap) );
		gpio_put( KB_INTR, 0 );
	}
}

// --------------------------------------------------------------------
static void led_control( void ) {

	count = (count + 1) & 16;

	if( is_first ) {
		gpio_put( GPIO_LED, (count & 4) != 0 );
	}
	else if( led_shift ) {
		gpio_put( GPIO_LED, 1 );
	}
	else if( led_sym ) {
		gpio_put( GPIO_LED, count >= 8 );
	}
	else {
		gpio_put( GPIO_LED, 0 );
	}
}

// --------------------------------------------------------------------
void setup() {

	gpio_init( KB_INTR );
	gpio_set_dir( KB_INTR, true );
	gpio_put( KB_INTR, 1 );

	gpio_init( GPIO_LED );
	gpio_set_dir( GPIO_LED, true );
	gpio_put( GPIO_LED, 1 );

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
	const uint8_t *p_key1, *p_key2;
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
	p_key1 = kbscan.get();
	std::memcpy( msx_keymap, p_key1, sizeof(msx_keymap) );
	//	FaceIIタイプキーマトリクスへ変換
	cf2key.begin();
	cf2key.regist_msx_key( msx_keymap );
	p_key1 = cf2key.end();
	//	シフトキー状態に変更があったか調べる
	p_key2 = cf2key.get_shift_key();
	if( p_key2 == NULL ) {
		//	変更が無かった場合は、新しい FaceIIタイプキーマトリクスを採用
		std::memcpy( keymap, p_key1, sizeof(keymap) );
	}
	else {
		//	変更があった場合は、まずシフトキーの変更を通知し、新しい FaceIIタイプマトリクスはバックアップをとっておく
		is_shift = true;
		std::memcpy( keymap, p_key2, sizeof(keymap) );
		std::memcpy( next_keymap, p_key1, sizeof(next_keymap) );
	}
	// 割り込み
	is_sending = true;
	gpio_put( KB_INTR, 0 );
	delay( 4 );
}
