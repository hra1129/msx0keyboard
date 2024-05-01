// --------------------------------------------------------------------
//	MSX0 Keyboard key scan
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
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// --------------------------------------------------------------------

#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <cstring>
#include "msx0kbscan.h"

// --------------------------------------------------------------------
//	MSX Keymatrix
//		MSB   7   6   5   4   3   2   1   0    LSB
//		+0   [7 ][6 ][5 ][4 ][3 ][2 ][1 ][0 ]
//		+1   [+ ][{ ][@ ][\ ][^ ][- ][9 ][8 ]
//		+2   [B ][A ][_ ][/ ][. ][, ][} ][* ]
//		+3   [J ][I ][H ][G ][F ][E ][D ][C ]
//		+4   [R ][Q ][P ][O ][N ][M ][L ][K ]
//		+5   [Z ][Y ][X ][W ][V ][U ][T ][S ]
//		+6   [F3][F2][F1][KN][CP][GR][CT][SH]
//		+7   [RT][SE][BS][ST][TB][ES][F5][F4]
//		+8   [RI][DN][UP][LE][DE][IN][HO][SP]
//		+9   [T4][T3][T2][T1][T0][  ][  ][  ]
//		+10  [T,][T.][T-][T9][T8][T7][T6][T5]
//		+11  [  ][  ][A7][A6][A4][A3][A2][A1]    JOYPAD1
//		+12  [  ][  ][B7][B6][B4][B3][B2][B1]    JOYPAD2
//
// --------------------------------------------------------------------
//	Keyboard ROW signals Y0...Y8
//		Y0 : GPIO0
//		Y1 : GPIO1
//		Y2 : GPIO2
//		Y3 : GPIO3
//		Y4 : GPIO4
//		Y5 : GPIO5
//		Y6 : GPIO6
//		Y7 : GPIO7
//		Y8 : GPIO8
//		Y9 : GPIO26
//		Y10: GPIO27
//	Keyboard COLUMN signals X0...X7
//		X0: GPIO9
//		X1: GPIO10
//		X2: GPIO11
//		X3: GPIO12
//		X4: GPIO13
//		X5: GPIO14
//		X6: GPIO15
//		X7: GPIO16
//	Others
//		KEY_INT: GPIO17
//		INT_SCA: GPIO18		※この class では処理しない
//		INT_SCL: GPIO19		※この class では処理しない
//
#define CVT( msx_y, msx_x )		( ((msx_y) << 8) | ((1 << (msx_x)) ^ 0xFF) )
#define CVT_X( convert )		( (convert) & 0xFF )
#define CVT_Y( convert )		( (convert) >> 8 )

//	MSX0 Keyboard のマトリクスから MSXキーマトリクスの ROW/COLUM を得るためのテーブル
static const int convert_table[ /* Y */ ][ 8 /* X */ ] = {
	//    F1        F2        F3        F4        F5        ST        HO        SE
	{ CVT( 6,5),CVT( 6,6),CVT( 6,7),CVT( 7,0),CVT( 7,1),CVT( 7,4),CVT( 8,1),CVT( 7,6) },
	//    9         8         0         -         ^         ￥        IN        DE
	{ CVT( 1,0),CVT( 1,1),CVT( 0,0),CVT( 1,2),CVT( 1,3),CVT( 1,4),CVT( 8,2),CVT( 8,3) },
	//    ES        1         2         3         4         5         6         7
	{ CVT( 7,2),CVT( 0,1),CVT( 0,2),CVT( 0,3),CVT( 0,4),CVT( 0,5),CVT( 0,6),CVT( 0,7) },
	//    TB        Q         W         E         R         T         Y         U
	{ CVT( 7,3),CVT( 4,6),CVT( 5,4),CVT( 3,2),CVT( 4,7),CVT( 5,1),CVT( 5,6),CVT( 5,2) },
	//    I         O         P         @         {         BS        }         RT
	{ CVT( 3,6),CVT( 4,4),CVT( 4,5),CVT( 1,5),CVT( 1,6),CVT( 7,5),CVT( 2,1),CVT( 7,7) },
	//    UP        CT        A         S         D         F         G         H
	{ CVT( 8,5),CVT( 6,1),CVT( 2,6),CVT( 5,0),CVT( 3,1),CVT( 3,3),CVT( 3,4),CVT( 3,5) },
	//    J         K         L         +         .         ,         M         N
	{ CVT( 3,7),CVT( 4,0),CVT( 4,1),CVT( 1,7),CVT( 2,3),CVT( 2,2),CVT( 4,2),CVT( 4,3) },
	//    LE        RI        SH        Z         X         C         V         B
	{ CVT( 8,4),CVT( 8,7),CVT( 6,0),CVT( 5,7),CVT( 5,5),CVT( 3,0),CVT( 5,3),CVT( 2,7) },
	//    DN        CP        GR        SP        KN        /         _         *
	{ CVT( 8,6),CVT( 6,3),CVT( 6,2),CVT( 8,0),CVT( 6,4),CVT( 2,4),CVT( 2,5),CVT( 2,0) },
	//    RSV       RSV       A7        A6        A4        A3        A2        A1
	{ -1,       -1,       CVT(11,5),CVT(11,4),CVT(11,3),CVT(11,2),CVT(11,1),CVT(11,0) },
	//    RSV       RSV       B7        B6        B4        B3        B2        B1
	{ -1,       -1,       CVT(12,5),CVT(12,4),CVT(12,3),CVT(12,2),CVT(12,1),CVT(12,0) },
};

