#! /usr/bin/env python
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
import control
from ui.ui import *

class lable(control.control):
	def __init__(self,frame,wnd,data):
		e = encoder()
		self.wnd = wnd
		self.text = e.convert(data[1])
		c = color_t()
		self.color = c.get_color(color_t.BLACK,color_t.WHITE) | curses.A_BOLD
	
	def draw(self,pos,begin,max):
		self.pos = pos
		self.wnd.addstr(self.pos.top, self.pos.left,self.text,self.color)
		return 1
	
	def get_size(self):
		return rect_t(len(self.text) + 2,1)
