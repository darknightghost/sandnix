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

#include "k_loader.h"
#include "elf.h"
#include "../fs/fs.h"
#include "../io/stdout.h"
#include "../string/string.h"
#include "../exception/exception.h"
#include "../../../../../common/arch/x86/kernel_image.h"
#include "../memtest/memtest.h"

static	bool		load_segment(Elf32_Phdr* p_pheader, pfile fp);
static	bool		load_initrd(char* path);

char num_buf[64];

bool load_os_kernel(char* path, char* initrd, char* parameters)
{
	pfile fp;
	Elf32_Ehdr elf_header;
	void* entry_address;
	Elf32_Phdr* pheader_buf;
	Elf32_Phdr* p_pheader;
	u32 cnt;
	//Open kernel file
	fp = open(path);

	if(fp == NULL) {
		return false;
	}

	print_string(
	    GET_REAL_ADDR("Kernel file	: "),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    path,
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\n\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	//Read elf header
	if(read(fp, (u8*)&elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		panic(EXCEPTION_UNKNOW_KERNEL_FORMAT);
	}

	entry_address = (void*)(elf_header.e_entry) - VIRTUAL_ADDR_OFFSET;
	print_string(
	    GET_REAL_ADDR("entry		: 0x"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    hextostr((u32)entry_address,
	             GET_REAL_ADDR(num_buf)),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\n\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	//Read program header table
	pheader_buf = malloc(elf_header.e_phnum * elf_header.e_phentsize);

	if(pheader_buf == NULL) {
		panic(EXCEPTION_NOT_ENOUGH_MEMORY);
	}

	seek(fp, elf_header.e_phoff, FILE_POS_BEGIN);

	if(read(fp, (u8*)pheader_buf, elf_header.e_phnum * elf_header.e_phentsize)
	   != elf_header.e_phnum * elf_header.e_phentsize) {
		panic(EXCEPTION_UNKNOW_KERNEL_FORMAT);
	}

	//Load segments
	for(p_pheader = pheader_buf, cnt = 0;
	    cnt < elf_header.e_phnum;
	    cnt++, p_pheader = (Elf32_Phdr*)((char*)p_pheader + elf_header.e_phentsize)) {
		if(!load_segment(p_pheader, fp)) {
			panic(EXCEPTION_UNKNOW_KERNEL_FORMAT);
		}
	}

	print_string(
	    GET_REAL_ADDR("Copying parameters...\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	//Copy parameters address
	*(char**)KERNEL_PARAMETER_PHYSICAL = parameters;

	//Copy memory info address
	*(void**)KERNEL_MEM_INFO_PHYSICAL = GET_REAL_ADDR(mem_info);

	print_string(
	    GET_REAL_ADDR("Loading ramdisk...\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	if(!load_initrd(initrd)) {
		return false;
	}

	print_string(
	    GET_REAL_ADDR("Starting kernel...\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	__asm__ __volatile__(
	    "cli\n\t"
	    "movl		%0,%%eax\n\t"
	    "jmpl		*%%eax\n\t"
	    ::"m"(entry_address));

	//Err...In fact,this function will never return true
	return true;
}

bool load_segment(Elf32_Phdr* p_pheader, pfile fp)
{
	u32 size_in_mem;

	seek(fp, p_pheader->p_offset, FILE_POS_BEGIN);

	print_string(
	    GET_REAL_ADDR("Segment:\naddr		: 0x"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    hextostr((p_pheader->p_vaddr - VIRTUAL_ADDR_OFFSET),
	             GET_REAL_ADDR(num_buf)),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\nalign		: 0x"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    hextostr(p_pheader->p_align,
	             GET_REAL_ADDR(num_buf)),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\nsize		: 0x"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    hextostr(p_pheader->p_filesz,
	             GET_REAL_ADDR(num_buf)),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\nfile offset	: 0x"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    hextostr(p_pheader->p_offset,
	             GET_REAL_ADDR(num_buf)),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\n\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	//Read segment
	if(read(fp, (u8*)(p_pheader->p_vaddr - VIRTUAL_ADDR_OFFSET), p_pheader->p_filesz)
	   != p_pheader->p_filesz) {
		return false;
	}

	return true;
}

bool load_initrd(char* path)
{
	pfile fp;

	print_string(
	    GET_REAL_ADDR("initrd = "),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    path,
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	//Open file
	fp = open(path);

	if(fp == NULL) {
		return false;
	}

	*(u32*)(RAMDISK_PHYSICAL) = fp->size;

	if(fp->size > RAMDISK_SIZE - 4) {
		panic(EXCEPTION_RAMDISK_TOO_LARGE);
	}

	if(read(fp, (u8*)(RAMDISK_PHYSICAL + 4), fp->size) != fp->size) {
		return false;
	}

	print_string(
	    GET_REAL_ADDR("initrd size = "),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    dectostr(fp->size,
	             GET_REAL_ADDR(num_buf)),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	print_string(
	    GET_REAL_ADDR("\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);

	return true;
}
