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

#include "stdout.h"
#include "io.h"
#include "../segment.h"
#include "../string/string.h"

#define	CRTC_ADDR_REG		0x03D4
#define	CRTC_DATA_REG		0x03D5
#define	START_ADDR_H_REG	0x0C
#define	START_ADDR_L_REG	0x0D

void init_stdout()
{
	memset(BASIC_VIDEO_BASE_ADDR, 0x4000, 0);
	out_byte(
		START_ADDR_H_REG,
		CRTC_ADDR_REG);
	out_byte(
		(BASIC_VIDEO_BASE_ADDR >> 8) & 0xFF
		, CRTC_DATA_REG);
	out_byte(
		START_ADDR_L_REG,
		CRTC_ADDR_REG);
	out_byte(
		BASIC_VIDEO_BASE_ADDR & 0xFF
		, CRTC_DATA_REG);
}
