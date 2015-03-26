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

#include "segment.h"
#include "io/io.h"
#include "io/stdout.h"
#include "interrupt/interrupt.h"
#include "exception/exception.h"
#include "memory/memory.h"
#include "io/keyboard.h"
#include "hdd/hdd.h"
#include "hdd/partition.h"


void loader_main()
{
	u8* p;
	u32 partition;
	u32 offset;
	u32 size;
	u32 disk_info;
	setup_interrupt();
	cls(BG_BLACK | FG_BRIGHT_WHITE);
	print_string(
		GET_REAL_ADDR("Protect mode entered.\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);
	init_heap();
	print_string(
		GET_REAL_ADDR("Searching for sanlo.cfg...\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);
	p = malloc(512);
	
	partition = get_partition_info(0, 0, &offset, &size);

	if(partition == PARTITION_NOT_FOUND) {
		print_string(
			GET_REAL_ADDR("PARTITION_NOT_FOUND\n"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
	} else if(partition == PARTITION_PRIMARY) {
		print_string(
			GET_REAL_ADDR("PARTITION_PRIMARY\n"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
	} else if(partition == PARTITION_EXTENDED) {
		print_string(
			GET_REAL_ADDR("PARTITION_EXTENDED\n"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
	} else if(partition == PARTITION_LOGIC) {
		print_string(
			GET_REAL_ADDR("PARTITION_LOGIC\n"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
	}

	disk_info = get_hdd_info(0);

	if(!hdd_read(disk_info, offset+2, 1, p)) {
		print_string(
			GET_REAL_ADDR("Can\'t read\n"),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
	}
	__asm__ __volatile__(
		".global bp\n\t"
		"bp:\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
	);

	__asm__ __volatile__(
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"movl		%0,%%eax\n\t"
		"movl		%1,%%ebx\n\t"
		::"m"(p),"m"(offset));
	print_string(
		GET_REAL_ADDR("\nEnd\n"),
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);

	while(1);

	return;
}
