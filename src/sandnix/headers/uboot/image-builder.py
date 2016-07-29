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

'''
    TODO:
        1. Create memory image.
        2. Fill kernel header.
        3. Create uboot header.
        4. Write image.
'''

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
        elif argv[i] == "-i":
            arg_dict["input"] = argv[i + 1]
            i = i + 1
        elif argv[i] == "-o":
            arg_dict["output"] = argv[i + 1]
            i = i + 1
        elif argv[i] == "-t":
            arg_dict["type"] = argv[i + 1]
            i = i + 1
        elif argv[i] == "-p":
            try:
                arg_dict["load_address"] = int(argv[i + 1], 16)
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
            or (not ("input" in  arg_dict.keys())) \
            or (not ("output" in arg_dict.keys())) \
            or (not ("type" in arg_dict.keys())) \
            or (not ("load_address" in arg_dict.keys())):
                usage()
                return -1;
    if not arg_dict["type"] in ("kernel", "initrd"):
        print("Unknow image type \"%s\"."%(arg_dict["type"]))
        usage()
        return -1

    print("\n-----------------------------------------------------------------")
    if arg_dict["tyoe"] == "kernel":
        print("Header file : \"%s\""%(arg_dict["header"]))
        print("Kernel file : \"%s\""%(arg_dict["input"]))
        print("Output file : \"%s\""%(arg_dict["output"]))
        print("Load address : \"0x%.16X\""%(arg_dict["load_address"]))
        ret = mk_kernel(arg_dict);
    else:
        print("Header file : \"%s\""%(arg_dict["header"]))
        print("Ramdisk file : \"%s\""%(arg_dict["input"]))
        print("Output file : \"%s\""%(arg_dict["output"]))
        print("Load address : \"0x%.16X\""%(arg_dict["load_address"]))
        ret = mk_initrd(argv_dict);
    
    return ret

def mk_kernel(arg_dict):
    return 0

def mk_initrd(arg_dict):
    return 0

def usage():
    print("Usage:")
    print("\timage_builder.py -t type -h header -i input -o output -p load_addr")
    print("TYPES:\n\tkernel\n\tinitrd")
    return

ret = main(sys.argv)
exit(ret)
