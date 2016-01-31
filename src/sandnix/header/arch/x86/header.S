/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

/*
     Multiboot header of kernel image,
*/

#include "../../../boot/multiboot2.h"

#define	HEADER_LEN	(_end - _start)
#define	ADDR_MAGIC	0xFFFFFFFF

.section	.text 
.global		_start

_start:
magic:
	.long	MULTIBOOT2_HEADER_MAGIC
architecture:
	.long	MULTIBOOT_ARCHITECTURE_I386
header_length:
	.long	HEADER_LEN
checksum:
	.long	0 - HEADER_LEN - MULTIBOOT_ARCHITECTURE_I386 - MULTIBOOT2_HEADER_MAGIC
//Multiboot information request
_ADDR_1:
	//type
	.word	MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
	//flags
	.word	0
	//size
	.long	_ADDR_2 - _ADDR_1
	//mbi_tag_types
	.long	MULTIBOOT_TAG_TYPE_MMAP
	.long	MULTIBOOT_TAG_TYPE_MODULE
	.long	MULTIBOOT_TAG_TYPE_CMDLINE
//The address tag of Multiboot header
_ADDR_2:
	//type
	.word	MULTIBOOT_HEADER_TAG_ADDRESS
	//flags
	.word	0
	//size
	.long	_ADDR_3 - _ADDR_2
	//header_addr
	.long	ADDR_MAGIC
	//load_addr
	.long	ADDR_MAGIC
	//load_end_addr
	.long	ADDR_MAGIC
	//bss_end_addr
	.long	ADDR_MAGIC
//The entry address tag of Multiboot header
_ADDR_3:
	//type
	.word	MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
	//flags
	.word	0
	//size
	.long	_ADDR_4 - _ADDR_3
	//entry_addr
	.long	ADDR_MAGIC
//End tag
_ADDR_4:
	//type
	.word	MULTIBOOT_HEADER_TAG_END
	//flags
	.word	0
	//size
	.long	_end - _ADDR_4
_end:
