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
import sys
import locale
import traceback
from ui.data import *
from ui import menu

class screen:
    '''
        This class is used to show a menu in console
    Example:
        from ui.ui import *

        scr = screen.screen()
        scr.screen_main(menu, title)
        
    The menu is a list in the format of
    [[type,text,value],
        [type,text,value],
        [type,text,value],
        ...
        [type,text,value]]
        
    Current support types:
    Type            Value                                       Description
    "lable"         None                                        Static text
    "submenu"       menu                                        Sub Menu Entery
    "checkbox"      True or False                               CheckBox
    "textbox"       string                                      TextBox
    "listcontrol"   [[text1,text2,text3...],selected-index]     Show a list and select one
    '''
    str_help = "Press <ESC> to return.Press <Page Up>, <Page Down> to switch pages.Press <↑>, <↓>, <←>, <→> to switch focus in one window.Press <TAB> to switch focus among all windows."
    def __init__(self):
        locale.setlocale(locale.LC_ALL, '')
        self.stdscr = None
        self.width = 0
        self.height = 0

    def screen_main(self,menu_list,title):
        try:
            #Begin GUI
            self.stdscr = curses.initscr()
            self.height = self.stdscr.getmaxyx()[0] + 1
            self.width = self.stdscr.getmaxyx()[1] + 1
            curses.noecho()
            curses.cbreak()
            self.stdscr.keypad(1)
            color = color_t()
            color.init_color()
            self.stdscr.nodelay(0)
            
            #Draw background
            self.stdscr.bkgd(' ', color.get_color(0,color_t.BLUE))
            curses.curs_set(0)
            self.stdscr.addstr(0, 0, title,
                color.get_color(color_t.WHITE, color_t.BLUE) | curses.A_BOLD)
            
            str_help_line_num = len(screen.str_help) // self.width
            if len(screen.str_help) % self.width > 0:
                str_help_line_num = str_help_line_num + 1
                
            self.stdscr.addstr(self.height - 1 - str_help_line_num, 0, screen.str_help,
                color.get_color(color_t.WHITE, color_t.BLUE) | curses.A_BOLD)
            
            self.update()
            
            #Create menu window
            m = menu.menu(self, title)
            m.show_menu(menu_list)

        except Exception as e:
            #End GUI
            self.stdscr.keypad(0)
            curses.echo()
            curses.nocbreak()
            curses.endwin()
            traceback.print_exc()
            raise e
        else:
            #End GUI
            self.stdscr.keypad(0)
            curses.echo()
            curses.nocbreak()
            curses.endwin()
                
        
    def update(self):
        self.stdscr.refresh()
        
    def get_size(self):
        '''
            Return the size of screen in the form of (width,height).
        '''
        return rect_t(self.width,self.height)
