#! /usr/bin/env python2
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
import os
import struct

class UnknowKernel(Exception):
	def __init(self,path):
		self.path = path
	def __str__(self):
		return "Illegal format of kernel file:\"%s\"\n"%(self.path)
		
class UnloadableSection(Exception):
	pass

class program_header:
	#Type
	PT_NULL = 0
	PT_LOAD = 1
	PT_DYNAMIC = 2
	PT_INTERP = 3
	PT_NOTE = 4
	PT_SHLIB = 5
	PT_PHDR = 6
	PT_TLS = 7
	PT_LOOS = 0x60000000
	PT_HIOS = 0x6fffffff
	PT_LOPROC = 0x70000000
	PT_HIPROC = 0x7fffffff
	PT_GNU_EH_FRAME = 0x6474e550
	
	#Offsets
	TYPE_OFF = 0
	OFFSET_OFF32 = TYPE_OFF + 4
	OFFSET_OFF64 = TYPE_OFF + 4 + 4
	VADDR_OFF32 = OFFSET_OFF32 + 4
	VADDR_OFF64 = OFFSET_OFF64 + 8
	FILESZ_OFF32 = VADDR_OFF32 + 4 + 4
	FILESZ_OFF64 = VADDR_OFF64 + 8 + 8
	MEMSZ_OFF32 = FILESZ_OFF32 + 4
	MEMSZ_OFF64 = FILESZ_OFF64 + 8
	def	__init__(self,data,file,header_type,machine):
		oldpos = file.tell()

		self.type = struct.unpack("<I",data[self.TYPE_OFF : self.TYPE_OFF + 4])[0]
		if self.type != self.PT_LOAD:
			raise UnloadableSection()
		
		if header_type == elf.ELFCLASS32:
			self.off = struct.unpack("<I",data[self.OFFSET_OFF32 : self.OFFSET_OFF32 + 4])[0]
			self.vaddr = struct.unpack("<I",data[self.VADDR_OFF32 :self.VADDR_OFF32 + 4])[0]
			self.filesz = struct.unpack("<I",data[self.FILESZ_OFF32 : self.FILESZ_OFF32 + 4])[0]
			self.memsz = struct.unpack("<I",data[self.MEMSZ_OFF32 : self.MEMSZ_OFF32 + 4])[0]
		elif header_type == elf.ELFCLASS64:
			self.off = struct.unpack("<I",data[self.OFFSET_OFF64 : self.OFFSET_OFF64 + 8])[0]
			self.vaddr = struct.unpack("<I",data[self.VADDR_OFF64 :self.VADDR_OFF64 + 8])[0]
			self.filesz = struct.unpack("<I",data[self.FILESZ_OFF64 : self.FILESZ_OFF64 + 8])[0]
			self.memsz = struct.unpack("<I",data[self.MEMSZ_OFF64 : self.MEMSZ_OFF64 + 8])[0]
			
		print("\nSegment:")
		print("Offset : 0x%0.8x."%(self.off))
		print("Virtual address : 0x%0.8x."%(self.vaddr))
		print("Size in file : 0x%0.8x."%(self.filesz))
		print("Size in memory : 0x%0.8x."%(self.memsz))
		
		#read data
		if self.filesz > 0:
			file.seek(self.off,0)
			self.data = file.read(self.filesz)
		else:
			self.data = b''
		file.seek(oldpos,0)
		
	def get_vaddr(self):
		return self.vaddr
		
	def get_memsz(self):
		return self.memsz
		
	def get_filesz(self):
		return self.filesz
		
	def get_data(self):
		return self.data

class elf:
	EI_NIDENT = 16
	ELFCLASS32 = 1
	ELFCLASS64 = 2
	MACHINE_OFF = EI_NIDENT + 2
	ENTERY_OFF = EI_NIDENT + 2 + 2 + 4
	PH_OFF32 = ENTERY_OFF + 4
	PH_OFF64 = ENTERY_OFF + 8
	PER_PH_SZ_OFF32 = PH_OFF32 + 4 + 4 + 4 + 2
	PER_PH_SZ_OFF64 = PH_OFF64 + 8 + 8 + 4 + 2
	PH_NUM_OFF32 = PER_PH_SZ_OFF32 + 2
	PH_NUM_OFF64 = PER_PH_SZ_OFF64 + 2
	def __init__(self,path):
		self.file = open(path,"rb")
		print("Kernel file \"%s\" opened."%(path))
		
		#Indent
		self.type = self.check_indent(path)
		
		if self.type == self.ELFCLASS32:
			print("Kernel file type : elf32.")
		elif self.type == self.ELFCLASS64:
			print("Kernel file type : elf64.")
			
		#Machine
		self.file.seek(self.MACHINE_OFF,0)
		self.entry = struct.unpack("<I",self.file.read(4))
			
		#Program header info
		self.get_program_header_info()
		print("Entery : 0x%0.8x."%(self.entry))
		
		#Program headers
		self.program_headers = []
		self.file.seek(self.ph_off,0)
		for i in range(0,self.ph_num):
			data = self.file.read(self.per_ph_size)
			try:
				self.program_headers.append(program_header(data,self.file,self.type,None))
			except UnloadableSection:
				pass
		
		return

	def __del__(self):
		self.file.close()
		
	def	get_entry(self):
		return self.entry
		
	def get_program_headers(self):
		return self.program_headers
		
	def check_indent(self,path):
		#Check magic
		data = self.file.read(4)

		if struct.unpack("b",data[0])[0] != 0x7F or struct.unpack("b",data[1])[0] != 0x45 or struct.unpack("b",data[2])[0] != 0x4C or struct.unpack("b",data[3])[0] != 0x46:
			print("Unknow elf magic,kernel file has been broken.")
			raise UnknowKernel(path)
		
		#Get class
		data = self.file.read(1)
		self.file.seek(self.EI_NIDENT,0)
		if struct.unpack("b",data[0])[0] in (self.ELFCLASS32,self.ELFCLASS64):
			return struct.unpack("b",data[0])[0]
		else:
			print("Unknow elf class.")
			raise UnknowKernel(path)
			
	def get_program_header_info(self):
		if self.type == self.ELFCLASS32:
			#Entery
			self.file.seek(self.ENTERY_OFF,0)
			self.entry = struct.unpack("<I",self.file.read(4))[0]
			#Program headers offset
			self.file.seek(self.PH_OFF32,0)
			self.ph_off = struct.unpack("<I",self.file.read(4))[0]
			#Program header size
			self.file.seek(self.PER_PH_SZ_OFF32,0)
			self.per_ph_size = struct.unpack("<H",self.file.read(2))[0]
			#Program header num
			self.file.seek(self.PH_NUM_OFF32,0)
			self.ph_num = struct.unpack("<H",self.file.read(2))[0]
		if self.type == self.ELFCLASS64:
			#Entery
			self.file.seek(self.ENTERY_OFF,0)
			self.entry = struct.unpack("<I",self.file.read(8))[0]
			#Program headers offset
			self.file.seek(self.PH_OFF64,0)
			self.ph_off = struct.unpack("<I",self.file.read(8))[0]
			#Program header size
			self.file.seek(self.PER_PH_SZ_OFF64,0)
			self.per_ph_size = struct.unpack("<H",self.file.read(2))[0]
			#Program header num
			self.file.seek(self.PH_NUM_OFF64,0)
			self.ph_num = struct.unpack("<H",self.file.read(2))[0]
