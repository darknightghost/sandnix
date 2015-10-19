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

#ifndef	ELF_H_INCLUDE
#define	ELF_H_INCLUDE

#include "../../../../common/common.h"

#pragma	pack(1)
/* 32-bit ELF base types. */
typedef	u32		Elf32_Addr;
typedef	u16		Elf32_Half;
typedef	u32		Elf32_Off;
typedef	s32		Elf32_Sword;
typedef	u32		Elf32_Word;

#define	EI_NIDENT			16

typedef struct elf32_hdr {
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half		e_type;
	Elf32_Half		e_machine;
	Elf32_Word		e_version;
	Elf32_Addr		e_entry;  /* Entry point */
	Elf32_Off		e_phoff;
	Elf32_Off		e_shoff;
	Elf32_Word		e_flags;
	Elf32_Half		e_ehsize;
	Elf32_Half		e_phentsize;
	Elf32_Half		e_phnum;
	Elf32_Half		e_shentsize;
	Elf32_Half		e_shnum;
	Elf32_Half		e_shstrndx;
} Elf32_Ehdr;

/*  Legal values for e_type (object file type).  */

#define ET_NONE		0		/*  No file type */
#define ET_REL		1		/*  Relocatable file */
#define ET_EXEC		2		/*  Executable file */
#define ET_DYN		3		/*  Shared object file */
#define ET_CORE		4		/*  Core file */
#define	ET_NUM		5		/*  Number of defined types */
#define ET_LOOS		0xFE00		/*  OS-specific range start */
#define ET_HIOS		0xFEFF		/*  OS-specific range end */
#define ET_LOPROC	0xFF00		/*  Processor-specific range start */
#define ET_HIPROC	0xFFFF		/*  Processor-specific range end */

/*  Legal values for e_machine (architecture).  */

#define EM_NONE		 0		/*  No machine */
#define EM_M32		 1		/*  AT&T WE 32100 */
#define EM_SPARC	 2		/*  SUN SPARC */
#define EM_386		 3		/*  Intel 80386 */
#define EM_68K		 4		/*  Motorola m68k family */
#define EM_88K		 5		/*  Motorola m88k family */
#define EM_860		 7		/*  Intel 80860 */
#define EM_MIPS		 8		/*  MIPS R3000 big-endian */
#define EM_S370		 9		/*  IBM System/370 */
#define EM_MIPS_RS3_LE	10		/*  MIPS R3000 little-endian */

#define EM_PARISC	15		/*  HPPA */
#define EM_VPP500	17		/*  Fujitsu VPP500 */
#define EM_SPARC32PLUS	18		/*  Sun's "v8plus" */
#define EM_960		19		/*  Intel 80960 */
#define EM_PPC		20		/*  PowerPC */
#define EM_PPC64	21		/*  PowerPC 64-bit */
#define EM_S390		22		/*  IBM S390 */

#define EM_V800		36		/*  NEC V800 series */
#define EM_FR20		37		/*  Fujitsu FR20 */
#define EM_RH32		38		/*  TRW RH-32 */
#define EM_RCE		39		/*  Motorola RCE */
#define EM_ARM		40		/*  ARM */
#define EM_FAKE_ALPHA	41		/*  Digital Alpha */
#define EM_SH		42		/*  Hitachi SH */
#define EM_SPARCV9	43		/*  SPARC v9 64-bit */
#define EM_TRICORE	44		/*  Siemens Tricore */
#define EM_ARC		45		/*  Argonaut RISC Core */
#define EM_H8_300	46		/*  Hitachi H8/300 */
#define EM_H8_300H	47		/*  Hitachi H8/300H */
#define EM_H8S		48		/*  Hitachi H8S */
#define EM_H8_500	49		/*  Hitachi H8/500 */
#define EM_IA_64	50		/*  Intel Merced */
#define EM_MIPS_X	51		/*  Stanford MIPS-X */
#define EM_COLDFIRE	52		/*  Motorola Coldfire */
#define EM_68HC12	53		/*  Motorola M68HC12 */
#define EM_MMA		54		/*  Fujitsu MMA Multimedia Accelerator*/
#define EM_PCP		55		/*  Siemens PCP */
#define EM_NCPU		56		/*  Sony nCPU embeeded RISC */
#define EM_NDR1		57		/*  Denso NDR1 microprocessor */
#define EM_STARCORE	58		/*  Motorola Start*Core processor */
#define EM_ME16		59		/*  Toyota ME16 processor */
#define EM_ST100	60		/*  STMicroelectronic ST100 processor */
#define EM_TINYJ	61		/*  Advanced Logic Corp. Tinyj emb.fam*/
#define EM_X86_64	62		/*  AMD x86-64 architecture */
#define EM_PDSP		63		/*  Sony DSP Processor */

