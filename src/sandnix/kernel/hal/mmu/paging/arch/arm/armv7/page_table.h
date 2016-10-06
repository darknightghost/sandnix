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

#include "../../../../../../../../common/common.h"

#include "../../../../../../core/rtl/rtl_defs.h"

/* MMU page table */
//LV1 descriptor
//Types
#define	MMU_PG_FAULT			0x00
#define	MMU_PG_LV2ENT			0x01
#define	MMU_PG_1MB				0x02
#define	MMU_PG_16MB				0x03

//Descriptor type
#define	LV1_PG_TYPE_FAULT1		0x00
#define	LV1_PG_TYPE_FAULT2		0x03
#define	LV1_PG_TYPE_LV2ENT		0x01
#define	LV1_PG_TYPE_LAGEPAGE	0x02

#define	LV1_DESC_TYPE(p_desc) { \
        u32 __tmp_ret; \
        if((p_desc)->type == LV_PG_TYPE_FAULT1 \
           || (p_desc)->type == LV_PG_TYPE_FAULT2) { \
            __tmp_ret = MMU_PG_FAULT; \
        } else if ((p_dest)->type == LV_PG_TYPE_LV2ENT) { \
            __tmp_ret = MMU_PG_LV2ENT; \
        } else { \
            if((p_desc)->type & 0x00040000) { \
                __tmp_ret = MMU_PG_16MB; \
            } else { \
                __tmp_ret = MMU_PG_1MB; \
            } \
        } \
        __tmp_ret; \
    }

#define	LV1_DESC_TYPE_SET(p_desc, desc_type) { \
        if((desc_type) == MMU_PG_FAULT) { \
            (p_desc)->type = LV1_PG_TYPE_FAULT1; \
        } else if((desc_type) == MMU_PG_LV2ENT) { \
            (p_desc)->type = LV1_PG_TYPE_LV2ENT; \
        } else { \
            (p_desc)->type = LV1_PG_TYPE_LAGEPAGE; \
            if((desc_type) == MMU_PG_1MB) { \
                (p_desc)->pg_1MB.zero = 0; \
            } else { \
                (p_desc)->pg_16MB.one = 1; \
            } \
        } \
    }

//Descriptor access permission
#define	PG_AP_NA_NA		0x00	//Privileged mode : no access, user mode : no access
#define	PG_AP_RW_NA		0x01	//Privileged mode : read/write, user mode : no access
#define	PG_AP_RW_RO		0x02	//Privileged mode : read/write, user mode : read-only
#define	PG_AP_RW_RW		0x03	//Privileged mode : read/write, user mode : read/write
#define	PG_AP_RO_NA		0x05	//Privileged mode : read-only, user mode : no access
#define	PG_AP_RO_RO		0x06	//Privileged mode : read-only, user mode : read-only

#define	LV1_SET_AP(p_desc, ap) { \
        (p_desc)->pg_1MB.ap1 = (ap) & 0x03; \
        (p_desc)->pg_1MB.ap2 = ((ap) & 0x04) >> 2; \
    }

//Physical address
#define	MMU_PG_DESC_GET_ADDR(p_desc, mask)	((p_desc)->value & (mask))
#define	MMU_PG_DESC_SET_ADDR(p_desc, addr, mask)	((p_desc)->value = \
        (((p_desc)->value & (~(mask))) \
         | ((addr) & (mask))))

#define	LV1_LV2ENT_ADDR_MASK				0xFFFFFC00
#define	LV1_LV2ENT_GET_ADDR(p_desc)			MMU_PG_DESC_GET_ADDR((p_desc), \
        LV1_LV2ENT_ADDR_MASK)
#define	LV1_LV2ENT_SET_ADDR(p_desc, addr)	MMU_PG_DESC_SET_ADDR((p_desc), \
        (addr), LV1_LV2ENT_ADDR_MASK)

#define	LV1_1MB_ADDR_MASK					0xFFF00000
#define	LV1_1MB_GET_ADDR(p_desc)			MMU_PG_DESC_GET_ADDR((p_desc), \
        LV1_1MB_ADDR_MASK)
#define	LV1_1MB_SET_ADDR(p_desc, addr)		MMU_PG_DESC_SET_ADDR((p_desc), \
        (addr), LV1_1MB_ADDR_MASK)

#define	LV1_16MB_ADDR_MASK					0xFF000000
#define	LV1_16MB_GET_ADDR(p_desc)			MMU_PG_DESC_GET_ADDR((p_desc), \
        LV1_16MB_ADDR_MASK)
#define	LV1_16MB_SET_ADDR(p_desc, addr)		MMU_PG_DESC_SET_ADDR((p_desc), \
        (addr), LV1_16MB_ADDR_MASK)

