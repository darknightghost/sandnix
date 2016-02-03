/*  multiboot2.h - Multiboot 2 header file.  */
/*  Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
	Edited by 暗夜幽灵，2016
 */


/*
	This file is copied from grub2
*/
#pragma once
#include "../../common/common.h"

//How many bytes from the start of the file we search for the header.
#define MULTIBOOT_SEARCH				32768
#define MULTIBOOT_HEADER_ALIGN			8

/* The magic field should contain this.  */
#define MULTIBOOT2_HEADER_MAGIC			0xe85250d6

/* This should be in %eax.  */
#define MULTIBOOT2_BOOTLOADER_MAGIC		0x36d76289

/* Alignment of multiboot modules.  */
#define MULTIBOOT_MOD_ALIGN				0x00001000

/* Alignment of the multiboot info structure.  */
#define MULTIBOOT_INFO_ALIGN			0x00000008

/* Flags set in the 'flags' member of the multiboot header.  */

#define MULTIBOOT_TAG_ALIGN                  8
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16

#define MULTIBOOT_HEADER_TAG_END				  0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT_HEADER_TAG_ADDRESS			  2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS		  3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS		  4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER		  5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN		  6

#define MULTIBOOT_ARCHITECTURE_I386		 0
#define MULTIBOOT_ARCHITECTURE_MIPS32	 4
#define MULTIBOOT_HEADER_TAG_OPTIONAL	 1

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED	1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED	2

#ifndef _ASM

typedef u8	multiboot_uint8_t;
typedef u16	multiboot_uint16_t;
typedef u32	multiboot_uint32_t;
typedef u64	multiboot_uint64_t;

#pragma	pack(push)
#pragma	pack(1)
typedef	struct	_multiboot_header_t {
	multiboot_uint32_t magic;			//Must be MULTIBOOT_MAGIC - see above.
	multiboot_uint32_t architecture;	//ISA
	multiboot_uint32_t header_length;	//Total header length.
	multiboot_uint32_t checksum;		//The above fields plus this one must equal 0 mod 2^32.
} multiboot_header_t, *pmultiboot_header_t;

typedef	struct	_multiboot_header_tag_t {
	multiboot_uint16_t type;
	multiboot_uint16_t flags;
	multiboot_uint32_t size;
} multiboot_header_tag_t, *pmultiboot_header_tag_t;

typedef	struct	_multiboot_header_tag_information_request_t {
	multiboot_header_tag_t	header;
	multiboot_uint32_t		requests[0];
} multiboot_header_tag_information_request_t, *pmultiboot_header_tag_information_request_t;

typedef	struct	_multiboot_header_tag_address_t {
	multiboot_header_tag_t	header;
	multiboot_uint32_t		header_addr;
	multiboot_uint32_t		load_addr;
	multiboot_uint32_t		load_end_addr;
	multiboot_uint32_t		bss_end_addr;
} multiboot_header_tag_address_t, *pmultiboot_header_tag_address_t;

typedef	struct	_multiboot_header_tag_entry_address_t {
	multiboot_header_tag_t	header;
	multiboot_uint32_t		entry_addr;
} multiboot_header_tag_entry_address_t, *pmultiboot_header_tag_entry_address_t;

typedef	struct	_multiboot_header_tag_console_flags_t {
	multiboot_header_tag_t	header;
	multiboot_uint32_t		console_flags;
} multiboot_header_tag_console_flags_t, *pmultiboot_header_tag_console_flags_t;

typedef	struct	_multiboot_header_tag_framebuffer_t {
	multiboot_header_tag_t	header;
	multiboot_uint32_t		width;
	multiboot_uint32_t		height;
	multiboot_uint32_t		depth;
} multiboot_header_tag_framebuffer_t, *pmultiboot_header_tag_framebuffer_t;

typedef	struct	_multiboot_header_tag_module_align_t {
	multiboot_header_tag_t	header;
	multiboot_uint32_t		width;
	multiboot_uint32_t		height;
	multiboot_uint32_t		depth;
} multiboot_header_tag_module_align_t, *pmultiboot_header_tag_module_align_t;

typedef	struct	_multiboot_color_t {
	multiboot_uint8_t		red;
	multiboot_uint8_t		green;
	multiboot_uint8_t		blue;
} multiboot_color_t, *pmultiboot_color_t;

typedef	struct	_multiboot_mmap_entry_t {
	multiboot_uint64_t addr;
	multiboot_uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE			1
#define MULTIBOOT_MEMORY_RESERVED			2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE	3
#define MULTIBOOT_MEMORY_NVS				4
#define MULTIBOOT_MEMORY_BADRAM				5
	multiboot_uint32_t type;
	multiboot_uint32_t zero;
} multiboot_mmap_entry_t, *pmultiboot_mmap_entry_t;

typedef	multiboot_mmap_entry_t multiboot_memory_map_t;

typedef	struct	_multiboot_tag_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
} multiboot_tag_t, *pmultiboot_tag_t;

