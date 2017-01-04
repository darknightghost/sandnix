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

class lable(control):
    def __init__(self,frame,wnd,data):
        self.wnd = wnd
        self.text = data[1]
        c = color_t()
        self.color = c.get_color(color_t.BLACK,color_t.WHITE) | curses.A_BOLD
        self.width = self.wnd.getmaxyx()[1] - 2
        self.text_width = self.width
        
        #Split text
        self.lines = []
        for l in self.text.split("\n"):
            while True:
                self.lines.append(l[ : self.text_width])
                l = l[self.text_width : ]
                if len(l) <= 0:
                    break;

    def draw(self,pos,begin,max):
        self.pos = pos
        
        for i in range(0, len(self.lines)):
            self.wnd.addstr(int(self.pos.top + i), int(self.pos.left), self.lines[i], self.color)
        return len(self.lines)

    def get_size(self):
        return rect_t(self.width, len(self.lines))
