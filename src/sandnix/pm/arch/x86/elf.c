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

#include "elf.h"
#include "../../../vfs/vfs.h"
#include "../../../mm/mm.h"
#include "../../../msg/msg.h"

k_status check_elf(char* path)
{
	u32 fd;
	k_status status;
	ppmo_t p_pmo;
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;
	Elf32_Ehdr* p_head;
	
	//Check if the file is executable
	status = vfs_access(path,X_OK | R_OK);
	if(status != ESUCCESS){
		return status;
	}
	
	//Open file
	fd = vfs_open(path,O_RDONLY,0);
	if(!OPERATE_SUCCESS){
		return pm_get_errno();
	}
	
	//Create buffer
	p_pmo = mm_pmo_create(sizeof(Elf32_Ehdr) + sizeof(msg_read_info_t));
	if(p_pmo == NULL){
		vfs_close(fd);
		pm_set_errno(EFAULT);
		return EFAULT;
	}
	
	//Map buffer
	p_info = mm_pmo_map(NULL,p_pmo,false);
	if(p_info == NULL){
		mm_pmo_unmap(p_info,p_pmo);
		vfs_close(fd);
		pm_set_errno(EFAULT);
		return EFAULT;
	}
	
	//Read elf header
	p_info->len = sizeof(Elf32_Ehdr);
	status = vfs_read(fd, p_pmo);
	if(status != ESUCCESS){
		mm_pmo_unmap(p_info,p_pmo);
		vfs_close(fd);
		pm_set_errno(status);
		return status;
	}
	
	p_data = (pmsg_read_data_t)p_info;
	if(p_data->len != sizeof(Elf32_Ehdr)){
		mm_pmo_unmap(p_info,p_pmo);
		vfs_close(fd);
		pm_set_errno(ENOEXEC);
		return ENOEXEC;
	}
	
	//Check elf header
	p_head = (Elf32_Ehdr*)(&(p_data->data));
	if(p_head->e_ident[0] != 0x7F
		||p_head->e_ident[0] != 0x45
		||p_head->e_ident[0] != 0x4C
		||p_head->e_ident[0] != 0x46){
		mm_pmo_unmap(p_info,p_pmo);
		vfs_close(fd);
		pm_set_errno(ENOEXEC);
		return ENOEXEC;
	}
	
	//TODO:
}

void* load_elf(char* path)
{
}