#define EM_FX66		66		/*  Siemens FX66 microcontroller */
#define EM_ST9PLUS	67		/*  STMicroelectronics ST9+ 8/16 mc */
#define EM_ST7		68		/*  STmicroelectronics ST7 8 bit mc */
#define EM_68HC16	69		/*  Motorola MC68HC16 microcontroller */
#define EM_68HC11	70		/*  Motorola MC68HC11 microcontroller */
#define EM_68HC08	71		/*  Motorola MC68HC08 microcontroller */
#define EM_68HC05	72		/*  Motorola MC68HC05 microcontroller */
#define EM_SVX		73		/*  Silicon Graphics SVx */
#define EM_ST19		74		/*  STMicroelectronics ST19 8 bit mc */
#define EM_VAX		75		/*  Digital VAX */
#define EM_CRIS		76		/*  Axis Communications 32-bit embedded processor */
#define EM_JAVELIN	77		/*  Infineon Technologies 32-bit embedded processor */
#define EM_FIREPATH	78		/*  Element 14 64-bit DSP Processor */
#define EM_ZSP		79		/*  LSI Logic 16-bit DSP Processor */
#define EM_MMIX		80		/*  Donald Knuth's educational 64-bit processor */
#define EM_HUANY	81		/*  Harvard University machine-independent object files */
#define EM_PRISM	82		/*  SiTera Prism */
#define EM_AVR		83		/*  Atmel AVR 8-bit microcontroller */
#define EM_FR30		84		/*  Fujitsu FR30 */
#define EM_D10V		85		/*  Mitsubishi D10V */
#define EM_D30V		86		/*  Mitsubishi D30V */
#define EM_V850		87		/*  NEC v850 */
#define EM_M32R		88		/*  Mitsubishi M32R */
#define EM_MN10300	89		/*  Matsushita MN10300 */
#define EM_MN10200	90		/*  Matsushita MN10200 */
#define EM_PJ		91		/*  picoJava */
#define EM_OPENRISC	92		/*  OpenRISC 32-bit embedded processor */
#define EM_ARC_A5	93		/*  ARC Cores Tangent-A5 */
#define EM_XTENSA	94		/*  Tensilica Xtensa Architecture */
#define EM_AARCH64	183		/*  ARM AARCH64 */
#define EM_TILEPRO	188		/*  Tilera TILEPro */
#define EM_MICROBLAZE	189		/*  Xilinx MicroBlaze */
#define EM_TILEGX	191		/*  Tilera TILE-Gx */
#define EM_NUM		192

/* These constants are for the segment types stored in the image headers */
#define	PT_NULL			0
#define	PT_LOAD			1
#define	PT_DYNAMIC		2
#define	PT_INTERP		3
#define	PT_NOTE			4
#define	PT_SHLIB		5
#define	PT_PHDR			6
#define	PT_TLS			7               /* Thread local storage segment */
#define	PT_LOOS			0x60000000      /* OS-specific */
#define	PT_HIOS			0x6fffffff      /* OS-specific */
#define	PT_LOPROC		0x70000000
#define	PT_HIPROC		0x7fffffff
#define	PT_GNU_EH_FRAME	0x6474e550

#define	PT_GNU_STACK	(PT_LOOS + 0x474e551)

/* These constants define the permissions on sections in the program
   header, p_flags. */
#define PF_R		0x4
#define PF_W		0x2
#define PF_X		0x1

typedef struct elf32_phdr {
	Elf32_Word		p_type;
	Elf32_Off		p_offset;
	Elf32_Addr		p_vaddr;
	Elf32_Addr		p_paddr;
	Elf32_Word		p_filesz;
	Elf32_Word		p_memsz;
	Elf32_Word		p_flags;
	Elf32_Word		p_align;
} Elf32_Phdr;

/* sh_type */
#define SHT_NULL		0
#define SHT_PROGBITS	1
#define SHT_SYMTAB		2
#define SHT_STRTAB		3
#define SHT_RELA		4
#define SHT_HASH		5
#define SHT_DYNAMIC		6
#define SHT_NOTE		7
#define SHT_NOBITS		8
#define SHT_REL			9
#define SHT_SHLIB		10
#define SHT_DYNSYM		11
#define SHT_NUM			12
#define SHT_LOPROC		0x70000000
#define SHT_HIPROC		0x7fffffff
#define SHT_LOUSER		0x80000000
#define SHT_HIUSER		0xffffffff

/* sh_flags */
#define SHF_WRITE		0x1
#define SHF_ALLOC		0x2
#define SHF_EXECINSTR	0x4
#define SHF_MASKPROC	0xf0000000

/* special section indexes */
#define SHN_UNDEF		0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC		0xff00
#define SHN_HIPROC		0xff1f
#define SHN_ABS			0xfff1
#define SHN_COMMON		0xfff2
#define SHN_HIRESERVE	0xffff

typedef struct {
	Elf32_Word		sh_name;
	Elf32_Word		sh_type;
	Elf32_Word		sh_flags;
	Elf32_Addr		sh_addr;
	Elf32_Off		sh_offset;
	Elf32_Word		sh_size;
	Elf32_Word		sh_link;
	Elf32_Word		sh_info;
	Elf32_Word		sh_addralign;
	Elf32_Word		sh_entsize;
} Elf32_Shdr;

#pragma	pack()

k_status	check_elf(char* path);
void*		load_elf(char* path);

#endif	//!	ELF_H_INCLUDE
