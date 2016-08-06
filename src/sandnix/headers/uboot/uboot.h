/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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
#include "../../../common/common.h"

//Copied from uboot

/*
 * Operating System Codes
 */
#define IH_OS_INVALID		0	/* Invalid OS	*/
#define IH_OS_OPENBSD		1	/* OpenBSD	*/
#define IH_OS_NETBSD		2	/* NetBSD	*/
#define IH_OS_FREEBSD		3	/* FreeBSD	*/
#define IH_OS_4_4BSD		4	/* 4.4BSD	*/
#define IH_OS_LINUX			5	/* Linux	*/
#define IH_OS_SVR4			6	/* SVR4		*/
#define IH_OS_ESIX			7	/* Esix		*/
#define IH_OS_SOLARIS		8	/* Solaris	*/
#define IH_OS_IRIX			9	/* Irix		*/
#define IH_OS_SCO			10	/* SCO		*/
#define IH_OS_DELL			11	/* Dell		*/
#define IH_OS_NCR			12	/* NCR		*/
#define IH_OS_LYNXOS		13	/* LynxOS	*/
#define IH_OS_VXWORKS		14	/* VxWorks	*/
#define IH_OS_PSOS			15	/* pSOS		*/
#define IH_OS_QNX			16	/* QNX		*/
#define IH_OS_U_BOOT		17	/* Firmware	*/
#define IH_OS_RTEMS			18	/* RTEMS	*/
#define IH_OS_ARTOS			19	/* ARTOS	*/
#define IH_OS_UNITY			20	/* Unity OS	*/
#define IH_OS_INTEGRITY		21	/* INTEGRITY	*/

/*
 * CPU Architecture Codes (supported by Linux)
 */
#define IH_ARCH_INVALID		0	/* Invalid CPU	*/
#define IH_ARCH_ALPHA		1	/* Alpha	*/
#define IH_ARCH_ARM			2	/* ARM		*/
#define IH_ARCH_I386		3	/* Intel x86	*/
#define IH_ARCH_IA64		4	/* IA64		*/
#define IH_ARCH_MIPS		5	/* MIPS		*/
#define IH_ARCH_MIPS64		6	/* MIPS	 64 Bit */
#define IH_ARCH_PPC			7	/* PowerPC	*/
#define IH_ARCH_S390		8	/* IBM S390	*/
#define IH_ARCH_SH			9	/* SuperH	*/
#define IH_ARCH_SPARC		10	/* Sparc	*/
#define IH_ARCH_SPARC64		11	/* Sparc 64 Bit */
#define IH_ARCH_M68K		12	/* M68K		*/
#define IH_ARCH_NIOS		13	/* Nios-32	*/
#define IH_ARCH_MICROBLAZE	14	/* MicroBlaze   */
#define IH_ARCH_NIOS2		15	/* Nios-II	*/
#define IH_ARCH_BLACKFIN	16	/* Blackfin	*/
#define IH_ARCH_AVR32		17	/* AVR32	*/
#define IH_ARCH_ST200	    18	/* STMicroelectronics ST200  */

/*
 * Image Types

 * "Standalone Programs" are directly runnable in the environment
 * provided by U-Boot; it is expected that (if they behave
 * well) you can continue to work in U-Boot after return from
 * the Standalone Program.
 * "OS Kernel Images" are usually images of some Embedded OS which
 * will take over control completely. Usually these programs
 * will install their own set of exception handlers, device
 * drivers, set up the MMU, etc. - this means, that you cannot
 * expect to re-enter U-Boot except by resetting the CPU.
 * "RAMDisk Images" are more or less just data blocks, and their
 * parameters (address, size) are passed to an OS kernel that is
 * being started.
 * "Multi-File Images" contain several images, typically an OS
 * (Linux) kernel image and one or more data images like
 * RAMDisks. This construct is useful for instance when you want
 * to boot over the network using BOOTP etc., where the boot
 * server provides just a single image file, but you want to get
 * for instance an OS kernel and a RAMDisk image.

 * "Multi-File Images" start with a list of image sizes, each
 * image size (in bytes) specified by an "uint32_t" in network
 * byte order. This list is terminated by an "(uint32_t)0".
 * Immediately after the terminating 0 follow the images, one by
 * one, all aligned on "uint32_t" boundaries (size rounded up to
 * a multiple of 4 bytes - except for the last file).

 * "Firmware Images" are binary images containing firmware (like
 * U-Boot or FPGA images) which usually will be programmed to
 * flash memory.

 * "Script files" are command sequences that will be executed by
 * U-Boot's command interpreter; this feature is especially
 * useful when you configure U-Boot to use a real shell (hush)
 * as command interpreter (=> Shell Scripts).
 */

