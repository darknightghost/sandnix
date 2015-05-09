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
#include "../fs/fs.h"
#include "../io/stdout.h"
#include "../string/string.h"
#include "../exception/exception.h"
#include "../../../../../common/arch/x86/elf_x86.h"
#include "../../../../../common/arch/x86/kernel_image.h"
#include "../memtest/memtest.h"

static	bool		load_segment(Elf32_Phdr* p_pheader, pfile fp);

bool load_os_kernel(char* path, char* parameters)
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

	//Read elf header
	if(read(fp, (u8*)&elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		panic(EXCEPTION_UNKNOW_KERNEL_FORMAT);
	}

	entry_address = (void*)(elf_header.e_entry) - VIRTUAL_ADDR_OFFSET;
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

	//Copy parameters address
	*(char**)KERNEL_PARAMETER_PHYSICAL = parameters;

	//Copy memory info address
	*(void**)KERNEL_MEM_INFO_PHYSICAL = mem_info;
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
	size_in_mem = (p_pheader->p_memsz % p_pheader->p_align ? 1 : 0
	               + p_pheader->p_memsz / p_pheader->p_align)
	              * p_pheader->p_align;
	seek(fp, p_pheader->p_offset, FILE_POS_BEGIN);

	//Read segment
	if(read(fp, (u8*)(p_pheader->p_vaddr - VIRTUAL_ADDR_OFFSET), p_pheader->p_filesz)
	   != p_pheader->p_filesz) {
		return false;
	}

	memset((u8*)(p_pheader->p_vaddr - VIRTUAL_ADDR_OFFSET + p_pheader->p_filesz),
	       0,
	       size_in_mem - p_pheader->p_filesz);
	return true;
}
