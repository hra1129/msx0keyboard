// --------------------------------------------------------------------
//	FaceII Keymap generator
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

#include "face2keymap.h"
#include <Arduino.h>
#include <cstring>

// --------------------------------------------------------------------
//	Keymatrix
//		LSB   0   1   2   3   4   5   6   7    MSB
//		+0   [0 ][1 ][2 ][3 ][4 ][5 ][6 ][7 ]
//		+1   [8 ][9 ][- ][^ ][￥][@ ][{ ][+ ]
//		+2   [* ][} ][, ][. ][/ ][_ ][A ][B ]
//		+3   [C ][D ][E ][F ][G ][H ][I ][J ]
//		+4   [K ][L ][M ][N ][O ][P ][Q ][R ]
//		+5   [S ][T ][U ][V ][W ][X ][Y ][Z ]
//		+6   [SH][CT][GR][CP][KN][F1][F2][F3]
//		+7   [F4][F5][ES][TB][ST][BS][SE][RT]
//		+8   [SP][HO][IN][DE][LE][UP][DN][RI]
//		+9   [  ][  ][  ][T0][T1][T2][T3][T4]
//		+10  [T5][T6][T7][T8][T9][T-][T.][T,]
//
#define MK( cf2k, priority ) ( (cf2k) | ((priority) << 24) )

static const int convert_table[ 2 /* shift */ ][ 11 /* Y */ ][ 8 /* X */ ] = {
	{	//	NORMAL
		{ MK(CF2K_0,CF2KP_L),     MK(CF2K_1,CF2KP_L),     MK(CF2K_2,CF2KP_L),     MK(CF2K_3,CF2KP_L),    MK(CF2K_4,CF2KP_L),     MK(CF2K_5,CF2KP_L),    MK(CF2K_6,CF2KP_L),     MK(CF2K_7,CF2KP_L)    },
		{ MK(CF2K_8,CF2KP_L),     MK(CF2K_9,CF2KP_L),     MK(CF2K_MINUS,CF2KP_L), MK(CF2K_HAT,CF2KP_L),  MK(CF2K_BSLA,CF2KP_L),  MK(CF2K_AT,CF2KP_L),   MK(CF2K_BLBRA,CF2KP_L), MK(CF2K_SEMI,CF2KP_L) },
		{ MK(CF2K_COLON,CF2KP_L), MK(CF2K_BRBRA,CF2KP_L), MK(CF2K_COMMA,CF2KP_L), MK(CF2K_DOT,CF2KP_L),  MK(CF2K_SLASH,CF2KP_L), MK(CF2K_BSLA,CF2KP_L), MK(CF2K_A,CF2KP_L),     MK(CF2K_B,CF2KP_L)    },
		{ MK(CF2K_C,CF2KP_L),     MK(CF2K_D,CF2KP_L),     MK(CF2K_E,CF2KP_L),     MK(CF2K_F,CF2KP_L),    MK(CF2K_G,CF2KP_L),     MK(CF2K_H,CF2KP_L),    MK(CF2K_I,CF2KP_L),     MK(CF2K_J,CF2KP_L)    },
		{ MK(CF2K_K,CF2KP_L),     MK(CF2K_L,CF2KP_L),     MK(CF2K_M,CF2KP_L),     MK(CF2K_N,CF2KP_L),    MK(CF2K_O,CF2KP_L),     MK(CF2K_P,CF2KP_L),    MK(CF2K_Q,CF2KP_L),     MK(CF2K_R,CF2KP_L)    },
		{ MK(CF2K_S,CF2KP_L),     MK(CF2K_T,CF2KP_L),     MK(CF2K_U,CF2KP_L),     MK(CF2K_V,CF2KP_L),    MK(CF2K_W,CF2KP_L),     MK(CF2K_X,CF2KP_L),    MK(CF2K_Y,CF2KP_L),     MK(CF2K_Z,CF2KP_L)    },
		{ MK(CF2K_ALT,CF2KP_L),   -1,                     -1,                     MK(CF2K_CAPS,CF2KP_L), MK(CF2K_KANA,CF2KP_H),  -1,                    -1,                     -1                    },
		{ -1,                     -1,                     MK(CF2K_ESC,CF2KP_L),   MK(CF2K_TAB,CF2KP_L),  MK(CF2K_END,CF2KP_L),   MK(CF2K_BS,CF2KP_H),   -1,                     MK(CF2K_RT,CF2KP_H)   },
		{ MK(CF2K_SP,CF2KP_H),    MK(CF2K_HOME,CF2KP_H),  MK(CF2K_INS,CF2KP_H),   MK(CF2K_DEL,CF2KP_M),  MK(CF2K_LE,CF2KP_H),    MK(CF2K_UP,CF2KP_H),   MK(CF2K_DN,CF2KP_H),    MK(CF2K_RI,CF2KP_H)   },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
	},
	{	//	with SHIFT
		{ -1,                     MK(CF2K_EXC,CF2KP_L),   MK(CF2K_DQUAT,CF2KP_L), MK(CF2K_SHARP,CF2KP_L), MK(CF2K_DL,CF2KP_L),    MK(CF2K_PARC,CF2KP_L), MK(CF2K_AMP,CF2KP_L),   MK(CF2K_SQUAT,CF2KP_L) },
		{ MK(CF2K_MLBRA,CF2KP_L), MK(CF2K_MRBRA,CF2KP_L), MK(CF2K_EQU,CF2KP_L),   MK(CF2K_CHILD,CF2KP_L), MK(CF2K_BAR,CF2KP_L),   MK(CF2K_APOS,CF2KP_L), MK(CF2K_LLBRA,CF2KP_L), MK(CF2K_PLUS,CF2KP_L)  },
		{ MK(CF2K_ASTA,CF2KP_L),  MK(CF2K_LRBRA,CF2KP_L), MK(CF2K_LT,CF2KP_L),    MK(CF2K_GT,CF2KP_L),    MK(CF2K_QUEST,CF2KP_L), MK(CF2K__,CF2KP_L),    MK(CF2K_A,CF2KP_L),     MK(CF2K_B,CF2KP_L)     },
		{ MK(CF2K_C,CF2KP_L),     MK(CF2K_D,CF2KP_L),     MK(CF2K_E,CF2KP_L),     MK(CF2K_F,CF2KP_L),     MK(CF2K_G,CF2KP_L),     MK(CF2K_H,CF2KP_L),    MK(CF2K_I,CF2KP_L),     MK(CF2K_J,CF2KP_L)     },
		{ MK(CF2K_K,CF2KP_L),     MK(CF2K_L,CF2KP_L),     MK(CF2K_M,CF2KP_L),     MK(CF2K_N,CF2KP_L),     MK(CF2K_O,CF2KP_L),     MK(CF2K_P,CF2KP_L),    MK(CF2K_Q,CF2KP_L),     MK(CF2K_R,CF2KP_L)     },
		{ MK(CF2K_S,CF2KP_L),     MK(CF2K_T,CF2KP_L),     MK(CF2K_U,CF2KP_L),     MK(CF2K_V,CF2KP_L),     MK(CF2K_W,CF2KP_L),     MK(CF2K_X,CF2KP_L),    MK(CF2K_Y,CF2KP_L),     MK(CF2K_Z,CF2KP_L)     },
		{ MK(CF2K_ALT,CF2KP_L),   -1,                     -1,                     MK(CF2K_CAPS,CF2KP_L),  MK(CF2K_KANA,CF2KP_H),  -1,                    -1,                     -1                     },
		{ -1,                     -1,                     MK(CF2K_ESC,CF2KP_L),   MK(CF2K_TAB,CF2KP_L),   MK(CF2K_END,CF2KP_L),   MK(CF2K_BS,CF2KP_H),   -1,                     MK(CF2K_RT,CF2KP_H)    },
		{ MK(CF2K_SP,CF2KP_H),    MK(CF2K_HOME,CF2KP_H),  MK(CF2K_INS,CF2KP_H),   MK(CF2K_DEL,CF2KP_M),   MK(CF2K_LE,CF2KP_H),    MK(CF2K_UP,CF2KP_H),   MK(CF2K_DN,CF2KP_H),    MK(CF2K_RI,CF2KP_H)    },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
	}
};