#define IH_TYPE_INVALID		0	/* Invalid Image		*/
#define IH_TYPE_STANDALONE	1	/* Standalone Program		*/
#define IH_TYPE_KERNEL		2	/* OS Kernel Image		*/
#define IH_TYPE_RAMDISK		3	/* RAMDisk Image		*/
#define IH_TYPE_MULTI		4	/* Multi-File Image		*/
#define IH_TYPE_FIRMWARE	5	/* Firmware Image		*/
#define IH_TYPE_SCRIPT		6	/* Script file			*/
#define IH_TYPE_FILESYSTEM	7	/* Filesystem Image (any type)	*/
#define IH_TYPE_FLATDT		8	/* Binary Flat Device Tree Blob	*/
#define IH_TYPE_KWBIMAGE	9	/* Kirkwood Boot Image		*/
#define IH_TYPE_IMXIMAGE	10	/* Freescale IMXBoot Image	*/

/*
 * Compression Types
 */
#define IH_COMP_NONE		0	/*  No	 Compression Used	*/
#define IH_COMP_GZIP		1	/* gzip	 Compression Used	*/
#define IH_COMP_BZIP2		2	/* bzip2 Compression Used	*/
#define IH_COMP_LZMA		3	/* lzma  Compression Used	*/
#define IH_COMP_LZO			4	/* lzo   Compression Used	*/

#define IH_MAGIC			0x27051956	/* Image Magic Number		*/
#define IH_NMLEN			32	/* Image Name Length		*/

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */

#ifndef	_ASM
typedef struct image_header {
    u32			ih_magic;			/* Image Header Magic Number	*/
    u32			ih_hcrc;			/* Image Header CRC Checksum	*/
    u32			ih_time;			/* Image Creation Timestamp	*/
    u32			ih_size;			/* Image Data Size		*/
    u32			ih_load;			/* Data	 Load  Address		*/
    u32			ih_ep;				/* Entry Point Address		*/
    u32			ih_dcrc;			/* Image Data CRC Checksum	*/
    u8			ih_os;				/* Operating System		*/
    u8			ih_arch;			/* CPU architecture		*/
    u8			ih_type;			/* Image Type			*/
    u8			ih_comp;			/* Compression Type		*/
    u8			ih_name[IH_NMLEN];	/* Image Name		*/
} __attribute__((aligned(1))) image_header_t;

#define	ATAG_NONE		0x00000000		//The list ends with an ATAG_NONE node.
#define	ATAG_CORE		0x54410001		//The list must start with an ATAG_CORE node.
#define	ATAG_MEM		0x54410002		//It is allowed to have multiple ATAG_MEM nodes.
#define	ATAG_INITRD		0x54410005		//Initrd.
#define ATAG_INITRD2	0x54420005		//Initrd.
#define ATAG_CMDLINE	0x54410009		//Command line: \0 terminated string. 

typedef	struct _uboot_tag_header {
    u32 size;
    u32 tag;
} uboot_tag_header_t, *puboot_tag_header_t;

typedef struct _uboot_tag_core {
    u32 flags;		/* bit 0 = read-only */
    u32 pagesize;
    u32 rootdev;
} uboot_tag_core_t, *puboot_tag_core_t;

typedef struct _uboot_tag_mem32 {
    u32	size;
    u32	start;	/* physical start address */
} uboot_tag_mem32_t, *puboot_tag_mem32_t;;

typedef struct _uboot_tag_initrd {
    u32 start;	/* physical start address */
    u32 size;	/* size of compressed ramdisk image in bytes */
} uboot_tag_initrd_t, *puboot_tag_initrf_t;;

typedef struct _uboot_tag_cmdline {
    char	cmdline[0];	/* this is the minimum size */
} uboot_tag_cmdline_t, *puboot_tag_cmdline_t;

typedef	struct	_uboot_tag {
    uboot_tag_header_t			tag_header;
    union {
        uboot_tag_core_t	tag_core;
        uboot_tag_mem32_t	tag_mem;
        uboot_tag_initrd_t	tag_initrd;
        uboot_tag_cmdline_t	tag_cmdline;
    } data;
} uboot_tag_t, *puboot_tag_t;

#endif	//!	_ASM
