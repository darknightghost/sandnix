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

from data import *
from controls.control import *
from controls.button import *
from controls.lable import *
from controls.checkbox import *
from controls.listcontrol import *
from controls.textbox import *

class ControlTypeError(Exception):
	def __init__(self,ctrl_type):
		self.ctrl_type = ctrl_type
		self.msg = ""
		return

	def __str__(self):
		return "Unknow type of control: %s.%s"%(self.ctrl_type,self.msg)

class submenu(control.control):
	def __init__(self,frame,wnd,data):
		e = encoder()
		self.wnd = wnd
		self.title = e.convert(data[1])
		self.data = data
		self.frame = frame
		c = color_t()
		self.focused = False
		self.color = c.get_color(color_t.BLACK,color_t.WHITE) | curses.A_BOLD
		self.width = self.wnd.getmaxyx()[1] - 2 - 4
		return
	
	def draw(self,pos,begin,max):
		self.pos = pos
		self.blank_len = self.width - len(self.title) - 3
		self.refresh()
		return 1
		
	def refresh(self):
		e = encoder()
		if self.focused:
			color = self.color | curses.A_REVERSE
		else:
			color = self.color

		self.wnd.addstr(self.pos.top,
			self.pos.left,self.title,color)
		
		self.wnd.addstr(self.pos.top,
			self.pos.left + len(self.title)," " * self.blank_len,color)
		self.wnd.addstr(self.pos.top,
			self.pos.left + len(self.title) + self.blank_len,"-->",color)
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
			e = encoder()
			m = menu(self.frame.scr,e.unconvert(self.title))
			m.show_menu(self.data[2])
			self.frame.refresh = True
		return

