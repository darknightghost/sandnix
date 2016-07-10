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

import os
import sys
from elf import elf
from kernel_header import kernel_header

def main(argv):
    #Analyse args
    arg_dict = {}
    i = 1
    while i < len(argv):
        if argv[i] == "-h":
            arg_dict["header"] = argv[i + 1]
            i = i + 1
        elif argv[i] == "-k":
            arg_dict["kernel"] = argv[i + 1]
            i = i + 1
        elif argv[i] == "-o":
            arg_dict["output"] = argv[i + 1]
            i = i + 1
        elif argv[i] == "-v":
            try:
                arg_dict["kernel_base"] = int(argv[i + 1], 16)
            except ValueError:
                print("Unknow address %s."%(argv[i + 1]))
                usage()
                return -1
            i = i + 1
        else:
            print("Unknow argument \"%s\"."%(argv[i]))
            usage()
            return -1
        i = i + 1

    if (not ("header" in arg_dict.keys())) \
            or (not ("kernel" in  arg_dict.keys())) \
            or (not ("output" in arg_dict.keys())) \
            or (not ("kernel_base" in arg_dict.keys())):
                usage()
                return -1;

    print("\n-----------------------------------------------------------------")
    print("Header file : \"%s\""%(arg_dict["header"]))
    print("Kernel file : \"%s\""%(arg_dict["kernel"]))
    print("Output file : \"%s\""%(arg_dict["output"]))
    print("Kernel mem address : \"0x%.16X\""%(arg_dict["kernel_base"]))

    #Load kernel
    kernel = elf.elf(arg_dict["kernel"])
    print(str(kernel))

    kheader = kernel_header.kernel_header(kernel);

    #Fill kernel header
    if len(kernel.program_headers) > 2:
        print("Too many segments in kernel image.\n")
        return -1
    for ph in kernel.program_headers:
        if ph.flags & elf.program_header.PF_X:
            kheader.code_start = ph.vaddr
            kheader.code_size = ph.memsz
        else:
            kheader.data_start = ph.vaddr
            kheader.data_size = ph.memsz

    kheader.update()
    print(str(kheader))
    #Write kernel image
    return 0;

def usage():
    print("Usage:")
    print("\timage_builder.py -h header -k kernel -o output -v kernel-mem-base-address")
    return

ret = main(sys.argv)
exit(ret)
