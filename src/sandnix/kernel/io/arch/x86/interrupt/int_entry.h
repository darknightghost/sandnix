/*
	Copyright 2015,∞µ“π”ƒ¡È <darknightghost.cn@gmail.com>

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

#pragma once

#define	INT_ENTRY(num)		void	int_##num()

#define	de_int_entry		int_0x00
#define	db_int_entry		int_0x01
#define	nmi_int_entry		int_0x02
#define	bp_int_entry		int_0x03
#define	of_int_entry		int_0x04
#define	br_int_entry		int_0x05
#define	ud_int_entry		int_0x06
#define	nm_int_entry		int_0x07
#define	df_int_entry		int_0x08
#define	fpu_int_entry		int_0x09
#define	ts_int_entry		int_0x0A
#define	np_int_entry		int_0x0B
#define	ss_int_entry		int_0x0C
#define	gp_int_entry		int_0x0D
#define	pf_int_entry		int_0x0E
#define	reserved_int_entry	int_0x0F
#define	mf_int_entry		int_0x10
#define	ac_int_entry		int_0x11
#define	mc_int_entry		int_0x12
#define	xf_int_entry		int_0x13

INT_ENTRY(0x00);
INT_ENTRY(0x01);
INT_ENTRY(0x02);
INT_ENTRY(0x03);
INT_ENTRY(0x04);
INT_ENTRY(0x05);
INT_ENTRY(0x06);
INT_ENTRY(0x07);
INT_ENTRY(0x08);
INT_ENTRY(0x09);
INT_ENTRY(0x0A);
INT_ENTRY(0x0B);
INT_ENTRY(0x0C);
INT_ENTRY(0x0D);
INT_ENTRY(0x0E);
INT_ENTRY(0x0F);

INT_ENTRY(0x10);
INT_ENTRY(0x11);
INT_ENTRY(0x12);
INT_ENTRY(0x13);
INT_ENTRY(0x14);
INT_ENTRY(0x15);
INT_ENTRY(0x16);
INT_ENTRY(0x17);
INT_ENTRY(0x18);
INT_ENTRY(0x19);
INT_ENTRY(0x1A);
INT_ENTRY(0x1B);
INT_ENTRY(0x1C);
INT_ENTRY(0x1D);
INT_ENTRY(0x1E);
INT_ENTRY(0x1F);

INT_ENTRY(0x20);
INT_ENTRY(0x21);
INT_ENTRY(0x22);
INT_ENTRY(0x23);
INT_ENTRY(0x24);
INT_ENTRY(0x25);
INT_ENTRY(0x26);
INT_ENTRY(0x27);
INT_ENTRY(0x28);
INT_ENTRY(0x29);
INT_ENTRY(0x2A);
INT_ENTRY(0x2B);
INT_ENTRY(0x2C);
INT_ENTRY(0x2D);
INT_ENTRY(0x2E);
INT_ENTRY(0x2F);

INT_ENTRY(0x30);
INT_ENTRY(0x31);
INT_ENTRY(0x32);
INT_ENTRY(0x33);
INT_ENTRY(0x34);
INT_ENTRY(0x35);
INT_ENTRY(0x36);
INT_ENTRY(0x37);
INT_ENTRY(0x38);
INT_ENTRY(0x39);
INT_ENTRY(0x3A);
INT_ENTRY(0x3B);
INT_ENTRY(0x3C);
INT_ENTRY(0x3D);
INT_ENTRY(0x3E);
INT_ENTRY(0x3F);

INT_ENTRY(0x40);
INT_ENTRY(0x41);
INT_ENTRY(0x42);
INT_ENTRY(0x43);
INT_ENTRY(0x44);
INT_ENTRY(0x45);
INT_ENTRY(0x46);
INT_ENTRY(0x47);
INT_ENTRY(0x48);
INT_ENTRY(0x49);
INT_ENTRY(0x4A);
INT_ENTRY(0x4B);
INT_ENTRY(0x4C);
INT_ENTRY(0x4D);
INT_ENTRY(0x4E);
INT_ENTRY(0x4F);

INT_ENTRY(0x50);
INT_ENTRY(0x51);
INT_ENTRY(0x52);
INT_ENTRY(0x53);
INT_ENTRY(0x54);
INT_ENTRY(0x55);
INT_ENTRY(0x56);
INT_ENTRY(0x57);
INT_ENTRY(0x58);
INT_ENTRY(0x59);
INT_ENTRY(0x5A);
INT_ENTRY(0x5B);
INT_ENTRY(0x5C);
INT_ENTRY(0x5D);
INT_ENTRY(0x5E);
INT_ENTRY(0x5F);

INT_ENTRY(0x60);
INT_ENTRY(0x61);
INT_ENTRY(0x62);
INT_ENTRY(0x63);
INT_ENTRY(0x64);
INT_ENTRY(0x65);
INT_ENTRY(0x66);
INT_ENTRY(0x67);
INT_ENTRY(0x68);
INT_ENTRY(0x69);
INT_ENTRY(0x6A);
INT_ENTRY(0x6B);
INT_ENTRY(0x6C);
INT_ENTRY(0x6D);
INT_ENTRY(0x6E);
INT_ENTRY(0x6F);

INT_ENTRY(0x70);
INT_ENTRY(0x71);
INT_ENTRY(0x72);
INT_ENTRY(0x73);
INT_ENTRY(0x74);
INT_ENTRY(0x75);
INT_ENTRY(0x76);
INT_ENTRY(0x77);
INT_ENTRY(0x78);
INT_ENTRY(0x79);
INT_ENTRY(0x7A);
INT_ENTRY(0x7B);
INT_ENTRY(0x7C);
INT_ENTRY(0x7D);
INT_ENTRY(0x7E);
INT_ENTRY(0x7F);

INT_ENTRY(0x80);
INT_ENTRY(0x81);
INT_ENTRY(0x82);
INT_ENTRY(0x83);
INT_ENTRY(0x84);
INT_ENTRY(0x85);
INT_ENTRY(0x86);
INT_ENTRY(0x87);
INT_ENTRY(0x88);
INT_ENTRY(0x89);
INT_ENTRY(0x8A);
INT_ENTRY(0x8B);
INT_ENTRY(0x8C);
INT_ENTRY(0x8D);
INT_ENTRY(0x8E);
INT_ENTRY(0x8F);

INT_ENTRY(0x90);
INT_ENTRY(0x91);
INT_ENTRY(0x92);
INT_ENTRY(0x93);
INT_ENTRY(0x94);
INT_ENTRY(0x95);
INT_ENTRY(0x96);
INT_ENTRY(0x97);
INT_ENTRY(0x98);
INT_ENTRY(0x99);
INT_ENTRY(0x9A);
INT_ENTRY(0x9B);
INT_ENTRY(0x9C);
INT_ENTRY(0x9D);
INT_ENTRY(0x9E);
INT_ENTRY(0x9F);

INT_ENTRY(0xA0);
INT_ENTRY(0xA1);
INT_ENTRY(0xA2);
INT_ENTRY(0xA3);
INT_ENTRY(0xA4);
INT_ENTRY(0xA5);
INT_ENTRY(0xA6);
INT_ENTRY(0xA7);
INT_ENTRY(0xA8);
INT_ENTRY(0xA9);
INT_ENTRY(0xAA);
INT_ENTRY(0xAB);
INT_ENTRY(0xAC);
INT_ENTRY(0xAD);
INT_ENTRY(0xAE);
INT_ENTRY(0xAF);

INT_ENTRY(0xB0);
INT_ENTRY(0xB1);
INT_ENTRY(0xB2);
INT_ENTRY(0xB3);
INT_ENTRY(0xB4);
INT_ENTRY(0xB5);
INT_ENTRY(0xB6);
INT_ENTRY(0xB7);
INT_ENTRY(0xB8);
INT_ENTRY(0xB9);
INT_ENTRY(0xBA);
INT_ENTRY(0xBB);
INT_ENTRY(0xBC);
INT_ENTRY(0xBD);
INT_ENTRY(0xBE);
INT_ENTRY(0xBF);

INT_ENTRY(0xC0);
INT_ENTRY(0xC1);
INT_ENTRY(0xC2);
INT_ENTRY(0xC3);
INT_ENTRY(0xC4);
INT_ENTRY(0xC5);
INT_ENTRY(0xC6);
INT_ENTRY(0xC7);
INT_ENTRY(0xC8);
INT_ENTRY(0xC9);
INT_ENTRY(0xCA);
INT_ENTRY(0xCB);
INT_ENTRY(0xCC);
INT_ENTRY(0xCD);
INT_ENTRY(0xCE);
INT_ENTRY(0xCF);

INT_ENTRY(0xD0);
INT_ENTRY(0xD1);
INT_ENTRY(0xD2);
INT_ENTRY(0xD3);
INT_ENTRY(0xD4);
INT_ENTRY(0xD5);
INT_ENTRY(0xD6);
INT_ENTRY(0xD7);
INT_ENTRY(0xD8);
INT_ENTRY(0xD9);
INT_ENTRY(0xDA);
INT_ENTRY(0xDB);
INT_ENTRY(0xDC);
INT_ENTRY(0xDD);
INT_ENTRY(0xDE);
INT_ENTRY(0xDF);

INT_ENTRY(0xE0);
INT_ENTRY(0xE1);
INT_ENTRY(0xE2);
INT_ENTRY(0xE3);
INT_ENTRY(0xE4);
INT_ENTRY(0xE5);
INT_ENTRY(0xE6);
INT_ENTRY(0xE7);
INT_ENTRY(0xE8);
INT_ENTRY(0xE9);
INT_ENTRY(0xEA);
INT_ENTRY(0xEB);
INT_ENTRY(0xEC);
INT_ENTRY(0xED);
INT_ENTRY(0xEE);
INT_ENTRY(0xEF);

INT_ENTRY(0xF0);
INT_ENTRY(0xF1);
INT_ENTRY(0xF2);
INT_ENTRY(0xF3);
INT_ENTRY(0xF4);
INT_ENTRY(0xF5);
INT_ENTRY(0xF6);
INT_ENTRY(0xF7);
INT_ENTRY(0xF8);
INT_ENTRY(0xF9);
INT_ENTRY(0xFA);
INT_ENTRY(0xFB);
INT_ENTRY(0xFC);
INT_ENTRY(0xFD);
INT_ENTRY(0xFE);
INT_ENTRY(0xFF);

