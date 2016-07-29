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
import time
import binascii

class UnsupportedArch(Exception):
    def __init__(self, arch_name):
        self,arch = arch_name

    def __str__(self):
        return "Unsupported architecture : \"%s\".\n"%(self.arch)

class uboot:
    #Offsets
    HCRC_OFF = 4
    TIME_OFF = HCRC_OFF + 4
    SIZE_OFF = TIME_OFF + 4
    LOAD_OFF = SIZE_OFF + 4
    EP_OFF = LOAD_OFF + 4
    DCRC_OFF  = EP_OFF + 4
    ARCH_OFF = EP_OFF + 5
    TYPE_OFF = ARCH_OFF + 1

    #Architectures
    LE_ARCH = {"x86" : 3, "amd64" : 3, "arm" : 2}
    BE_ARCH = {}

    #Types
    IH_TYPE_KERNEL = 2
    IH_TYPE_RAMDISK = 3
    def __init__(self, path, arch):
        f = open(path, "rb")
        self.data = f.read()
        f.close();

        if not (arch in LE_ARCH.keys() or arch in BE_ARCH.keys()):
            raise UnsupportedArch(arch)

        self.arch = arch

        return

    def set_data_address(self, addr):
        self.write_32(uboot.LOAD_OFF, addr)
        return

    def set_data_size(self, addr):
        self.write_32(uboot.SIZE_OFF, addr)
        return

    def set_type(self, header_type):
        self.write_8(uboot.TYPE_OFF, header_type)
        return

    def set_entry_point(self, addr):
        self.write_32(uboot.EP_OFF, addr)
        return

    def write_32(self, off, num):
        if self.arch in LE_ARCH:
            packed = struct.pack("<I", num)
        else:
            packed = struct.pack(">I", num)
        self.data = self.data[: off] + packed + self.data[off + 4 :]
        return
        
    def write_8(self, off, num):
        if self.arch in LE_ARCH:
            packed = struct.pack("<B", num)
        else:
            packed = struct.pack(">B", num)
        self.data = self.data[: off] + packed + self.data[off + 1 :]
        return

    def save(self, image_data, f):
        #Fill uimage header
        write_32(uboot.TIME_OFF, int(time.time()))

        if self.arch in uboot.LE_ARCH:
            write_8(uboot.ARCH_OFF, uboot.LE_ARCH[self.arch])
        else:
            write_8(uboot.ARCH_OFF, uboot.BE_ARCH[self.arch])

        #Data crc32
        write_32(uboot.DCRC_OFF, binascii.crc32(image_data))

        #Header crc32
        write_32(uboot.HCRC_OFF, 0)
        write_32(uboot.HCRC_OFF, binascii.crc32(self.data))

        f.write(self.data)
        f.write(self.image_data)
        return
