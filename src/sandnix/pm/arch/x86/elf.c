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
	u32 prog_hdr_num;
	Elf32_Phdr* p_prog_header;
	u32 i;
	u64 file_size;

	//Check if the file is executable
	status = vfs_access(path, X_OK | R_OK);

	if(status != ESUCCESS) {
		return status;
	}

	//Open file
	fd = vfs_open(path, O_RDONLY, 0);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	//Create buffer
	p_pmo = mm_pmo_create(sizeof(Elf32_Ehdr) + sizeof(msg_read_info_t));

	if(p_pmo == NULL) {
		vfs_close(fd);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Read elf header
	p_info->len = sizeof(Elf32_Ehdr);
	status = vfs_read(fd, p_pmo);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(status);
		return status;
	}

	p_data = (pmsg_read_data_t)p_info;

	if(p_data->len != sizeof(Elf32_Ehdr)) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(ENOEXEC);
		return ENOEXEC;
	}

	//Check elf header
	p_head = (Elf32_Ehdr*)(&(p_data->data));

	if(p_head->e_ident[0] != 0x7F
	   || p_head->e_ident[0] != 0x45
	   || p_head->e_ident[0] != 0x4C
	   || p_head->e_ident[0] != 0x46) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(ENOEXEC);
		return ENOEXEC;
	}

	if(p_head->e_type != ET_EXEC
	   || p_head->e_machine != EM_386) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(ENOEXEC);
		return ENOEXEC;
	}

	//Read program headers
	prog_hdr_num = p_head->e_phnum;
	fs_seek(fd, SEEK_SET, p_head->e_phoff);
	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);

	//Create buffer
	p_pmo = mm_pmo_create(sizeof(Elf32_Phdr) * prog_hdr_num
	                      + sizeof(msg_read_info_t));

	if(p_pmo == NULL) {
		vfs_close(fd);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_info->len = sizeof(Elf32_Phdr) * prog_hdr_num;
	status = vfs_read(fd, p_pmo);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(status);
		return status;
	}

	p_data = (pmsg_read_data_t)p_info;

	//Check program header
	if(p_data->len != sizeof(Elf32_Phdr) * prog_hdr_num) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		vfs_close(fd);
		pm_set_errno(ENOEXEC);
		return ENOEXEC;
	}

	p_prog_header = &(p_data->data);

	//Check segments
	file_size = vfs_seek(fd, SEEK_END, 0);

	for(i = 0; i < prog_hdr_num; i++, p_prog_header++) {
		if(p_prog_header->p_filesz > 0
		   || p_prog_header->p_offset + p_prog_header->p_filesz > file_size
		   || p_prog_header->p_vaddr + p_prog_header->p_memsz >= KERNEL_MEM_BASE) {
			mm_pmo_unmap(p_info, p_pmo);
			mm_pmo_free(p_pmo);
			vfs_close(fd);
			pm_set_errno(ENOEXEC);
			return ENOEXEC;

		}
	}

	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);
	vfs_close(fd);
	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

