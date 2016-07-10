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

class Multiboot2HeaderFormatError(Exception):
    pass

class multiboot2:
    ADDR_MAGIC = 0xFFFFFFFF
    def __init__(self, path):
        f = open(path, "rb")
        self.data = f.read()
        f.close()
        
        #header_addr
        self.header_addr = self.ADDR_MAGIC
        self.header_addr_off = self.get_next_offset(0)
        if self.header_addr_off == None:
            raise Multiboot2HeaderFormatError()

        #load_addr
        self.load_addr = self.ADDR_MAGIC
        self.load_addr_off = self.get_next_offset(self.header_addr_off + 4)
        if self.load_addr_off == None:
            raise Multiboot2HeaderFormatError()

        #load_end_addr
        self.load_end_addr = self.ADDR_MAGIC
        self.load_end_addr_off = self.get_next_offset(self.load_addr_off + 4)
        if self.load_end_addr_off == None:
            raise Multiboot2HeaderFormatError()

        #bss_end_addr
        self.bss_end_addr = self.ADDR_MAGIC
        self.bss_end_addr_off = self.get_next_offset(self.load_end_addr_off + 4)
        if self.bss_end_addr_off == None:
            raise Multiboot2HeaderFormatError()

        #entry_addr
        self.entry_addr = self.ADDR_MAGIC
        self.entry_addr_off = self.get_next_offset(self.bss_end_addr_off + 4)
        if self.entry_addr_off == None:
            raise Multiboot2HeaderFormatError()

    def get_next_offset(self, begining):
        offset = begining
        while offset < len(self.data):
            num = struct.unpack("<I", self.data[offset : offset + 4])[0]
            if num == self.ADDR_MAGIC:
                return offset
            offset = offset + 4
        return None

    def update(self):
        #header_addr
        packed = struct.pack("<I", self.header_addr);
        self.data = self.data[: self.header_addr_off] + packed \
                + self.data[self.header_addr_off + 4 :]

        #load_addr
        packed = struct.pack("<I", self.load_addr);
        self.data = self.data[: self.load_addr_off] + packed \
                + self.data[self.load_addr_off + 4 :]

        #load_end_addr
        packed = struct.pack("<I", self.load_end_addr);
        self.data = self.data[: self.load_end_addr_off] + packed \
                + self.data[self.load_end_addr_off + 4 :]

        #bss_end_addr
        packed = struct.pack("<I", self.bss_end_addr);
        self.data = self.data[: self.bss_end_addr_off] + packed \
                + self.data[self.bss_end_addr_off + 4 :]

        #entry_addr
        packed = struct.pack("<I", self.entry_addr);
        self.data = self.data[: self.entry_addr_off] + packed \
                + self.data[self.entry_addr_off + 4 :]

    def __str__(self):
        ret = "Multiboot2 Header info:\n" \
                + "Header address : 0x%.8X\n"%(self.header_addr) \
                + "Load address : 0x%.8X\n"%(self.load_addr) \
                + "Load end address : 0x%.8X\n"%(self.load_end_addr) \
                + "Bss end address : 0x%.8X\n"%(self.bss_end_addr) \
                + "Entry address : 0x%.8X\n"%(self.entry_addr)

        return ret

