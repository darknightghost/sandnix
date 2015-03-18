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

#ifndef	SEGMENT_H_INCLUDE
#define	SEGMENT_H_INCLUDE

#define	DESCRIPTOR_SIZE			8
#define	SELECTOR_K_DATA			(1 * DESCRIPTOR_SIZE)
#define	SELECTOR_K_CODE			(2 * DESCRIPTOR_SIZE)
#define	SELECTOR_U_DATA			(3 * DESCRIPTOR_SIZE | 3)
#define	SELECTOR_U_CODE			(4 * DESCRIPTOR_SIZE | 3)
#define	BASIC_VIDEO_BASE_ADDR	0x00005000

#endif	//! SEGMENT_H_INCLUDE
