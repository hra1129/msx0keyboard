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

#ifndef __FACE2KEYMAP_H__
#define __FACE2KEYMAP_H__

#include <Arduino.h>
#include <cstring>

#define CF2K_KEYMACRO( row, column, shift_keycode )	( ((shift_keycode) << 16) | (((row) - 1) << 8) | ((1 << (column)) ^ 0xFF) )
#define CF2K_GET_COLUMN( keycode )	( (keycode) & 0xFF )
#define CF2K_GET_ROW( keycode )		( ((keycode) >> 8) & 0xFF )
#define CF2K_GET_SHIFT( keycode )	(  (keycode) >> 16 )

typedef enum {
	CF2K_UPD_NORMAL	= 1,
	CF2K_UPD_SYM	= 2,
	CF2K_UPD_FN		= 4,
	CF2K_UPD_ALL	= CF2K_UPD_NORMAL | CF2K_UPD_SYM | CF2K_UPD_FN,
	//	1st row
	CF2K_Q		= CF2K_KEYMACRO( 1, 1, CF2K_UPD_NORMAL ),	//	Q
	CF2K_W		= CF2K_KEYMACRO( 1, 0, CF2K_UPD_NORMAL ),	//	W
	CF2K_E		= CF2K_KEYMACRO( 2, 7, CF2K_UPD_NORMAL ),	//	E
	CF2K_R		= CF2K_KEYMACRO( 2, 6, CF2K_UPD_NORMAL ),	//	R
	CF2K_T		= CF2K_KEYMACRO( 2, 5, CF2K_UPD_NORMAL ),	//	T
	CF2K_Y		= CF2K_KEYMACRO( 2, 4, CF2K_UPD_NORMAL ),	//	Y
	CF2K_U		= CF2K_KEYMACRO( 2, 3, CF2K_UPD_NORMAL ),	//	U
	CF2K_I		= CF2K_KEYMACRO( 2, 2, CF2K_UPD_NORMAL ),	//	I
	CF2K_O		= CF2K_KEYMACRO( 2, 1, CF2K_UPD_NORMAL ),	//	O
	CF2K_P		= CF2K_KEYMACRO( 2, 0, CF2K_UPD_NORMAL ),	//	P

	CF2K_SHARP	= CF2K_KEYMACRO( 1, 1, CF2K_UPD_SYM ),	//	#
	CF2K_1		= CF2K_KEYMACRO( 1, 0, CF2K_UPD_SYM ),	//	1
	CF2K_2		= CF2K_KEYMACRO( 2, 7, CF2K_UPD_SYM ),	//	2
	CF2K_3		= CF2K_KEYMACRO( 2, 6, CF2K_UPD_SYM ),	//	3
	CF2K_MLBRA	= CF2K_KEYMACRO( 2, 5, CF2K_UPD_SYM ),	//	(
	CF2K_MRBRA	= CF2K_KEYMACRO( 2, 4, CF2K_UPD_SYM ),	//	)
	CF2K__		= CF2K_KEYMACRO( 2, 3, CF2K_UPD_SYM ),	//	_
	CF2K_MINUS	= CF2K_KEYMACRO( 2, 2, CF2K_UPD_SYM ),	//	-
	CF2K_PLUS	= CF2K_KEYMACRO( 2, 1, CF2K_UPD_SYM ),	//	+
	CF2K_AT		= CF2K_KEYMACRO( 2, 0, CF2K_UPD_SYM ),	//	@

	CF2K_CHILD	= CF2K_KEYMACRO( 1, 1, CF2K_UPD_FN ),	//	~
	CF2K_HAT	= CF2K_KEYMACRO( 1, 0, CF2K_UPD_FN ),	//	^
	CF2K_AMP	= CF2K_KEYMACRO( 2, 7, CF2K_UPD_FN ),	//	&
	CF2K_APOS	= CF2K_KEYMACRO( 2, 6, CF2K_UPD_FN ),	//	`
	CF2K_LT		= CF2K_KEYMACRO( 2, 5, CF2K_UPD_FN ),	//	<
	CF2K_GT		= CF2K_KEYMACRO( 2, 4, CF2K_UPD_FN ),	//	>
	CF2K_LLBRA	= CF2K_KEYMACRO( 2, 3, CF2K_UPD_FN ),	//	{
	CF2K_LRBRA	= CF2K_KEYMACRO( 2, 2, CF2K_UPD_FN ),	//	}
	CF2K_BLBRA	= CF2K_KEYMACRO( 2, 1, CF2K_UPD_FN ),	//	[
	CF2K_BRBRA	= CF2K_KEYMACRO( 2, 0, CF2K_UPD_FN ),	//	]
	//	2nd row
	CF2K_A		= CF2K_KEYMACRO( 3, 1, CF2K_UPD_NORMAL ),	//	A
	CF2K_S		= CF2K_KEYMACRO( 3, 0, CF2K_UPD_NORMAL ),	//	S
	CF2K_D		= CF2K_KEYMACRO( 4, 7, CF2K_UPD_NORMAL ),	//	D
	CF2K_F		= CF2K_KEYMACRO( 4, 6, CF2K_UPD_NORMAL ),	//	F
	CF2K_G		= CF2K_KEYMACRO( 4, 5, CF2K_UPD_NORMAL ),	//	G
	CF2K_H		= CF2K_KEYMACRO( 4, 4, CF2K_UPD_NORMAL ),	//	H
	CF2K_J		= CF2K_KEYMACRO( 4, 3, CF2K_UPD_NORMAL ),	//	J
	CF2K_K		= CF2K_KEYMACRO( 4, 2, CF2K_UPD_NORMAL ),	//	K
	CF2K_L		= CF2K_KEYMACRO( 4, 1, CF2K_UPD_NORMAL ),	//	L
	CF2K_BS		= CF2K_KEYMACRO( 4, 0, CF2K_UPD_NORMAL ),	//	BS

	CF2K_ASTA	= CF2K_KEYMACRO( 3, 1, CF2K_UPD_SYM ),	//	* 
	CF2K_4		= CF2K_KEYMACRO( 3, 0, CF2K_UPD_SYM ),	//	4
	CF2K_5		= CF2K_KEYMACRO( 4, 7, CF2K_UPD_SYM ),	//	5
	CF2K_6		= CF2K_KEYMACRO( 4, 6, CF2K_UPD_SYM ),	//	6
	CF2K_SLASH	= CF2K_KEYMACRO( 4, 5, CF2K_UPD_SYM ),	//	/
	CF2K_COLON	= CF2K_KEYMACRO( 4, 4, CF2K_UPD_SYM ),	//	:
	CF2K_SEMI	= CF2K_KEYMACRO( 4, 3, CF2K_UPD_SYM ),	//	;
	CF2K_SQUAT	= CF2K_KEYMACRO( 4, 2, CF2K_UPD_SYM ),	//	' 
	CF2K_DQUAT	= CF2K_KEYMACRO( 4, 1, CF2K_UPD_SYM ),	//	" 
	CF2K_DEL	= CF2K_KEYMACRO( 4, 0, CF2K_UPD_SYM ),	//	DEL 

	CF2K_BAR	= CF2K_KEYMACRO( 3, 1, CF2K_UPD_FN ),	//	_
	CF2K_EQU	= CF2K_KEYMACRO( 3, 0, CF2K_UPD_FN ),	//	=
	CF2K_BSLA	= CF2K_KEYMACRO( 4, 7, CF2K_UPD_FN ),	//	￥ 
	CF2K_PARC	= CF2K_KEYMACRO( 4, 6, CF2K_UPD_FN ),	//	% 
	CF2K_UP		= CF2K_KEYMACRO( 4, 2, CF2K_UPD_FN ),	//	↑
	CF2K_INS	= CF2K_KEYMACRO( 4, 1, CF2K_UPD_FN ),	//	INS
	//	3rd row
	CF2K_0		= CF2K_KEYMACRO( 5, 1, CF2K_UPD_NORMAL ),	//	0
	CF2K_Z		= CF2K_KEYMACRO( 5, 0, CF2K_UPD_NORMAL ),	//	Z
	CF2K_X		= CF2K_KEYMACRO( 6, 7, CF2K_UPD_NORMAL ),	//	X
	CF2K_C		= CF2K_KEYMACRO( 6, 6, CF2K_UPD_NORMAL ),	//	C
	CF2K_V		= CF2K_KEYMACRO( 6, 5, CF2K_UPD_NORMAL ),	//	V
	CF2K_B		= CF2K_KEYMACRO( 6, 4, CF2K_UPD_NORMAL ),	//	B
	CF2K_N		= CF2K_KEYMACRO( 6, 3, CF2K_UPD_NORMAL ),	//	N
	CF2K_M		= CF2K_KEYMACRO( 6, 2, CF2K_UPD_NORMAL ),	//	M
	CF2K_DL		= CF2K_KEYMACRO( 6, 1, CF2K_UPD_NORMAL ),	//	$
	CF2K_SP		= CF2K_KEYMACRO( 6, 0, CF2K_UPD_ALL ),	//	SPACE

	CF2K_ESC	= CF2K_KEYMACRO( 5, 1, CF2K_UPD_SYM ),	//	ESC
	CF2K_7		= CF2K_KEYMACRO( 5, 0, CF2K_UPD_SYM ),	//	7
	CF2K_8		= CF2K_KEYMACRO( 6, 7, CF2K_UPD_SYM ),	//	8
	CF2K_9		= CF2K_KEYMACRO( 6, 6, CF2K_UPD_SYM ),	//	9
	CF2K_QUEST	= CF2K_KEYMACRO( 6, 5, CF2K_UPD_SYM ),	//	?
	CF2K_EXC	= CF2K_KEYMACRO( 6, 4, CF2K_UPD_SYM ),	//	!
	CF2K_COMMA	= CF2K_KEYMACRO( 6, 3, CF2K_UPD_SYM ),	//	,
	CF2K_DOT	= CF2K_KEYMACRO( 6, 2, CF2K_UPD_SYM ),	//	.

	CF2K_NA5	= CF2K_KEYMACRO( 5, 1, CF2K_UPD_FN ),	//	
	CF2K_TAB	= CF2K_KEYMACRO( 5, 0, CF2K_UPD_FN ),	//	TAB
	CF2K_HOME	= CF2K_KEYMACRO( 6, 7, CF2K_UPD_FN ),	//	HOME
	CF2K_END	= CF2K_KEYMACRO( 6, 6, CF2K_UPD_FN ),	//	END
	CF2K_PU		= CF2K_KEYMACRO( 6, 5, CF2K_UPD_FN ),	//	PU
	CF2K_PD		= CF2K_KEYMACRO( 6, 4, CF2K_UPD_FN ),	//	PD
	CF2K_LE		= CF2K_KEYMACRO( 6, 3, CF2K_UPD_FN ),	//	←
	CF2K_DN		= CF2K_KEYMACRO( 6, 2, CF2K_UPD_FN ),	//	↓
	CF2K_RI		= CF2K_KEYMACRO( 6, 1, CF2K_UPD_FN ),	//	→
	CF2K_NA6	= CF2K_KEYMACRO( 6, 0, CF2K_UPD_FN ),	//	

	CF2K_RT		= CF2K_KEYMACRO( 8, 2, CF2K_UPD_ALL ),	//	RET
	CF2K_ALT	= CF2K_KEYMACRO( 8, 1, CF2K_UPD_ALL ),	//	ALT
	CF2K_CAPS	= CF2K_KEYMACRO( 8, 0, CF2K_UPD_ALL ),	//	CAPS
} CF2K_KEYCODE;