typedef	struct	_multiboot_tag_string_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	char string[0];
} multiboot_tag_string_t, *pmultiboot_tag_string_t;

typedef	struct	_multiboot_tag_module_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t mod_start;
	multiboot_uint32_t mod_end;
	char cmdline[0];
} multiboot_tag_module_t, *pmultiboot_tag_module_t;

typedef	struct	_multiboot_tag_basic_meminfo_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t mem_lower;
	multiboot_uint32_t mem_upper;
} multiboot_tag_basic_meminfo_t, *pmultiboot_tag_basic_meminfo_t;

typedef	struct	_multiboot_tag_bootdev_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t biosdev;
	multiboot_uint32_t slice;
	multiboot_uint32_t part;
} multiboot_tag_bootdev_t, *pmultiboot_tag_bootdev_t;

typedef	struct	_multiboot_tag_mmap_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t entry_size;
	multiboot_uint32_t entry_version;
	multiboot_mmap_entry_t entries[0];
} multiboot_tag_mmap_t, *pmultiboot_tag_mmap_t;

typedef	struct	_multiboot_vbe_info_block_t {
	multiboot_uint8_t external_specification[512];
} multiboot_vbe_info_block_t, *pmultiboot_vbe_info_block_t;

typedef	struct	_multiboot_vbe_mode_info_block_t {
	multiboot_uint8_t external_specification[256];
} multiboot_vbe_mode_info_block_t, *pmultiboot_vbe_mode_info_block_t;

typedef	struct	_multiboot_tag_vbe_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;

	multiboot_uint16_t vbe_mode;
	multiboot_uint16_t vbe_interface_seg;
	multiboot_uint16_t vbe_interface_off;
	multiboot_uint16_t vbe_interface_len;

	multiboot_vbe_info_block_t vbe_control_info;
	multiboot_vbe_mode_info_block_t vbe_mode_info;
} multiboot_tag_vbe_t, *pmultiboot_tag_vbe_t;

typedef	struct	_multiboot_tag_framebuffer_common_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;

	multiboot_uint64_t framebuffer_addr;
	multiboot_uint32_t framebuffer_pitch;
	multiboot_uint32_t framebuffer_width;
	multiboot_uint32_t framebuffer_height;
	multiboot_uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT	2
	multiboot_uint8_t framebuffer_type;
	multiboot_uint16_t reserved;
} multiboot_tag_framebuffer_common_t, *pmultiboot_tag_framebuffer_common_t;

typedef	struct	_multiboot_tag_framebuffer_t {
	multiboot_tag_framebuffer_common_t common;

	union {
		struct {
			multiboot_uint16_t framebuffer_palette_num_colors;
			multiboot_color_t framebuffer_palette[0];
		};
		struct {
			multiboot_uint8_t framebuffer_red_field_position;
			multiboot_uint8_t framebuffer_red_mask_size;
			multiboot_uint8_t framebuffer_green_field_position;
			multiboot_uint8_t framebuffer_green_mask_size;
			multiboot_uint8_t framebuffer_blue_field_position;
			multiboot_uint8_t framebuffer_blue_mask_size;
		};
	};
} multiboot_tag_framebuffer_t, *pmultiboot_tag_framebuffer_t;

typedef	struct	_multiboot_tag_elf_sections_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t num;
	multiboot_uint32_t entsize;
	multiboot_uint32_t shndx;
	char sections[0];
} multiboot_tag_elf_sections_t, *pmultiboot_tag_elf_sections_t;

typedef	struct	_multiboot_tag_apm_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint16_t version;
	multiboot_uint16_t cseg;
	multiboot_uint32_t offset;
	multiboot_uint16_t cseg_16;
	multiboot_uint16_t dseg;
	multiboot_uint16_t flags;
	multiboot_uint16_t cseg_len;
	multiboot_uint16_t cseg_16_len;
	multiboot_uint16_t dseg_len;
} multiboot_tag_apm_t, *pmultiboot_tag_apm_t;

typedef	struct	_multiboot_tag_efi32_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t pointer;
} multiboot_tag_efi32_t, *pmultiboot_tag_efi32_t;

typedef	struct	_multiboot_tag_efi64_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint64_t pointer;
} multiboot_tag_efi64_t, *pmultiboot_tag_efi64_t;

typedef	struct	_multiboot_tag_smbios_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint8_t major;
	multiboot_uint8_t minor;
	multiboot_uint8_t reserved[6];
	multiboot_uint8_t tables[0];
} multiboot_tag_smbios_t, *pmultiboot_tag_smbios_t;

typedef	struct	_multiboot_tag_old_acpi_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint8_t rsdp[0];
} multiboot_tag_old_acpi_t, *pmultiboot_tag_old_acpi_t;

typedef	struct	_multiboot_tag_new_acpi_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint8_t rsdp[0];
} multiboot_tag_new_acpi_t, *pmultiboot_tag_new_acpi_t;

typedef	struct	_multiboot_tag_network_t {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint8_t dhcpack[0];
} multiboot_tag_network_t, *pmultiboot_tag_network_t;

#pragma	pack(pop)
#endif