void* load_elf(char* path)
{
	u32 fd;
	k_status status;
	ppmo_t p_head_pmo;
	ppmo_t p_seg_pmo;
	void* entry;
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;
	Elf32_Ehdr* p_head;
	u32 prog_hdr_num;
	u32	prog_hdr_off;
	Elf32_Phdr* p_prog_header;
	u32 i;
	u32 attr;
	char* p_mem;
	pmsg_read_info_t p_seg_info;
	pmsg_read_data_t p_seg_data;

	//Get program header offset&entery address
	fd = vfs_open(path, O_RDONLY, 0);

	if(!OPERATE_SUCCESS) {
		pm_exit_thrd(pm_get_errno());
	}

	//Create buffer
	p_head_pmo = mm_pmo_create(sizeof(Elf32_Ehdr) + sizeof(msg_read_info_t));

	if(p_head_pmo == NULL) {
		vfs_close(fd);
		pm_exit_thrd(EFAULT);
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_head_pmo, false);

	if(p_info == NULL) {
		mm_pmo_unmap(p_info, p_head_pmo);
		mm_pmo_free(p_head_pmo);
		vfs_close(fd);
		pm_exit_thrd(EFAULT);
	}

	p_info->len = sizeof(Elf32_Phdr) * prog_hdr_num;
	status = vfs_read(fd, p_head_pmo);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_head_pmo);
		mm_pmo_free(p_head_pmo);
		vfs_close(fd);
		pm_exit_thrd(status);
	}

	p_data = (pmsg_read_data_t)p_info;
	p_head = (Elf32_Ehdr*)(&(p_data->data));
	entry = p_head->e_entry;
	prog_hdr_off = p_head->e_phoff;
	prog_hdr_num = p_head->e_phnum;

	//Get segments
	fs_seek(fd, SEEK_SET, p_head->e_phoff);
	mm_pmo_unmap(p_info, p_head_pmo);
	mm_pmo_free(p_head_pmo);

	//Create buffer
	p_head_pmo = mm_pmo_create(sizeof(Elf32_Phdr) * prog_hdr_num
	                           + sizeof(msg_read_info_t));

	if(p_head_pmo == NULL) {
		vfs_close(fd);
		pm_exit_thrd(EFAULT);
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_head_pmo, false);

	if(p_info == NULL) {
		mm_pmo_unmap(p_info, p_head_pmo);
		mm_pmo_free(p_head_pmo);
		vfs_close(fd);
		pm_exit_thrd(EFAULT);
	}

	p_info->len = sizeof(Elf32_Phdr) * prog_hdr_num;
	status = vfs_read(fd, p_head_pmo);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_head_pmo);
		mm_pmo_free(p_head_pmo);
		vfs_close(fd);
		pm_exit_thrd(status);
	}

	p_data = (pmsg_read_data_t)p_info;
	p_prog_header = &(p_data->data);

	//Load segments
	for(i = 0; i < prog_hdr_num; i++, p_prog_header++) {
		//Allocate pages
		attr = 0;

		if(p_prog_header->p_flags & PF_X) {
			attr |= PAGE_EXECUTABLE;
		}

		if(p_prog_header->p_flags & PF_W) {
			attr |= PAGE_WRITEABLE;
		}

		p_mem = mm_virt_alloc(p_prog_header->p_vaddr,
		                      p_prog_header->p_memsz,
		                      MEM_USER | MEM_RESERVE | MEM_COMMIT, attr);

		if(p_mem == NULL) {
			mm_pmo_unmap(p_info, p_head_pmo);
			mm_pmo_free(p_head_pmo);
			vfs_close(fd);
			pm_exit_thrd(EFAULT);
		}

		//Clear pages
		rtl_memset(p_mem + p_prog_header->file_size,
		           0, p_prog_header->p_memsz - p_prog_header->file_size);

		//Load segment
		if(p_prog_header->file_size > 0) {
			p_seg_pmo = mm_pmo_create(p_prog_header->p_filesz
			                          + sizeof(msg_read_info_t));

			if(p_seg_pmo == NULL) {
				mm_pmo_unmap(p_info, p_head_pmo);
				mm_pmo_free(p_head_pmo);
				vfs_close(fd);
				pm_exit_thrd(EFAULT);
			}

			p_seg_info = mm_pmo_map(NULL, p_seg_pmo, false);

			if(p_seg_info == NULL) {
				mm_pmo_free(p_seg_pmo);
				mm_pmo_unmap(p_info, p_head_pmo);
				mm_pmo_free(p_head_pmo);
				vfs_close(fd);
				pm_exit_thrd(EFAULT);
			}

			p_seg_info->len = p_prog_header->p_filesz;
			vfs_seek(fd, SEEK_SET, p_prog_header->p_offset);
			status = vfs_read(fd, p_seg_pmo);

			if(status != ESUCCESS) {
				mm_pmo_unmap(p_seg_info, p_seg_pmo);
				mm_pmo_free(p_seg_pmo);
				mm_pmo_unmap(p_info, p_head_pmo);
				mm_pmo_free(p_head_pmo);
				vfs_close(fd);
				pm_exit_thrd(status);
			}

			p_seg_data = (pmsg_read_data_t)p_seg_info;
			rtl_memcpy(p_mem, &(p_seg_data->data), p_seg_data->len);
			mm_pmo_unmap(p_seg_info, p_seg_pmo);
			mm_pmo_free(p_seg_pmo);

		}
	}

	pm_set_errno(ESUCCESS);
	return entry;
}
