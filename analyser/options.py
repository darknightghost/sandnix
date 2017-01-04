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

class options:
    def __init__(self, node, path):
        self.root = node
        self.path = path
        self.__load__()

    def close(self):
        self.__restore__()

    def __load__(self):
        raise Exception("abstract")

    def __restore__(self):
        raise Exception("abstract")

    def open_menu(self):
        raise Exception("abstract")

    def close_menu(self):
        raise Exception("abstract")

    def configure(self, dict):
        raise Exception("abstract")

class opt_checkbox(options):
    def __str__(self):
        return "%12s: %s"%("Build option", self.value)
        
    def __load__(self):
        #name
        try:
            self.name = self.root.getAttribute("name").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "name")

        #value
        try:
            self.value = self.root.getAttribute("value").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "value")

        #enable
        try:
            self.enable = (self.root.getAttribute("enable").encode('utf-8').decode().lower() == "true")
        except IndexError:
            raise MissingAttribute(self.path, "option", "enable")

        #target
        try:
            target_str = self.root.getAttribute("target").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "target")
        self.targets = []
        for k in target_str.split("|"):
            k = k.strip()
            if k != "":
                self.targets.append(k)
        
        return

    def __restore__(self):
        self.root.setAttribute("enable", str(self.enable).lower())
        return

    def open_menu(self):
        self.menu = ["checkbox", self.name, self.enable]
        return self.menu

    def close_menu(self):
        self.enable = self.menu[2]
        self.menu = None
        return

    def configure(self, dict):
        if self.enable:
            for k in self.targets:
                if dict[k] != "":
                    dict[k] = dict[k] + " "
                dict[k] = dict[k] + self.value
            
        return dict

class opt_list(options):
    def __str__(self):
        return "%12s: %s"%("Build option", self.items[self.selected][1])

    def __load__(self):
        #name
        try:
            self.name = self.root.getAttribute("name").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "name")

        #Selected index
        try:
            self.selected = int(self.root.getAttribute("selected").encode('utf-8').decode())
        except IndexError:
            raise MissingAttribute(self.path, "option", "selected")
        except ValueError:
            raise MissingAttribute(self.path, "option", "selected")

        #target
        try:
            target_str = self.root.getAttribute("target").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "target")
        self.targets = []
        for k in target_str.split("|"):
            k = k.strip()
            if k != "":
                self.targets.append(k)

        #item
        #[name, value]
        self.items = []
        for item in self.root.getElementsByTagName("item"):
            self.items.append([item.getAttribute("name").encode('utf-8').decode(),
                item.getAttribute("value").encode('utf-8').decode()])
        return

    def __restore__(self):
        self.root.setAttribute("selected", str(self.selected))
        return

    def open_menu(self):
        item_name_list = []
        for n in self.items:
            item_name_list.append(n[0])
        self.menu = ["listcontrol", self.name, [item_name_list, self.selected]]
        return self.menu

    def close_menu(self):
        self.selected = self.menu[2][1]
        self.menu = None

    def configure(self, dict):
        for k in self.targets:
            if dict[k] != "":
                dict[k] = dict[k] + " "
            dict[k] = dict[k] + self.items[self.selected][1]
            
        return dict

class opt_input(options):
    def __str__(self):
        return "%12s: %s=%s"%("Build option", self.macro, self.value)

    def __load__(self):
        #name
        try:
            self.name = self.root.getAttribute("name").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "name")

        #macro
        try:
            self.macro = self.root.getAttribute("macro").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "macro")

        #value
        try:
            self.value = self.root.getAttribute("value").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "value")

        #target
        try:
            target_str = self.root.getAttribute("target").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "target")
        self.targets = []
        for k in target_str.split("|"):
            k = k.strip()
            if k != "":
                self.targets.append(k)

        return

    def __restore__(self):
        self.root.setAttribute("value", self.value)
        return

    def open_menu(self):
        self.menu = ["textbox", self.name, self.value]
        return self.menu

    def close_menu(self):
        self.value = self.menu[2]
        self.menu = None

    def configure(self, dict):
        for k in self.targets:
            if dict[k] != "":
                dict[k] = dict[k] + " "
            dict[k] = dict[k] + self.macro + "=" + self.value
            
        return dict

class opt_menu(options):
    def __str__(self):
        ret = ""
        for opt in self.options:
            ret = ret + str(opt)
        return ret

    def __load__(self):
        #name
        try:
            self.name = self.root.getAttribute("name").encode('utf-8').decode()
        except IndexError:
            raise MissingAttribute(self.path, "option", "name")

        #options
        self.options = []
        for opt_node in self.root.getElementsByTagName("option"):
            self.options.append(get_option(opt_node, self.path))
        return

    def close(self):
        for opt in self.options:
            opt.close()
        return

    def open_menu(self):
        submenu = []
        for opt in self.options:
            submenu.append(opt.open_menu())
        self.menu = ["submenu", self.name, submenu]
        return self.menu

    def close_menu(self):
        for opt in self.options:
            opt.close_menu()
        self.menu = None
        return

    def configure(self, dict):
        for o in self.options:
            dict = o.configure(dict)
            
        return dict

OPTION_DICT = {"checkbox" : opt_checkbox,
    "list" : opt_list,
    "input" : opt_input,
    "menu" : opt_menu}
def get_option(node, path):
    global OPTION_DICT
    return OPTION_DICT[node.getAttribute("type")](node, path)
