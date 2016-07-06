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
from ui.ui import *

class control:
    def __init__(self,frame,wnd,data):
        pass

    def draw(self,pos,begin,max):
        return 0

    def get_size(self):
        return rect_t(0,0)

    def on_get_focus(self):
        pass

    def on_lost_focus(self):
        pass

    def on_key_press(self,key):
        pass
