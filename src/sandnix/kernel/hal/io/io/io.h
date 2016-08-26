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

#include "../../../../../common/common.h"

#if defined X86
    #include "arch/x86/io.h"
#endif

#if defined X86

    //IN
    u8		hal_io_in_8(address_t port);
    u16		hal_io_in_16(address_t port);
    u32		hal_io_in_32(address_t port);

    //INS
    void	hal_io_ins_8(void* dest, size_t count, address_t port);
    void	hal_io_ins_16(void* dest, size_t count, address_t port);
    void	hal_io_ins_32(void* dest, size_t count, address_t port);

    //OUT
    void	hal_io_out_8(address_t port, u8 data);
    void	hal_io_out_16(address_t port, u16 data);
    void	hal_io_out_32(address_t port, u32 data);

    //OUTS
    void	hal_io_outs_8(address_t port, size_t count, void* src);
    void	hal_io_outs_16(address_t port, size_t count, void* src);
    void	hal_io_outs_32(address_t port, size_t count, void* src);

#endif
