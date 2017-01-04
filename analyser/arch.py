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

import xml.dom.minidom
from analyser.target_exceptions import *
from analyser.target import *

def get_child_tags_by_name(parent, name):
    ret = []
    for node in parent.getElementsByTagName(name):
        if node.parentNode == parent:
            ret.append(node)
    return ret

class arch:
    tag_list = ["PREV", "ARCHDEF", "DEP", "DEPFLAGS", "DEPRULE", "CC", "CFLAGS", "CCRULE", "AS", "ASFLAGS",
        "ASRULE", "LD", "LDFLAGS", "LDRULE", "AR", "ARRULE", "AFTER"]
    def __init__(self, node, dom, path, parent = None):
        self.path = path
        self.root = node
        self.dom = dom
        self.__load__(parent)

    def __str__(self):
        ret = "Architecture \"%s\":"%(self.name)
        ret = ret + "\n%12s: %s"%("name", self.name)
        for k in self.build_dict.keys():
            ret = ret + "\n%12s: %s"%(k, self.build_dict[k][1])
        ret = ret + "\n"
        return ret

    def close(self):
        self.__restore__()

    def regist(self, dict):
        dict[self.name] = self

    def __load__(self, parent):
        #name
        try:
            self.name = self.root.getAttribute("name").encode('utf-8').decode()
        except TypeErrot:
            raise MissingAttribute(self.path, "arch", "name")
        
        #Makeflie variables
        #[node, value]
        self.build_dict = {}
        for k in arch.tag_list:
            try:
                self.build_dict[k] = [get_child_tags_by_name(self.root, k)[0]]
            except IndexError:
                raise ArchMissingTag(self.path, k, self.name)
            try:
                self.build_dict[k].append(self.build_dict[k][0].childNodes[0].nodeValue.encode('utf-8').decode())
            except IndexError:
                self.build_dict[k].append("")
                
        return

    def __restore__(self):
        #Makeflie variables
        #[node, value]
        for k in arch.tag_list:
            if len(self.build_dict[k][0].childNodes) == 0:
                self.build_dict[k][0].appendChild(self.dom.createTextNode(self.build_dict[k][1]))
            else:
                self.build_dict[k][0].childNodes[0].nodeValue = self.build_dict[k][1]

    def open_menu(self):
        self.menu = []
        for k in arch.tag_list:
            self.menu.append(["textbox", k, self.build_dict[k][1]])
        return ["submenu", self.name , self.menu]

    def close_menu(self):
        for c in self.menu:
            self.build_dict[c[1]][1] = c[2]
        self.menu = None
        return

    def configure(self):
        ret = {}
        for k in self.build_dict.keys():
            ret[k] = self.build_dict[k][1]
        return ret
