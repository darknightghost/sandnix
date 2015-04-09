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

static	bool		load_sector(u32 base_addr,pfile fp);

bool load_os_kernel(char* path, char* paramter)
{
	pfile fp;
	Elf32_Shdr elf_header;

	//Open kernel file
	fp=open(path);

	if(fp==NULL){
		panic(EXCEPTION_NO_KERNEL);
	}
	return false;
}

bool load_sector(u32 base_addr,pfile fp)
{
	return true;
}