typedef enum {
	CF2KP_L = 3,
	CF2KP_M = 9,
	CF2KP_H = 27,
} CF2K_PRIORITY;

class CF2KEY {
private:
	const uint8_t init_keymap[3][8] = {
		{ 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0x30, 0x1F },		//	CF2K_DECO_NONE
		{ 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0xB0, 0x1F },		//	CF2K_DECO_SYM
		{ 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0xB0, 0x1F },		//	CF2K_DECO_FN
	};
	uint8_t last_keymap[8] = { 0x03, 0xFF, 0x13, 0xFF, 0x23, 0xFF, 0x30, 0x1F };
	uint8_t send_bytes[10] = { 0x0A, 0x83, 0xFF, 0x93, 0xFF, 0xA3, 0xFF, 0xB0, 0x1F, 0x71 };
	uint8_t current_keymap[3][8];
	int priority[3];
	int last_priority = -1;
	bool has_shift_changed = false;
public:
	// ----------------------------------------------------------------
	//	begin()
	//		Clear the previous key entry information and prepare to 
	//		start registering the current key entry information.
	//		前回のキー入力情報をクリアして、現在のキー入力情報の登録を
	//		開始する準備をする。
	//
	void begin( void );

	// ----------------------------------------------------------------
	//	regist_pressed_key( keycode, priority )
	//		The specified key is registered as the currently pressed state.
	//		指定のキーが、現在押されている状態として登録する。
	//
	void regist_pressed_key( CF2K_KEYCODE keycode, CF2K_PRIORITY priority = CF2KP_L );

	// ----------------------------------------------------------------
	//	regist_msx_key( keymap[11] )
	//		The specified key is registered as the currently pressed state.
	//		指定のキーが、現在押されている状態として登録する。
	//
	void regist_msx_key( const uint8_t *p_msx_key );

	// ----------------------------------------------------------------
	//	end()
	//		Generate and return a key matrix to be sent according to 
	//		the keys pressed and their priority.
	//		押されたキーと、そのプライオリティに従って、
	//		送信すべきキーマトリクスを生成して返す。
	//
	const uint8_t *end( void );

	// ----------------------------------------------------------------
	//	get_shift_key()
	//		Generate and return a key matrix to be sent according to 
	//		the keys pressed and their priority.
	//		最後に end() した時に、装飾キーの状態が変化していた場合、
	//		装飾キー変更のキーマトリクスを返す。
	//		変更が無い場合は NULL を返す。
	//
	const uint8_t *get_shift_key( void );
};

#endif
