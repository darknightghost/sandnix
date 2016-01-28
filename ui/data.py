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

import curses
import locale

class utf8buf:
	def __init__(self):
		self.len = 0
		self.buf = []
	
	def input(self,c):
		if self.len == 0:
			self.len = self.get_len(c)
		self.buf.append(c)
		self.len = self.len - 1
		if self.len == 0:
			e = encoder()
			ret = e.encode(self.get_char())
			self.buf = []
			return ret
		return None
	
	def get_len(self,c):
		ret = 1
		if (c & 0xC0) == 0xC0:
			ret = ret + 1
			if (c & 0xE0) == 0xE0:
				ret = ret + 1
				if (c & 0xF0) == 0xF0:
					ret = ret + 1
					if (c & 0xF8) == 0xF8:
						ret = ret + 1
						if (c & 0xFC) == 0xFC:
							ret = ret + 1
		return ret
		
	def get_char(self):
		c = ''
		for t in self.buf:
			c + chr(t)
		return c.decode("utf-8");

class encoder:
	def __init__(self):
		self.code = locale.getpreferredencoding()
		return
		
	def convert(self,str):
		return str.decode('utf-8','ignore').encode(self.code)
		
	def encode(self,str):
		return str.encode(self.code)
		
	def unconvert(self,str):
		return str.decode(self.code,'ignore').encode('utf-8')

class color_t:
	BLACK = curses.COLOR_BLACK
	BLUE = curses.COLOR_BLUE
	GREEN = curses.COLOR_GREEN
	CYAN = curses.COLOR_CYAN
	RED = curses.COLOR_RED
	MAGENTA = curses.COLOR_MAGENTA
	YELLOW = curses.COLOR_YELLOW
	WHITE = curses.COLOR_WHITE
	
	def init_color(self):
		curses.start_color()
		for i in range(7 + 1):
			for j in range(7 + 1):
				if (7 - i) * 8 + j != 0:
					curses.init_pair((7 - i) * 8 + j,i,j)
		return
		
	def get_color(self,fg,bg):
		return curses.color_pair((7 - fg) * 8 + bg)
	
class pos_t:
	def __init__(self):
		self.top = 0
		self.left = 0
		
	def __init__(self,top,left):
		self.top = top
		self.left = left
	
class rect_t:
	def __init__(self):
		self.width = 0
		self.height = 0
		
	def __init__(self,width,height):
		self.width = width
		self.height = height
	
class region_t:
	def __init__(self):
		self.pos = pos_t()
		self.rect = rect_t()
	
	def __init__(self,top,left,width,height):
		self.pos = pos_t(top,left)
		self.rect = rect_t(width,height)