static const int gpio_x[] = { 9, 10, 11, 12, 13, 14, 15, 16 };
static const int gpio_y[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 26, 27 };

// ----------------------------------------------------------------
//	begin()
//		Initialization process to use the keyboard.
//		キーボードを利用するための初期化処理。
//
void CMSX0KBSCAN::begin( void ) {
	int i;

	//	全キー押されていない状態にクリア
	std::memset( this->keymatrix, 0xFF, sizeof(this->keymatrix) );

	//	GPIOを初期化する (Y, X)
	for( i = 0; i < sizeof(gpio_x)/sizeof(gpio_x[0]); i++ ) {
		gpio_init( gpio_x[i] );
		gpio_set_dir( gpio_x[i], false );	// input
		gpio_pull_up( gpio_x[i] );			// pull up
	}
	for( i = 0; i < sizeof(gpio_y)/sizeof(gpio_y[0]); i++ ) {
		gpio_init( gpio_y[i] );
		gpio_set_dir( gpio_y[i], false );	// input
		gpio_pull_up( gpio_y[i] );			// pull up
	}
}

// ----------------------------------------------------------------
//	update()
//		Update key matrix information.
//		キーマトリクス情報を更新する。
//
bool CMSX0KBSCAN::update( void ) {
	int x, y, gy;
	uint8_t previous_matrix[ sizeof( this->keymatrix ) ];
	uint32_t keys;
	int convert, msx_y, msx_x;

	//	直前の状態をバックアップ
	std::memcpy( previous_matrix, this->keymatrix, sizeof( this->keymatrix ) );
	//	キー状態を更新
	std::memset( this->keymatrix, 0xFF, sizeof( this->keymatrix ) );
	for( y = 0; y < 11; y++ ) {
		//	ROW を L にする
		gy = gpio_y[y];
		gpio_put( gy, 0 );
		gpio_set_dir( gy, true );	// output
		sleep_us(10);
		//	X を読む
		keys = (gpio_get_all() >> 9) & 0xFF;
		for( x = 0; x < 8; x++ ) {
			if( (keys & 1) == 0 ) {
				//	対応するキーが押されている場合
				convert = convert_table[y][x];
				msx_x = CVT_X( convert );
				msx_y = CVT_Y( convert );
				this->keymatrix[ msx_y ] &= msx_x;
			}
			keys >>= 1;
		}
		//	ROW を HiZ に戻す
		gpio_set_dir( gy, false );	// input
		sleep_us(10);
	}
	for( y = 0; y < 13; y++ ) {
		if( this->keymatrix[ y ] != previous_matrix[ y ] ) return true;
	}
	return false;
}