// ----------------------------------------------------------------
//	begin()
//		Clear the previous key entry information and prepare to 
//		start registering the current key entry information.
//		前回のキー入力情報をクリアして、現在のキー入力情報の登録を
//		開始する準備をする。
//
void CF2KEY::begin( void ) {

	std::memcpy( this->current_keymap, this->init_keymap, sizeof( this->current_keymap ) );
	std::memset( this->priority, 0, sizeof( this->priority ) );

	this->is_kana_key = false;
}

// ----------------------------------------------------------------
//	regist_pressed_key( keycode, priority )
//		The specified key is registered as the currently pressed state.
//		指定のキーが、現在押されている状態として登録する。
//
void CF2KEY::regist_pressed_key( CF2K_KEYCODE keycode, CF2K_PRIORITY priority ) {
	int i;
	int column	= CF2K_GET_COLUMN( keycode );
	int row		= CF2K_GET_ROW( keycode ) + 1;
	int shift	= CF2K_GET_SHIFT( keycode );

	for( i = 0; i < 3; i++ ) {
		if( (shift & (1 << i)) != 0 ) {
			this->priority[ i ] += priority;
			this->current_keymap[ i ][ row ] &= column;
		}
	}
}

// ----------------------------------------------------------------
//	regist_msx_key( keymap[11] )
//		The specified key is registered as the currently pressed state.
//		指定のキーが、現在押されている状態として登録する。
//
void CF2KEY::regist_msx_key( const uint8_t *p_msx_key ) {
	int y, x, key, cf2k, shift_key;
	CF2K_PRIORITY priority;
	CF2K_KEYCODE keycode;

	shift_key = (p_msx_key[6] & 1) ^ 1;
	for( y = 0; y < 11; y++ ) {
		key = p_msx_key[y];
		for( x = 0; x < 8; x++ ) {
			if( (key & 1) == 0 ) {
				//	キーが押されていた場合
				if( x == 4 && y == 6 ) {
					//	かなキーの場合
					if( !this->is_kana_key_pressed ) {
						DEBUG( Serial.printf( "KANA Key pressed.\n" ) );
						this->is_kana_key = true;
						this->is_kana_key_pressed = true;
					}
				}
				else {
					//	その他のキーの場合
					cf2k = convert_table[shift_key][y][x];
					if( cf2k != -1 ) {
						keycode = (CF2K_KEYCODE)(cf2k & 0xFFFFFF);
						priority = (CF2K_PRIORITY)(cf2k >> 24);
						this->regist_pressed_key( keycode, priority );
					}
				}
			}
			else {
				//	キーが解放されている場合
				if( x == 4 && y == 6 ) {
					//	かなキーの場合
					if( this->is_kana_key_pressed ) {
						DEBUG( Serial.printf( "KANA Key released.\n" ) );
						this->is_kana_key = true;
						this->is_kana_key_pressed = false;
					}
				}
			}
			key >>= 1;
		}
	}
}

