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

class UnknowHeader(Exception):
	def __init(self,path):
		self.path = path
	def __str__(self):
		return "Illegal format of header file:\"%s\"\n"%(self.path)

class multiboot_header:
	ADDR_MAGIC = 0xFFFFFFFF
	def __init__(self,path):
		self.file = open(path,"rb")
		print("Header file \"%s\" opened."%(path))
		self.header_addr_off = self.search_magic()
		print("Header address offset : 0x%0.8X."%(self.header_addr_off))
		self.load_addr_off = self.search_magic()
		print("Load address offset : 0x%0.8X."%(self.load_addr_off))
		self.load_end_addr_off = self.search_magic()
		print("Load end address offset : 0x%0.8X."%(self.load_end_addr_off))
		self.bss_end_addr_off = self.search_magic()
		print("Bss end address offset : 0x%0.8X."%(self.bss_end_addr_off))
		self.entry_addr_off = self.search_magic()
		print("Entery address offset : 0x%0.8X."%(self.entry_addr_off))
		print("")
		
		if None in (self.header_addr_off,self.load_addr_off,self.load_end_addr_off,
			self.bss_end_addr_off,self.entry_addr_off):
			raise UnknowHeader(path)
		return
		
	
	def __del__(self):
		self.file.close()
		return
		
	def set_header_addr(self,num):
		self.header_addr = num
		return
		
	def set_load_addr(self,num):
		self.load_addr = num
		return
	
	def set_load_end_addr(self,num):
		self.load_end_addr = num
		return
		
	def set_bss_end_addr(self,num):
		self.bss_end_addr = num;
		return
		
	def set_entry_addr(self,num):
		self.entry_addr = num
		return
		
	def write(self,file):
		#Write file header
		self.file.seek(0,0)
		data = self.file.read()
		file.seek(0,0)
		file.write(data)
		
		#Write address
		for t in ((self.header_addr_off,self.header_addr),
			(self.load_addr_off,self.load_addr),
			(self.load_end_addr_off,self.load_end_addr),
			(self.bss_end_addr_off,self.bss_end_addr),
			(self.entry_addr_off,self.entry_addr)):
			file.seek(t[0],0)
			file.write(struct.pack("<I",t[1]))
		return
		
	def search_magic(self):
		while (1):
			data = self.file.read(4)
			if len(data) < 4:
				return None
			num = struct.unpack("<I",data)[0]
			if num == self.ADDR_MAGIC:
				return self.file.tell() - 4
			else:
				self.file.seek(-3,1)
