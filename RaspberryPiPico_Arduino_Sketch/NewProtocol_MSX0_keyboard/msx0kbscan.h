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

#ifndef __MSX0KBSCAN_H__
#define __MSX0KBSCAN_H__

class CMSX0KBSCAN {
private:
	uint8_t keymatrix[13];
public:
	// ----------------------------------------------------------------
	//	begin()
	//		Initialization process to use the keyboard.
	//		キーボードを利用するための初期化処理。
	//
	void begin( void );

	// ----------------------------------------------------------------
	//	update()
	//		Update key matrix information.
	//		Returns true if there is a change in input status.
	//		キーマトリクス情報を更新する。
	//		入力状態に変化があれば true を返す。
	//
	bool update( void );

	// ----------------------------------------------------------------
	//	get()
	//		Obtain key matrix information.
	//		キーマトリクス情報を取得する。
	//
	const uint8_t *get( void ) const {
		return this->keymatrix;
	}
};

#endif
