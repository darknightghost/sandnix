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

#ifndef	SETUP_H_INCLUDE
#define	SETUP_H_INCLUDE

#include "../mm/mm.h"
#include "../../common/common.h"

#ifdef	X86

#define	TMP_PAGE_MEM_BASE		(((KERNEL_MAX_SIZE+(KERNEL_BASE-VIRTUAL_ADDR_OFFSET))/4096\
                                  +((KERNEL_MAX_SIZE+(KERNEL_BASE-VIRTUAL_ADDR_OFFSET))%4096?1:0))\
                                 *4096)
//A temporary marcos used to compute TMP_PAGE_SIZE
#define	TMP_SIZE_MID1			(TMP_PAGE_MEM_BASE/4096*4+TMP_PAGE_TABLE_BASE+4096)
#define	TMP_SIZE_MID2			((TMP_SIZE_MID1/4096+(TMP_SIZE_MID1%4096?1:0))*4+TMP_SIZE_MID1)
#define	TMP_SIZE_MID3			((TMP_SIZE_MID2/4096+1)*4096)

#define	TMP_PAGE_SIZE			(TMP_SIZE_MID3+(TMP_SIZE_MID3/4096?0:4096))
#define	TMP_PAGE_NUM			(TMP_PAGE_SIZE/4096)

#define	TMP_PAGE_TABLE_BASE		(TMP_PAGE_MEM_BASE+4096)
#define	TMP_PAGED_MEM_SIZE		(TMP_PAGE_SIZE+TMP_PAGE_TABLE_BASE)
#define	TMP_PDT_BASE			TMP_PAGE_MEM_BASE
#define	PT_MAPPING_PAGE			(TMP_PAGED_MEM_SIZE/4096*sizeof(pte_t)+TMP_PAGE_TABLE_BASE+VIRTUAL_ADDR_OFFSET)
#define	PT_MAPPING_ADDR			(TMP_PAGED_MEM_SIZE+VIRTUAL_ADDR_OFFSET)

//Segment selectors
#define	DESCRIPTOR_SIZE			8
//Kernel segmends
#define	SELECTOR_K_DATA			(DESCRIPTOR_SIZE * 1)
#define	SELECTOR_K_CODE			(DESCRIPTOR_SIZE * 2)
//User segment
#define	SELECTOR_U_DATA			(DESCRIPTOR_SIZE * 3 | 3)
#define	SELECTOR_U_CODE			(DESCRIPTOR_SIZE * 4 | 3)
#define	SELECTOR_BASIC_VIDEO	(DESCRIPTOR_SIZE * 5)
#define	SELECTOR_TSS			(DESCRIPTOR_SIZE * 6)
//Video segment
#define	BASIC_VIDEO_BASE_ADDR	((void*)0xC00B8000)

#endif	//X86

#endif	//!	SETUP_H_INCLUDE
