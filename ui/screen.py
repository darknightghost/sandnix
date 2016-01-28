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
import traceback
import locale
from data import *
from menu import *

class screen:
	'''
		This class is used to show a menu in console
	Example:
		from ui.ui import *

		scr = screen.screen()
		scr.screen_main(menu)
		
	The menu is a list in the format of
	[[type,text,value],
		[type,text,value],
		[type,text,value],
		...
		[type,text,value]]
		
	Current support types:
	Type			Value										Description
	"lable"			None										Static text
	"submenu"		menu										Sub Menu Entery
	"checkbox"		True or False								CheckBox
	"textbox"		string										TextBox
	"listcontrol"	[[text1,text2,text3...],selected-index]		Show a list and select one
	'''
	def __init__(self):
		locale.setlocale(locale.LC_ALL, '')
		self.stdscr = None
		self.width = 0
		self.height = 0
	
	def screen_main(self,menu_list,title):
		success = True
		try:
			#Begin GUI
			self.stdscr = curses.initscr()
			self.height = self.stdscr.getmaxyx()[0]
			self.width = self.stdscr.getmaxyx()[1]
			curses.noecho()
			curses.cbreak()
			self.stdscr.keypad(1)
			color = color_t()
			color.init_color()
			self.stdscr.nodelay(0)
			
			#Draw background
			self.stdscr.bkgd(' ',color.get_color(0,color_t.BLUE))
			curses.curs_set(0)
			e = encoder()
			self.stdscr.addstr(0,0,e.convert(title),
				color.get_color(color_t.WHITE,color_t.BLUE) | curses.A_BOLD)
			self.update()
			
			#Create menu window
			m = menu(self,title)
			m.show_menu(menu_list)

		except:
			success = False
		finally:
			#End GUI
			self.stdscr.keypad(0)
			curses.echo()
			curses.nocbreak()
			curses.endwin()
			if not success:
				traceback.print_exc()
		
	def update(self):
		self.stdscr.refresh()
		
	def get_size(self):
		'''
			Return the size of screen in the form of (width,height).
		'''
		return rect_t(self.width,self.height)
