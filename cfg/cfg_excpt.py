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
class ConflictedName(Exception):
	def __init__(self,name):
		self.name = name
		return

class ConflictedArchName(ConflictedName):
	def __str__(self):
		return "Conflicted name of architecture: %s."%(self.name)

class UnknownName(Exception):
	def __init__(self,name):
		self.name = name
		return

class ElementNotFound(Exception):
	def __init__(self,name):
		self.name = name
		return
		
	def __str__(self):
		return "Element %s not found!"%(self.name)

class UnknownNode(Exception):
	def __init__(self,name,nodetype):
		self.name = name
		self.nodetype = nodetype
		return

	def __str__(self):
		return "Unknow name of node,name=%s,type=%s."%(self.name,self.nodetype)
