#! /usr/bin/env python3
# -*- coding: utf-8 -*-

'''
    Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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
'''

import struct
from elf import elf

class KernelHeaderNotFound(Exception):
    pass

class KernelHeaderNotSupport(Exception):
    pass

class kernel_header:
    KERNEL_HEADER_MAGIC = 0x444E4153

    def __init__(self, image):
        self.image = image
        if image.machine in [0x0003]:
            for ph in image.program_headers:
                if ph.flags & elf.program_header.PF_X != 0:
                    if self.search_kernel_header(ph.data):
                        self.ph = ph
                        return
            raise KernelHeaderNotFound()
        else:
            raise KernelHeaderNotSupport()

    def __str__(self):
        ret = "Kernel header info:\n" \
                + "Magic : 0x%.8X\n"%(self.magic) \
                + "Code start : 0x%.8X\n"%(self.code_start) \
                + "Code size : 0x%.8X\n"%(self.code_size) \
                + "Data start : 0x%.8X\n"%(self.data_start) \
                + "Data size : 0x%.8X\n"%(self.data_size) \
                + "Kernel header size : 0x%.8X\n"%(self.header_size) \
                + "Checksum : 0x%.8X\n"%(self.checksum)
        return ret

    def search_kernel_header(self, data):
        offset = 0
        while offset < len(data):
            magic = struct.unpack("<I",data[offset : offset + 4])[0]
            if magic == self.KERNEL_HEADER_MAGIC:
                self.magic = magic
                self.code_start = struct.unpack("<I", data[offset + 4 : offset + 8])[0]
                self.code_size = struct.unpack("<I", data[offset + 8 : offset + 12])[0]
                self.data_start = struct.unpack("<I", data[offset + 12 : offset + 16])[0]
                self.data_size = struct.unpack("<I", data[offset + 16 : offset + 20])[0]
                self.header_size = struct.unpack("<I", data[offset + 20 : offset + 24])[0]
                self.checksum = struct.unpack("<I", data[offset + 24 : offset + 28])[0]

                if ((self.magic + self.code_start + self.code_size \
                        + self.data_start + self.data_size \
                        + self.header_size + self.checksum) & 0xFFFFFFFF) == 0:
                    self.offset = offset
                    return True

            offset = offset + 8
        return False

    def update(self):
        #Compute checksum
        self.checksum = self.magic + self.code_start + self.code_size \
                + self.data_start + self.data_size + self.header_size;
        self.checksum = self.checksum & 0xFFFFFFFF
        self.checksum = 0x100000000 - self.checksum

        #code_start
        packed = struct.pack("<I", self.code_start);
        self.ph.data = self.ph.data[: self.offset + 4] + packed \
                + self.ph.data[self.offset + 8 :]

        #code_size
        packed = struct.pack("<I", self.code_size);
        self.ph.data = self.ph.data[: self.offset + 8] + packed \
                + self.ph.data[self.offset + 12 :]

        #data_start
        packed = struct.pack("<I", self.data_start);
        self.ph.data = self.ph.data[: self.offset + 12] + packed \
                + self.ph.data[self.offset + 16 :]

        #data_size
        packed = struct.pack("<I", self.data_size);
        self.ph.data = self.ph.data[: self.offset + 16] + packed \
                + self.ph.data[self.offset + 20 :]

        #checksum
        packed = struct.pack("<I", self.checksum);
        self.ph.data = self.ph.data[: self.offset + 24] + packed \
                + self.ph.data[self.offset + 28 :]

        return
