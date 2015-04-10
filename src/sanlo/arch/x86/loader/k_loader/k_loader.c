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
#include "../../../../../common/arch/x86/paramter.h"

#define	VIRTUAL_ADDR_OFFSET			0xC0100000

static	bool		load_section(u32 base_addr, pfile fp);

bool load_os_kernel(char* path, char* paramter)
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
	if(read(fp, &elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		panic(EXCEPTION_UNKNOW_KERNEL_FORMAT);
	}

	entry_address = (void*)(elf_header.e_entry) - VIRTUAL_ADDR_OFFSET;
	//Read program headers
	pheader_buf = malloc(elf_header.phnum * elf_header.e_phentsize);

	if(pheader_buf == NULL) {
		panic(EXCEPTION_NOT_ENOUGH_MEMORY);
	}

	seek(fp, elf_header.e_phoff, FILE_POS_BEGIN);

	if(read(fp, pheader_buf, elf_header.phnum * elf_header.e_phentsize)
	   != elf_header.phnum * elf_header.e_phentsize) {
		panic(EXCEPTION_UNKNOW_KERNEL_FORMAT);
	}
	
	p_pheader=pheader_buf;
	
	for(cnt=0;cnt<elf_header.phnum;cnt++){
	
	}
	return false;
}

bool load_section(void* base_addr, u32 size, u32 offset, pfile fp)
{
	return true;
}
