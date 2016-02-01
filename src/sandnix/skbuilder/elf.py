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

class program_header:
	def	__init__(self,data,file):
		pass

class elf:
	EI_NIDENT = 16
	ELFCLASS32 = 1
	ELFCLASS64 = 2
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
			
		#Program header info
		self.get_program_header_info()
		
		#Program headers
		self.program_headers = []
		self.file.seek(self.ph_off,0)
		for i in range(0,self.ph_num):
			data = self.file.read(self.per_ph_size)
			self.program_headers.append(program_header(data,self.file))
		
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
		if data[0] != 0x7F or data[1] != 0x45 or data[2] != 0x4C or data[3] != 0x46:
			print("Unknow elf magic,kernel file has been broken.")
			raise UnknowKernel(path)
		
		#Get class
		data = self.file.read(1)
		self.file.seek(self.EI_NIDENT,0)
		if data[0] in (self.ELFCLASS32,self.ELFCLASS64):
			return data[0]
		else:
			print("Unknow elf class.")
			raise UnknowKernel(path)
			
	def get_program_header_info(self):
		if self.type == self.ELFCLASS32:
			#Entery
			self.file.seek(self.ENTERY_OFF,0)
			self.entry = struct.unpack("<I",self.file.read(4))
			#Program headers offset
			self.file.seek(self.PH_OFF32,0)
			self.ph_off = struct.unpack("<I",self.file.read(4))
			#Program header size
			self.file.seek(self.PER_PH_SZ_OFF32,0)
			self.per_ph_size = struct.unpack("<I",self.file.read(4))
			#Program header num
			self.file.seek(self.PH_NUM_OFF32,0)
			self.ph_num = struct.unpack("<I",self.file.read(4))
		if self.type == self.ELFCLASS64:
			#Entery
			self.file.seek(self.ENTERY_OFF,0)
			self.entry = struct.unpack("<I",self.file.read(8))
			#Program headers offset
			self.file.seek(self.PH_OFF64,0)
			self.ph_off = struct.unpack("<I",self.file.read(8))
			#Program header size
			self.file.seek(self.PER_PH_SZ_OFF64,0)
			self.per_ph_size = struct.unpack("<I",self.file.read(8))
			#Program header num
			self.file.seek(self.PH_NUM_OFF64,0)
			self.ph_num = struct.unpack("<I",self.file.read(8))
