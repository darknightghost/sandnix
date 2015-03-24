/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "keyboard.h"
#include "io.h"

u32 get_keyboard_input()
{
	u32 pressed_key;
	u8 scan_code;
	pressed_key = 0;

	while(pressed_key == 0) {
		scan_code = in_byte(0x60);

		//Analyze scan code
		if(scan_code == 0x1C) {
			pressed_key = KEY_ENTER_PRESSED;
		} else if(scan_code == 0x01) {
			pressed_key = KEY_ESC_PRESSED;
		} else if(scan_code == 0xE0) {
			scan_code = in_byte(0x60);

			if(scan_code == 0x48) {
				pressed_key = KEY_UP_PRESSED;
			} else if(scan_code == 0x50) {
				pressed_key = KEY_DOWN_PRESSED;
			}
		} else {
			pressed_key = 0;
		}
	}

	return pressed_key;
}