class menu:
	FOCUS_BACK = -1
	FOCUS_PREV = -2
	FOCUS_NEXT = -3
	#{control-name:(class,indent,able-get-focus)}
	control_dict = {"lable" : (lable,0,False),
		"checkbox" : (checkbox,4,True),
		"listcontrol" : (listcontrol,4,True),
		"textbox" : (textbox,4,True),
		"submenu" : (submenu,4,True)}
	def __init__(self,scr,title):
		e = encoder()
		self.scr = scr
		self.back = False
		self.title = e.convert(title)

	def show_menu(self,menu):
		#Draw window
		self.draw_wnd()

		#Draw menu
		self.page = self.analyse_menu(menu)
		self.index = 0
		
		self.btn_back = button(self.wnd,("Back",self.on_back))
		self.btn_prev = button(self.wnd,("Prev",self.on_prev))
		self.btn_next = button(self.wnd,("Next",self.on_next))
		self.focus = 0

		while not self.back:
			#Draw title
			color = color_t()
			self.wnd.addstr(0,0," " * self.rect.width,color.get_color(0,color_t.WHITE) | curses.A_BOLD)
			self.wnd.addstr(0, self.rect.width / 2 - len(self.title) / 2,self.title,color.get_color(0,color_t.WHITE) | curses.A_BOLD)
			self.wnd.refresh()

			#Clear menu
			self.client.erase()
			self.client.box()


			#Draw buttons
			for left in range(1,self.rect.width - 1):
				self.wnd.addch(self.rect.height - 2,left,' ')

			self.btn_back.draw(pos_t(self.rect.height - 2,self.rect.width / 4 - 2),0,1)

			if menu != []:
				if self.index > 0:
					self.btn_prev.draw(pos_t(self.rect.height - 2,self.rect.width / 4 * 2 - 2),0,1)
			
				if self.index + 1 < len(self.page):
					self.btn_next.draw(pos_t(self.rect.height - 2,self.rect.width / 4 * 3 - 2),0,1)

			#Draw menu
			if len(self.page) > 0:
				current_page = self.page[self.index]
				for l in current_page:
					l[0].draw(l[1],l[2],l[3])

				#Check focus
				i = self.focus
				while True:
					if i<0:
						if i == self.FOCUS_BACK:
							self.focus = i
							self.btn_back.on_get_focus()
							break
						elif i == self.FOCUS_NEXT \
							and self.index + 1 < len(self.page):
							self.focus = i
							self.btn_next.on_get_focus()
							break
						elif i == self.FOCUS_PREV and self.index > 0:
							self.focus = i
							self.btn_prev.on_get_focus()
							break
					elif current_page[i][4]:
						self.focus = i
						current_page[i][0].on_get_focus()
						break
					else:
						self.focus = self.focus + 1
					i = i + 1
					if i >= len(current_page):
						i = self.FOCUS_NEXT
				
			else:
				self.focus = self.FOCUS_BACK
				self.btn_back.on_get_focus()

			self.wnd.refresh()
			self.client.refresh()
			
			self.refresh = False

			#Key input
			while not self.refresh:
				key = self.get_input()
				self.dispatch_input(key)


	def draw_wnd(self):
		#Compute window size
		parent_rect = self.scr.get_size()
		self.rect = rect_t(parent_rect.width * 80 / 100,
			parent_rect.height * 80 / 100)
		self.pos = pos_t((parent_rect.height - self.rect.height) / 2,
			(parent_rect.width - self.rect.width) / 2)

		color = color_t()
		e = encoder()

		#Draw shadow
		shadow_color = color.get_color(color_t.BLACK,color_t.BLUE)
		for top in range(self.pos.top + 1,
			self.pos.top + self.rect.height + 1):
			self.scr.stdscr.addstr(top,self.pos.left + self.rect.width,
				e.convert('█'),shadow_color)
		for left in range(self.pos.left + 1,
			self.pos.left + self.rect.width):
			self.scr.stdscr.addstr(self.pos.top + self.rect.height,left,
				e.convert('█'),shadow_color)
		self.scr.stdscr.refresh()
		
		#Draw window
		self.wnd = self.scr.stdscr.subwin(self.rect.height,self.rect.width,
			self.pos.top,self.pos.left)
		self.wnd.bkgd(' ',color.get_color(0,color_t.WHITE))
		self.wnd.refresh()
		
		#Draw client region
		self.client_region = region_t(1,1,self.rect.width - 2,self.rect.height - 1 - 3)
		self.client = self.wnd.derwin(self.client_region.rect.height,self.client_region.rect.width,
			self.client_region.pos.top,self.client_region.pos.left)
		self.client.box()
		self.client.refresh()
		
		return

	def get_input(self):
		return self.scr.stdscr.getch()
		
	def dispatch_input(self,key):
		if key == ord('\t'):
			self.on_tab()
		elif key == curses.KEY_UP:
			self.on_up()
		elif key == curses.KEY_DOWN:
			self.on_down()
		elif key == curses.KEY_LEFT:
			self.on_left()
		elif key == curses.KEY_RIGHT:
			self.on_right()
		elif key == curses.KEY_PPAGE:
			self.on_prev()
		elif key == curses.KEY_NPAGE:
			self.on_next()
		elif key == 0x1B:
			self.on_back()
		else:
			if self.focus >= 0:
				self.page[self.index][self.focus][0].on_key_press(key)
			else:
				if self.focus == self.FOCUS_BACK:
					self.btn_back.on_key_press(key)
				elif self.focus == self.FOCUS_PREV:
					self.btn_prev.on_key_press(key)
				elif self.focus == self.FOCUS_NEXT:
					self.btn_next.on_key_press(key)
		return
	
	def analyse_menu(self,menu):
		#[[(control,pos,begin,max,able-focus),...],...]
		ret = []
		if menu == []:
			return ret
		max_height = self.client.getmaxyx()[0] - 2
		
		#Create controls
		ctrl_index = 0
		line = 1
		page = []
		while ctrl_index < len(menu):
			if line >= max_height:
				line = 1
				ret.append(page)
				page = []

			#Create control
			try:
				ctrl = self.control_dict[menu[ctrl_index][0]][0](self,self.client,menu[ctrl_index])
				indent = self.control_dict[menu[ctrl_index][0]][1]
			except KeyError:
				raise ControlTypeError(menu[ctrl_index][0])
			except TypeError:
				raise TypeError(str(menu))
				
			#Get control height
			ctrl_height = ctrl.get_size().height
			
			#Add control
			begin = 0
			while ctrl_height > max_height - line:
				page.append((ctrl,pos_t(line,1 + indent),begin,
					max_height - line,self.control_dict[menu[ctrl_index][0]][1]))
				begin = begin + (max_height - line)
				ctrl_height = ctrl_height - (max_height - line)
				line = 1
				ret.append(page)
				page = []
				
			page.append((ctrl,pos_t(line,1 + indent),begin,
				ctrl_height,self.control_dict[menu[ctrl_index][0]][1]))
			line = line + ctrl_height

			ctrl_index = ctrl_index + 1
		if page != []:
			ret.append(page)
			
		return ret
	
	def on_back(self):
		self.back = True
		self.refresh = True
		
	def on_prev(self):
		if self.index > 0:
			if self.focus >= 0: 
				self.page[self.index][self.focus][0].on_lost_focus()
				self.focus = 0
			self.index = self.index - 1
			self.refresh = True
		return
		
	def on_next(self):
		if self.index < len(self.page) - 1:
			if self.focus >= 0:
				self.page[self.index][self.focus][0].on_lost_focus()
				self.focus = 0
			self.index = self.index + 1
			self.refresh = True
		return
		
	def on_tab(self):
		if len(self.page) > 0:
			if self.focus >= 0:
				self.page[self.index][self.focus][0].on_lost_focus()
			self.focus = self.focus + 1
			if self.focus >= len(self.page[self.index]):
				self.focus = self.FOCUS_NEXT
			self.refresh = True
		return
		
	def on_up(self):
		if self.focus > 0 and len(self.page) > 0:
			i = self.focus - 1
			while i >= 0:
				if self.page[self.index][i][4]:
					self.page[self.index][self.focus][0].on_lost_focus()
					self.focus = i
					self.refresh = True
					break
				i = i - 1
		return
		
	def on_down(self):
		if self.focus >= 0 and len(self.page) > 0:
			for i in range(self.focus + 1,len(self.page[self.index])):
				if self.page[self.index][i][4]:
					self.page[self.index][self.focus][0].on_lost_focus()
					self.focus = i
					self.refresh = True
					break
		return
		
	def on_left(self):
		if self.focus < self.FOCUS_BACK:
			self.focus = self.focus + 1
			self.refresh = True
		return
		
	def on_right(self):
		if self.focus < 0 and self.focus > self.FOCUS_NEXT:
			self.focus = self.focus - 1
			if self.focus == self.FOCUS_PREV and self.index <= 0:
				self.focus = self.FOCUS_NEXT
			self.refresh = True
		return
