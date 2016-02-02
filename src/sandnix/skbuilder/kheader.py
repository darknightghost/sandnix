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

class IllegaleKernelHeader(Exception):
	pass

class kheader:
	LE = 0
	BE = 1
	MAGIC = 0x444E4153
	MAGIC_OFF = 0
	TEXT_BEGIN_OFF = MAGIC_OFF + 4
	TEXT_END_OFF = TEXT_BEGIN_OFF + 4
	DATA_BEGIN_OFF = TEXT_END_OFF + 4
	DATA_END_OFF = DATA_BEGIN_OFF + 4
	CHECKSUM_OFF = DATA_END_OFF + 4
	def __init__(self,data,end_type = 0):
		self.data = data
		self.end_type = end_type
		self.off = 0
		
		try:
			for i in range(0,64):
				self.off = i
				if struct.unpack("<I",data[self.MAGIC_OFF + self.off: self.MAGIC_OFF + self.off + 4])[0] == self.MAGIC:
					break
		except:
			print("Illegal kernel header.")
			raise IllegaleKernelHeader()
			
		if struct.unpack("<I",data[self.MAGIC_OFF + self.off: self.MAGIC_OFF + self.off + 4])[0] != self.MAGIC:
			print("Illegal kernel header.")
			raise IllegaleKernelHeader()
			
		self.text_begin = 0
		self.text_end = 0
		self.data_begin = 0
		self.data_end = 0
		
	def set_text_begin(self,value):
		self.text_begin = value
		self.set_value(value,self.TEXT_BEGIN_OFF + self.off)
	
	def set_text_end(self,value):
		self.text_end = value
		self.set_value(value,self.TEXT_END_OFF + self.off)
	
	def set_data_begin(self,value):
		self.data_begin = value
		self.set_value(value,self.DATA_BEGIN_OFF + self.off)
	
	def set_data_end(self,value):
		self.data_end = value
		self.set_value(value,self.DATA_END_OFF + self.off)

	def get_data(self):
		self.set_checksum()
		return self.data

	def set_value(self,value,off):
		if self.end_type == self.LE:
			self.data = self.data[0 : off] + struct.pack("<I",value) + self.data[off + 4 : ]
		elif self.end_type == self.BE:
			self.data = self.data[0 : off] + struct.pack(">I",value) + self.data[off + 4 : ]
		return
		
	def set_checksum(self):
		if self.end_type == self.LE:
			magic = self.MAGIC
		else:
			magic = struct.unpack(">I",struct.pack("<I",value))[0]
			
		checksum = magic + self.text_begin + self.text_end + self.data_begin + self.data_end
		checksum = checksum - int(checksum / 0x100000000) * 0x100000000
		checksum = 0x100000000 - checksum
		self.set_value(checksum,self.CHECKSUM_OFF + self.off)