typedef	union _lv1_pg_desc {
    u32	value;			//Value of the descriptor
    u32	type	: 2;	//Descriptor type

    struct {
        u32	type		: 2;	//Descriptor type
        u32	reserv1		: 1;	//Reserved
        u32	none_secure	: 1;	//Non-secure
        u32	reserv2		: 6;	//Reserved
        u32	lv2_phyaddr	: 22;	//Physical address of level2 table
    } __attribute__((aligned(1))) lv2_entry;

    struct {
        u32	type			: 2;	//Descriptor type
        u32	reserv1			: 8;	//Reserved
        u32	ap1				: 2;	//Access permission
        u32	reserv2			: 3;	//Reserved
        u32	ap2				: 1;	//Access permission
        u32	reserv3			: 2;	//Reserved
        u32	zero			: 1;	//Must be zero
        u32	none_secure		: 1;	//Non-secure
        u32	pg_base_phyaddr	: 12;	//Physical base address of the 1MB page
    } __attribute__((aligned(1))) pg_1MB;

    struct {
        u32	type			: 2;	//Descriptor type
        u32	reserv1			: 8;	//Reserved
        u32	ap1				: 2;	//Access permission
        u32	reserv2			: 3;	//Reserved
        u32	ap2				: 1;	//Access permission
        u32	reserv3			: 2;	//Reserved
        u32	one				: 1;	//Must be one
        u32	none_secure		: 1;	//Non-secure
        u32	reserv4			: 4;	//Reserved
        u32	pg_base_phyaddr	: 8;	//Physical base address of the 16MB page
    } __attribute__((aligned(1))) pg_16MB;

} __attribute__((aligned(1)))lv1_pg_desc_t, *plv1_pg_desc_t;

//LV2 descriptor
#define	MMU_PG_64KB		0x04
#define	MMU_PG_4KB		0x05

#define	LV2_DESC_TYPE(p_desc) ({ \
        u32 __tmp_ret; \
        if((p_desc)->type == LV2_PG_TYPE_FAULT) { \
            __tmp_ret = MMU_PG_FAULT; \
        } else if((p_desc)->type == LV2_PG_TYPE_64KB) { \
            __tmp_ret = MMU_PG_64KB; \
        } else { \
            __tmp_ret = MMU_PG_4KB; \
        } \
        __tmp_ret; \
    })

//Descriptor type
#define	LV2_PG_TYPE_FAULT		0x00
#define	LV2_PG_TYPE_64KB		0x01
#define	LV2_PG_TYPE_4KB1		0x02
#define	LV2_PG_TYPE_4KB2		0x03

#define	LV2_DESC_TYPE_SET(p_desc, desc_type) ({ \
        if((desc_type) == MMU_PG_FAULT) { \
            (p_desc)->type = LV2_PG_TYPE_FAULT; \
        } else if((desc_type) == MMU_PG_64KB) { \
            (p_desc)->type = LV2_PG_TYPE_64KB; \
        } else { \
            (p_desc)->type = LV2_PG_TYPE_4KB1; \
        } \
    })

//Access permission
#define	LV2_GET_AP(p_desc) ((p_desc)->pg_4KB.ap1 | ((p_desc)->pg_4KB.ap2 << 2))

#define	LV2_SET_AP(p_desc, ap) { \
        (p_desc)->pg_64KB.ap1 = (ap) & 0x03; \
        (p_desc)->pg_64KB.ap2 = ((ap) & 0x04) >> 2; \
    }

//Physical addres
#define	LV2_64KB_ADDR_MASK		0xFFFF0000
#define	LV2_64KB_GET_ADDR(p_desc)		MMU_PG_DESC_GET_ADDR((p_desc), \
        LV2_64KB_ADDR_MASK)
#define	LV2_64KB_SET_ADDR(p_desc, addr)	MMU_PG_DESC_SET_ADDR((p_desc), \
        (addr), LV2_64KB_ADDR_MASK)

#define	LV2_4KB_ADDR_MASK		0xFFFFF000
#define	LV2_4KB_GET_ADDR(p_desc)		MMU_PG_DESC_GET_ADDR((p_desc), \
        LV2_4KB_ADDR_MASK)
#define	LV2_4KB_SET_ADDR(p_desc, addr)	MMU_PG_DESC_SET_ADDR((p_desc), \
        (addr), LV2_4KB_ADDR_MASK)

typedef	union _lv2_pg_desc {
    u32	value;			//Value of the descriptor
    u32	type	: 2;	//Descriptor type

    struct {
        u32	type				: 2;	//Descriptor type
        u32	reserv1				: 2;	//Reserved
        u32	ap1					: 2;	//Access permission
        u32	reserv2				: 3;	//Reserved
        u32	ap2					: 1;	//Access permission
        u32 reserv3				: 6;	//Reserved
        u32	pg_base_phyaddr		: 16;	//Physical base address of the 64KB page
    } __attribute__((packed)) pg_64KB;

    struct {
        u32	xn					: 1;	//Never execute
        u32	type				: 1;	//Descriptor type
        u32	reserv1				: 2;	//Reserved
        u32	ap1					: 2;	//Access permission
        u32	reserv2				: 3;	//Reserved
        u32	ap2					: 1;	//Accesss permission
        u32 reserv3				: 2;	//Reserved
        u32	pg_base_phyaddr		: 20;	//Physical base address of the 4KB page
    } __attribute__((aligned(1))) pg_4KB;
} __attribute__((packed))lv2_pg_desc_t, *plv2_pg_desc_t;

typedef	struct	_tlb_entry {
    u32		vitrual_page_number	: 20;
    u32		valid				: 1;
    u32		page_size			: 2;
    u32		phypage_base_addr1	: 9;
    u16		phypage_base_addr2	: 11;
    u16		shareable			: 1;
    u16		none_secure			: 1;
    u16		access_permission	: 3;
} __attribute__((packed)) tlb_entry_t, *ptlb_entry_t;

typedef	struct	_lv1_tbl_info {
    address_t	physical_addr;
    map_t		lv2_info_map;
} lv1_tbl_info_t, *plv1_tbl_info_t;

//4 lv2 page table used an lv2_tbl_info
typedef struct	_lv2_tbl_info {
    address_t	physical_addr;
    u32			ref;
    bool		freeable;
} lv2_tbl_info_t, *plv2_tbl_info_t;
