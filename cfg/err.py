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

class ConfilctDepecncency(Exception):
    def __init__(self, t1_name, t2_name):
        self.t1_name = t1_name
        self.t2_name = t2_name
        
    def __str__(self):
        return "Conflict target : \"%s\" \"%s\"."%(self.t1_name, self.t2_name)
    
class MissingDepecncency(Exception):
    def __init__(self, deps):
        self.missing_names = ""
        for t in deps:
            if self.missing_names == "":
                self.missing_names = self.missing_names + "\"%s\""%(t.name)
            else:
                self.missing_names = self.missing_names + ", \"%s\""%(t.name)
        
    def __str__(self):
        return "Missing depencencies : %s."%(self.missing_names)
    
class MissingSourceFile(Exception):
    def __init__(self, path):
        self.path = path
        
    def __str__(self):
        return "Missing source file : \"%s\"."%(self.path)
    
class SourceFileIsDir(Exception):
    def __init__(self, path):
        self.path = path
        
    def __str__(self):
        return "Source file : \"%s\" is a directory."%(self.path)
