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

import curses
from ui.controls.control import *
from ui.ui import *

class checkbox(control):
	def __init__(self,frame,wnd,data):
		self.wnd = wnd
		self.data = data
		c = color_t()
		self.focus = False
		self.color = c.get_color(color_t.BLACK,color_t.WHITE) | curses.A_BOLD
	
	def draw(self,pos,begin,max):
		self.pos = pos
		self.refresh()
		return 1
		
	def refresh(self):
		if self.focus:
			color = self.color | curses.A_REVERSE
		else:
			color = self.color
		if self.data[2] == True:
			self.wnd.addstr(self.pos.top, self.pos.left,
				"[√] %s"%(self.data[1]),color)
		else:
			self.wnd.addstr(self.pos.top, self.pos.left,"[ ] %s"%(self.data[1]),color)
		self.wnd.refresh()
		return
		
	def get_size(self):
		return rect_t(len(self.data[1]) + 4,1)
	
	def on_get_focus(self):
		self.focus = True;
		self.refresh()
		return

	def on_lost_focus(self):
		self.focus = False;
		self.refresh()
		return
	
	def on_key_press(self,key):
		if key == ord('\n'):
			self.data[2] = not self.data[2]
			self.refresh()
		return
