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
import sys
from elf import elf
from kernel_header import kernel_header
import uboot

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
    if arg_dict["type"] == "kernel":
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
    #Analyse elf file
    kernel = elf.elf(arg_dict["input"])

    #Fill kernel header
    kheader = kernel_header.kernel_header(kernel);
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

    #Create kernel image
    for ph in kernel.program_headers:
        if ph.flags & elf.program_header.PF_X:
            code_data = ph.data
        else:
            data_data = ph.data

    vaddr_base = min(kheader.code_start, kheader.data_start)
    vaddr_size = max(kheader.code_start + kheader.code_size, \
            kheader.data_start + kheader.data_size) - vaddr_base;
    kernel_img = b'\x00' * vaddr_size
    kernel_img = kernel_img[: kheader.code_start - vaddr_base] \
            + code_data + kernel_img[kheader.code_start - vaddr_base + \
            len(code_data) :]
    kernel_img = kernel_img[: kheader.data_start - vaddr_base] \
            + data_data + kernel_img[kheader.data_start - vaddr_base + \
            len(data_data) :]

    #Create uboot header
    uboot_header = uboot.uboot(arg_dict["header"], "arm")
    uboot_header.set_data_address(arg_dict["load_address"])
    uboot_header.set_data_size(len(kernel_img))
    uboot_header.set_type(uboot.uboot.IH_TYPE_KERNEL)
    uboot_header.set_entry_point(kernel.entry - vaddr_base + \
            arg_dict["load_address"])

    fout = open(arg_dict["output"], "wb")

    #Write image file
    uboot_header.save(kernel_img, fout);

    print(str(uboot_header))
    fout.close()

    return 0

def mk_initrd(arg_dict):
    #Load image
    f = open(arg_dict["input"], "rb")
    img = f.read()

    #Create uboot header
    uboot_header = uboot,uboot(arg_dict["header"], "arm")
    uboot_header.set_data_address(arg_dict["load_address"])
    uboot_header.set_data_size(len(img))
    uboot_header.set_type(uboot.uboot.IH_TYPE_RAMDISK)
    uboot_header.set_entry_point(arg_dict["load_address"])

    fout = open(arg_dict["output"], "wr")

    #Write image file
    uboot_header.save(kernel_img, fout);
    fout.close()

    return 0

def usage():
    print("Usage:")
    print("\timage_builder.py -t type -h header -i input -o output -p load_addr")
    print("TYPES:\n\tkernel\n\tinitrd")
    return

ret = main(sys.argv)
exit(ret)
