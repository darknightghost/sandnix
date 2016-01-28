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

class listcontrol(control.control):
	def __init__(self,frame,wnd,data):
		self.wnd = wnd
		self.data = data
		self.frame = frame
		c = color_t()
		self.color = c.get_color(color_t.BLACK,color_t.WHITE) | curses.A_BOLD
		self.selections = data[2]
		self.focus = False
		self.color_selection = c.get_color(color_t.WHITE,color_t.RED) | curses.A_BOLD
		self.color_selected = c.get_color(color_t.YELLOW,color_t.RED) | curses.A_BOLD
		self.shadow_color = c.get_color(color_t.BLACK,color_t.BLACK)

	def draw(self,pos,begin,max):
		self.pos = pos
		e = encoder()
		self.wnd.addstr(self.pos.top,
			self.pos.left,e.convert("%s : [%s]"%(self.data[1],self.selections[0][self.selections[1]])),self.color)
		return 1
	
	def get_size(self):
		return rect_t(len(self.data[1]) + len(self.selections[0][self.selections[1]]) + 5,1)
		
	def refresh(self):
		color = self.color
		if self.focus:
			color = color | curses.A_REVERSE
		e = encoder()
		self.wnd.addstr(self.pos.top,
			self.pos.left,e.convert("%s : [%s]"%(self.data[1],self.selections[0][self.selections[1]])),color)
		self.wnd.refresh()
		return

	def on_get_focus(self):
		self.focus = True
		self.refresh()
		return
	
	def on_lost_focus(self):
		self.focus = False
		self.refresh()
		return
	
	def on_key_press(self,key):
		if key == ord('\n'):
			self.get_selection()
		return
		
	def get_selection(self):
		self.frame.refresh = True
		self.client_rect = rect_t(self.wnd.getmaxyx()[1] - 4,self.wnd.getmaxyx()[0] - 4)
		self.client_pos = pos_t(2,2)
		
		#Draw menu window
		self.draw_shadow()
		self.draw_wnd()
		
		self.focused_item = self.selections[1]

		#Compute begining index
		if len(self.selections[0]) < self.max_item:
			self.begin = 0
		elif min(len(self.selections[0]) - self.selections[1],
			self.selections[1]) <= self.max_item:
			if len(self.selections[0]) - self.selections[1] <= self.selections[1]:
				self.begin = self.selections[1] - (self.max_item - (len(self.selections[0]) - self.selections[1]))
			else:
				self.begin = 0
		else:
			self.begin = self.selections[1] - self.max_item / 2

		self.retflag = False
		while not self.retflag:
			self.draw_menu()
			key = self.frame.scr.stdscr.getch()
			if key == 0x1B:
				self.retflag = True
			elif key == curses.KEY_UP:
				self.on_up()
			elif key == curses.KEY_DOWN:
				self.on_down()
			elif key == ord('\n'):
				self.on_enter()

		del self.client_wnd
		del self.selection_wnd
	
	def draw_shadow(self):
		e = encoder()
		for top in range(self.client_pos.top + 1,
			self.client_pos.top + self.client_rect.height):
			self.wnd.addstr(top,self.client_pos.left + self.client_rect.width,
				e.convert('█'),self.shadow_color)
		for left in range(self.client_pos.left + 1,
			self.client_pos.left + self.client_rect.width + 1):
			self.wnd.addstr(self.client_pos.top + self.client_rect.height,left,
				e.convert('█'),self.shadow_color)
		self.wnd.refresh()
		
	def draw_wnd(self):
		e = encoder()
		
		#Draw client window
		self.client_wnd = self.wnd.derwin(self.client_rect.height,self.client_rect.width,
			self.client_pos.top,self.client_pos.left)
		self.client_wnd.bkgd(' ',self.color_selection)
		self.client_wnd.erase()
		title = e.convert(self.data[1])
		self.client_wnd.addstr(0,self.client_rect.width / 2 - len(title) / 2,
				title,self.color_selection)
		self.selection_wnd = self.client_wnd.derwin(self.client_rect.height - 2,
			self.client_rect.width - 3,1,1)
		
		#Draw selection window
		self.selection_wnd.bkgd(' ',self.color_selection)
		self.selection_wnd.box()
		self.max_item = self.selection_wnd.getmaxyx()[0] - 2
		self.client_wnd.refresh()
		self.selection_wnd.refresh()

	def draw_scoll_bar(self,pos):
		if pos < 0 or pos > 1:
			raise ValueError("pos = %f.pos must between 0 and 1!"%(pos))

		left = self.client_rect.width - 2
		length = self.client_rect.height - 4
		for top in range(2,2 + length):
			if top == 2 + int((length - 1) * pos):
				self.client_wnd.addch(top,left,' ',self.color_selection | curses.A_REVERSE)
			else:
				self.client_wnd.addch(top,left,'|',self.color_selection)
		self.client_wnd.refresh()

	def draw_menu(self):
		i = 0
		e = encoder()
		self.selection_wnd.erase()
		self.selection_wnd.box()
		while i < self.max_item and i < len(self.selections[0]):
			if i + self.begin == self.selections[1]:
				if i + self.begin == self.focused_item:
					self.selection_wnd.addstr(1 + i,1,
						e.convert(self.selections[0][i + self.begin]),self.color_selected | curses.A_REVERSE)
				else:
					self.selection_wnd.addstr(1 + i,1,
						e.convert(self.selections[0][i + self.begin]),self.color_selected)
			else:
				if i + self.begin == self.focused_item:
					self.selection_wnd.addstr(1 + i,1,
						e.convert(self.selections[0][i + self.begin]),self.color_selection | curses.A_REVERSE)
				else:
					self.selection_wnd.addstr(1 + i,1,
						e.convert(self.selections[0][i + self.begin]),self.color_selection)
			i = i + 1
		
		self.selection_wnd.refresh()
		self.draw_scoll_bar(self.begin * 1.0 / (len(self.selections[0]) - self.max_item))
		
		return
		
	def on_up(self):
		if self.focused_item > 0:
			self.focused_item = self.focused_item - 1
			if self.focused_item < self.begin:
				self.begin = self.focused_item
		
	def on_down(self):
		if self.focused_item < len(self.selections[0]) - 1:
			self.focused_item = self.focused_item + 1
			if self.focused_item > self.begin + self.max_item - 1:
				self.begin = self.begin + 1
		
	def on_enter(self):
		self.selections[1] = self.focused_item
		self.retflag = True
