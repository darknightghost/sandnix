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

class textbox(control.control):
	def __init__(self,frame,wnd,data):
		e = encoder()
		self.wnd = wnd
		self.title = e.convert(data[1])
		self.data = data
		self.frame = frame
		c = color_t()
		self.focused = False
		self.color = c.get_color(color_t.BLACK,color_t.WHITE) | curses.A_BOLD
		self.normal_color = c.get_color(color_t.BLACK,color_t.CYAN) | curses.A_BOLD
		self.focus_color = c.get_color(color_t.BLACK,color_t.YELLOW) | curses.A_BOLD
		self.edit_color = c.get_color(color_t.WHITE,color_t.RED) | curses.A_BOLD
		self.width = self.wnd.getmaxyx()[1] - 2 - 4
		return
	
	def draw(self,pos,begin,max):
		self.pos = pos
		self.max_len = self.width - len(self.title) - 3 - 4
		self.refresh()
		return 1
		
	def refresh(self):
		e = encoder()
		if self.focused:
			title_color = self.color | curses.A_REVERSE
			text_color = self.focus_color
		else:
			text_color = self.normal_color
			title_color = self.color

		self.wnd.addstr(self.pos.top,
			self.pos.left,"%s : "%(self.title),title_color)
		str = ""
		text = e.convert(self.data[2])
		if len(text) > self.max_len:
			str = text[0 : self.max_len]
		else:
			str = text + (" " * (self.max_len - len(text)))

		self.wnd.addstr(self.pos.top,
			self.pos.left + len(self.title) + 3,str,text_color)
		return
	
	def get_size(self):
		return rect_t(self.wnd.getmaxyx()[1] - 2,1)
	
	def on_get_focus(self):
		self.focused = True
		self.refresh()
		return
	
	def on_lost_focus(self):
		self.focused = False
		self.refresh()
		return
	
	def on_key_press(self,key):
		if key == ord('\n'):
			self.edit()
			self.refresh()
		return
		
	def refresh_edit(self):
		left = self.pos.left + len(self.title) + 3
		str = self.buffer[0][self.buffer[1] :]

		if len(str) > self.max_len:
			str = str[: self.max_len]
			
		i = 0
		for c in str:
			if i == self.buffer[2] - self.buffer[1]:
				self.wnd.addch(self.pos.top,left + i,c,self.edit_color)
				cur_left = left + i
			else:
				self.wnd.addch(self.pos.top,left + i,c,self.edit_color)
			i = i + 1

		if self.buffer[2] == -1:
			self.wnd.addch(self.pos.top,left + i,' ',self.edit_color)
			cur_left = left + i
			i = i + 1
			
		if i < self.max_len:
			self.wnd.addstr(self.pos.top,left + i,' ' * (self.max_len - i),self.edit_color)

		self.wnd.move(self.pos.top,cur_left)
		self.wnd.refresh()
		return
		
	def edit(self):
		e = encoder()
		
		#[buffer,begin,curser]
		self.buffer = [e.convert(self.data[2]),0,-1]
		if len(self.buffer[0]) >= self.max_len:
			self.buffer[1] = len(self.buffer[0]) - (self.max_len - 1)
		else:
			self.buffer[1] = 0
		curses.curs_set(1)
		self.refresh_edit()
		
		self.input_buf = utf8buf()

		while True:
			key = self.frame.scr.stdscr.getch()
			if key == 0x1B:
				break
			elif key == curses.KEY_LEFT:
				self.on_left()
			elif key == curses.KEY_RIGHT:
				self.on_right()
			elif key == curses.KEY_DC:
				self.on_delete()
			elif key == ord('\n'):
				self.update()
				break
			elif key == ord('\x7F'):
				self.on_backspace()
			else:
				self.on_input(key)

		curses.curs_set(0)
		self.refresh()
		self.wnd.refresh()
		return
			
	def on_left(self):
		if self.buffer[2] == 0:
			return
		if self.buffer[2] == -1:
			self.buffer[2] = len(self.buffer[0]) - 1
		else:
			self.buffer[2] = self.buffer[2] - 1
		if self.buffer[2] < self.buffer[1]:
			self.buffer[1] = self.buffer[2]
		self.refresh_edit()
	
	def on_right(self):
		if self.buffer[2] == -1:
			return
		
		self.buffer[2] = self.buffer[2] + 1
		if self.buffer[2] >= len(self.buffer[0]):
			self.buffer[2] = -1
		if self.buffer[2] > self.buffer[1] + self.max_len - 1:
			self.buffer[1] = self.buffer[2] - self.max_len + 1
		
		self.refresh_edit()
		
	def on_delete(self):
		c = self.buffer[2]
		if self.buffer[2] == -1:
			return
		self.buffer[0] = self.buffer[0][: c] + self.buffer[0][(c + 1) :]
		if self.buffer[2] >= len(self.buffer[0]):
			self.buffer[2] = -1

		self.refresh_edit()
		
	def on_backspace(self):
		c = self.buffer[2]
		if c == 0:
			return
		if c == -1:
			self.buffer[0] = self.buffer[0][: -1]
			if self.buffer[1] != 0:
				self.buffer[1] = self.buffer[1] - 1
		else:
			self.buffer[0] = self.buffer[0][: (c - 1)] + self.buffer[0][c :]
			self.buffer[2] = c - 1
			if self.buffer[1] != 0:
				self.buffer[1] = self.buffer[1] - 1
		self.refresh_edit()
		
	def on_input(self,key):
		if key > 255:
			return

		#s = self.input_buf.input(key)
		s = chr(key) + ""
		if s != None:
			c = self.buffer[2]
			if c == -1:
				self.buffer[0] = self.buffer[0] + s
				if len(self.buffer[0][self.buffer[1] :]) + 1> self.max_len:
					self.buffer[1] = self.buffer[1] + 1
			else:
				self.buffer[0] = self.buffer[0][: c] + s + self.buffer[0][c :]
				self.buffer[2] = c + 1
				if self.buffer[2] - self.buffer[1] + 1 >self.max_len:
					self.buffer[1] = self.buffer[1] + 1
			self.refresh_edit()

	def update(self):
		e = encoder()
		self.data[2] = e.unconvert(self.buffer[0])