// ----------------------------------------------------------------
//	copy_send_data()
//		Copy send data
//		送信バイト列をセットする。
//
void CF2KEY::copy_send_data( const uint8_t *p, int shift_key ) {
	int i, sum;
	uint8_t d0, d1;

	sum = 0x0A;
	this->send_bytes.data[0] = 0x0A;
	shift_key |= 0x1D;
	for( i = 1; i < 9; i += 2 ) {
		d0 = p[i + 0] & 0x7F;
		d1 = p[i + 1];
		if( i == 7 && shift_key != -1 ) {
			//	ALTキーの状態は継承する
			d1 &= shift_key;
		}
		if( (this->send_bytes.data[i + 0] & 0x7F) != d0 || this->send_bytes.data[i + 1] != d1 ) {
			d0 |= 0x80;
		}
		this->send_bytes.data[i + 0] = d0;
		this->send_bytes.data[i + 1] = d1;
		sum += d0 + d1;
	}
	this->send_bytes.data[9] = (uint8_t) -sum;
	send_fifo.insert( send_fifo.begin(), this->send_bytes );
}

// ----------------------------------------------------------------
//	end()
//		Generate and return a key matrix to be sent according to 
//		the keys pressed and their priority.
//		押されたキーと、そのプライオリティに従って、
//		送信すべきキーマトリクスを生成して返す。
//
void CF2KEY::end( void ) {
	static const uint8_t shift_keymap[4][10] = {
		{ 0x0A, 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0xB0, 0x1F, 0xF1 },		//	0: CF2K_DECO_NONE
		{ 0x0A, 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0xB0, 0x17, 0xF9 },		//	1: CF2K_DECO_SYM
		{ 0x0A, 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0xB0, 0x0F, 0x01 },		//	2: CF2K_DECO_FN
		{ 0x0A, 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0xB0, 0x0E, 0x02 },		//	3: CF2K_DECO_FN  + aA
	};
	int i;

	//	優先度の高いシフトキー状態を選択する
	int top_priority_shift = 0;
	int top_priority = this->priority[0];
	for( i = 1; i < sizeof(this->priority)/sizeof(this->priority[0]); i++ ) {
		if( top_priority < this->priority[i] ) {
			top_priority_shift = i;
			top_priority = this->priority[i];
		}
	}
	//	かなキーが変化した場合は、かなキー専用のコマンドを送信する
	if( this->is_kana_key ) {
		if( this->is_kana_key_pressed ) {
			this->is_kana_mode = !this->is_kana_mode;
		}
		if( this->is_kana_key_pressed ) {
			DEBUG( Serial.printf( "Begin KANA mode\n" ) );
			this->copy_send_data( &shift_keymap[2][0], current_keymap[ top_priority_shift ][8] );	//	[Fn]
			this->copy_send_data( &shift_keymap[3][0], current_keymap[ top_priority_shift ][8] );	//	[Fn] + [aA]
		}
		else {
			DEBUG( Serial.printf( "End KANA mode\n" ) );
			this->copy_send_data( &shift_keymap[2][0], current_keymap[ top_priority_shift ][8] );	//	[Fn]
			this->copy_send_data( &shift_keymap[0][0], current_keymap[ top_priority_shift ][8] );	//	None
		}
	}
	//	シフトキー状態が変化する場合は、変更マトリクスを単独で送信する
	DEBUG( Serial.printf( "Shift key changed [%d]\n", top_priority_shift ) );
	this->copy_send_data( &shift_keymap[ top_priority_shift ][0] );
	this->send_bytes.data[8] = 0x1F;

	//	更新通知ビットの生成とバッファへコピー
	this->copy_send_data( &(this->current_keymap[ top_priority_shift ][0]) );
	DEBUG( Serial.printf( "Check sum: %02X\n", this->send_bytes.data[9] ) );
}
