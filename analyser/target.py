#! /usr/bin/env python3
# -*- coding: utf-8 -*-
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
import os
from analyser.target_exceptions import *
from analyser.arch import *
from analyser.options import *

def get_child_tags_by_name(parent, name):
    ret = []
    for node in parent.getElementsByTagName(name):
        if node.parentNode == parent:
            ret.append(node)
    return ret

class target:
    KEEP = 0
    RESERVE = 1
    CONFLICT = 3
    target_dict = {}
    def __init__(self, path, actived_arch = None):
        self.path = os.path.abspath(path)
        old_path = os.path.abspath(".")
        os.chdir(os.path.dirname(self.path))
        self.file = open(self.path, "r+")
        self.dom = xml.dom.minidom.parse(self.file)
        self.root = self.dom.documentElement
        if actived_arch == None:
            self.is_root = True
        else:
            self.is_root = False
        self.__load__(actived_arch)
        os.chdir(old_path)

    def __del__(self):
        try:
            self.file.close()
        except Exception:
            pass

    def __str__(self):
        ret = "************************************************\n"
        ret = ret + "\nTarget info:"
        ret = ret + "\n%12s: %s"%("name", self.name)
        ret = ret + "\n%12s: %s"%("type", self.build_type)
        ret = ret + "\n%12s: %s"%("path", self.path)
        ret = ret + "\n%12s: %s"%("output", self.output)
        ret = ret + "\n%12s: %s"%("outdir", self.outdir)
        ret = ret + "\n%12s: %s"%("middir", self.middir)
        ret = ret + "\n%12s: %s"%("Introduction", self.introduction)
        ret = ret + "\n%12s: %s"%("Actived arch", self.arch_name)
        ret = ret + "\n%12s:"%("Architectures")
        for i in self.base_archs:
            ret = ret + "\n" + str(i)

        ret = ret + "\n%12s:"%("Dependencies")
        for d in self.dependencies:
            ret = ret + "\n%12s = \"%s\""%("path", d)

        ret = ret + "\n%12s:"%("Options")
        for o in self.options:
            ret = ret + "\n\t" + str(o)

        ret = ret + "\n%12s:"%("Sub targets")
        for t in self.sub_targets:
            ret = ret + "\n" + str(t[1])

        return ret

    def close(self):
        #Architectures
        for t in self.base_archs:
            t.close()
        
        #Sub targets
        for t in self.sub_targets:
            t[1].close()
        
        #Options
        for t in self.options:
            t.close()
        
        target.target_dict.pop(self.path)
        self.__restore__()
        try:
            self.file.seek(0,0)
            self.file.truncate(0)
            self.dom.writexml(self.file, addindent='', newl='', encoding='utf-8')
            self.file.close()
        except Exception:
            pass
        self.file = None
        self.dom = None
        return

    def __load__(self, actived_arch):
        #Name
        self.name = self.root.getAttribute("name").encode('utf-8').decode()
        
        #type
        self.build_type = self.root.getAttribute("type").encode('utf-8').decode()
        if self.build_type != "build" and self.build_type != "virtual":
            raise MissingAttribute(self.path, "target", "type")
        
        #Output file name
        try:
            self.output_node = get_child_tags_by_name(self.root, "output")[0]
        except IndexError:
            raise MissingTag(self.path, "output")
        self.output = self.output_node.getAttribute("name").encode('utf-8').decode()
        if self.output == "" and self.build_type != "virtual":
            raise MissingAttribute(self.path, "output", "name")

        #Output dir
        try:
            self.outdir_node = get_child_tags_by_name(self.root, "outdir")[0]
        except IndexError:
            raise MissingTag(self.path, "outdir")
        self.outdir = self.outdir_node.getAttribute("path").encode('utf-8').decode()
        if self.output == "" and self.build_type != "virtual":
            raise MissingAttribute(self.path, "outdir", "path")

        #Middie dir
        try:
            self.middir_node = get_child_tags_by_name(self.root, "middir")[0]
        except IndexError:
            raise MissingTag(self.path, "middir")
        self.middir = self.middir_node.getAttribute("path").encode('utf-8').decode()
        if self.middir == "" and self.build_type != "virtual":
            raise MissingAttribute(self.path, "middir", "path")

        #Introduction
        try:
            self.introduction_node = get_child_tags_by_name(self.root, "introduction")[0]
        except IndexError:
            raise MissingTag(self.path, "introduction")
        try:
            self.introduction = self.introduction_node.childNodes[0].nodeValue.encode('utf-8').decode()
        except IndexError:
            self.introduction = ""
            
        tmp_introduction = self.introduction
        self.introduction = ""
        for l in tmp_introduction.split("\n"):
            if self.introduction != "":
                self.introduction = self.introduction + "\n"
            self.introduction = self.introduction + l.strip()

        #Architectures
        self.archs = {}
        self.base_archs = []
        try:
            self.archs_node = get_child_tags_by_name(self.root, "archs")[0]
            if actived_arch == None:
                self.arch_name = self.archs_node.getAttribute("actived").encode('utf-8').decode()
                if self.arch_name == "":
                    raise MissingAttribute(self.path, "archs", "actived")
            else:
                self.arch_name = actived_arch.name
        except IndexError:
            if actived_arch == None:
                raise MissingTag(self.path, "archs")

        else:
            #Scan arch list
            for node in get_child_tags_by_name(self.archs_node, "arch"):
                current_arch = arch(node, self.dom, self.path)
                current_arch.regist(self.archs)
                self.base_archs.append(current_arch)

        #Get actived arch
        try:
            self.actived_arch = self.archs[self.arch_name]
        except KeyError:
            if actived_arch == None:
                raise MissingArch(self.path, self.arch_name)
            else:
                self.actived_arch = actived_arch
        
        #Dependencies
        self.dependencies = []
        try:
            dep_node = get_child_tags_by_name(self.root, "dependencies")[0]
        except IndexError:
            raise MissingTag(self.path, "dependencies")
        for dep in get_child_tags_by_name(dep_node, "dep"):
            try:
                new_dep = os.path.abspath(os.path.abspath(dep.getAttribute("path").encode('utf-8').decode()))
                if not os.path.exists(new_dep):
                    raise FileNotFoundError(new_dep)
                self.dependencies.append(new_dep)
            except IndexError:
                raise MissingAttribute(path, "dep", "path")

        #Sub targets
        #[node, target, enabled, checkbox]
        self.sub_targets = []
        try:
            subtarget_node = get_child_tags_by_name(self.root, "sub-targets")[0]
        except IndexError:
            raise MissingTag(self.path, "sub-targets")
        for subtarget in get_child_tags_by_name(subtarget_node, "target"):
            self.sub_targets.append([subtarget,
                target(subtarget.getAttribute("path"), self.actived_arch),
                subtarget.getAttribute("enable").encode('utf-8').decode().lower() == "true", None])

        #Options
        self.options = []
        try:
            options_node = get_child_tags_by_name(self.root, "options")[0]
        except IndexError:
            raise MissingTag(self.path, "options")
        for o in get_child_tags_by_name(options_node, "option"):
            self.options.append(get_option(o, self.path))

        target.target_dict[self.path] = self

        return

    def __restore__(self):
        #Actived arch
        if self.is_root:
            self.archs_node.setAttribute("actived", self.arch_name)
        
        #Sub targets
        for t in self.sub_targets:
            t[0].setAttribute("enable", str(t[2]).lower())
        return

    def open_menu(self):
        #Intruduction
        self.menu = [["lable", "Introduction:", None]]
        self.menu.append(["lable", self.introduction, None])
        
        #Dependencies
        dep_str = "\nRequired targets:"
        for d in self.dependencies:
            dep_str = dep_str + "\n    " + target.target_dict[d].name
            self.menu.append(["lable", dep_str, None])
        
        self.menu.append(["lable", "", None])
        self.menu.append(["lable", "Architecture:", None])
        #Actived arch
        if self.is_root:
            arch_list = []
            i = 0
            selected = 0
            for a in self.base_archs:
                arch_list.append(a.name)
                if a.name == self.arch_name:
                    selected = i
                i = i + 1
            self.active_arch_menu = ["listcontrol", "Actived architecture", [arch_list, selected]]
            self.menu.append(self.active_arch_menu)
        
        #Architecuture settings
        if self.build_type == "build":
            arch_setting_menu = []
            for a in self.base_archs:
                arch_setting_menu.append(a.open_menu())
            self.menu.append(["submenu", "Architecture settings" , arch_setting_menu])

        #Build options
        if self.build_type == "build":
            if len(self.options) > 0:
                self.menu.append(["lable", "", None])
                self.menu.append(["lable", "Build options:", None])
                option_menu = []
                for opt in self.options:
                    option_menu.append(opt.open_menu())
                self.menu.append(["submenu", "Build options" , option_menu])
        
        #Sub targets
        if self.build_type == "build":
            if len(self.sub_targets) > 0:
                self.menu.append(["lable", "", None])
                self.menu.append(["lable", "Sub targets:", None])
                sub_targets_menu = []
                for t in self.sub_targets:
                    sub_targets_menu.append(["lable", t[1].name, None])
                    c = ["checkbox", "Build this target", t[2]]
                    sub_targets_menu.append(c)
                    t[3] = c
                    sub_targets_menu.append(["submenu", t[1].name + " options", t[1].open_menu()])
                self.menu.append(["submenu", "Sub targets options" , sub_targets_menu])
        else:
            if len(self.sub_targets) > 0:
                self.menu.append(["lable", "", None])
                self.menu.append(["lable", "Sub targets:", None])
                for t in self.sub_targets:
                    self.menu.append(["lable", " " * 4 + t[1].name + ":", None])
                    c = ["checkbox", "Build this target", t[2]]
                    self.menu.append(c)
                    t[3] = c
                    self.menu.append(["submenu", t[1].name + " options", t[1].open_menu()])
                    self.menu.append(["lable", " ", None])
        
        return self.menu

    def close_menu(self):
        #Actived arch
        if self.is_root:
            self.arch_name = self.base_archs[self.active_arch_menu[2][1]].name
            self.active_arch_menu = None
        
        #Architecuture settings
        if self.build_type == "build":
            for a in self.base_archs:
                a.close_menu()
        
        #Options
        if self.build_type == "build":
            for opt in self.options:
                opt.close_menu()
            
        #sub targets
        for t in self.sub_targets:
            t[2] = t[3][2]
            t[3] = None
            t[1].close_menu()
        
        self.menu = None
            
        return

    def configure(self):
        #Arch
        build_dict = self.actived_arch.configure()
        
        #Options
        for opt in self.options:
            build_dict = opt.configure(build_dict)
            
        ret = ["NAME = " + self.name,
               "ARCH = " + self.arch_name,
               "OUTPUT = " + self.output,
               "OUTDIR = " + self.outdir,
               "MIDDIR = " + self.middir]
        
        for k in arch.tag_list:
            ret.append(k + " = " + build_dict[k])
        
        return ret

    def get_sub_targets(self):
        ret = []
        for l in self.sub_targets:
            if l[2]:
                ret.append(l[1])
        return ret
    
    def get_all_sub_targets(self):
        ret = []
        for l in self.sub_targets:
            if l[2]:
                ret.append(l[1])
                ret = ret + l[1].get_all_sub_targets()
        return ret
    
    def get_dependencies(self):
        sub_targets = self.get_all_sub_targets()
        ret = []

        for p in self.dependencies:
            ret.append(target.target_dict[p])
            
        for t in sub_targets:
            ret = ret + t.get_dependencies()

        for d in ret:
            if d in sub_targets:
                ret.remove(d)
        
        return ret
    
    def check_order(t1, t2):
        t1_deps = t1.get_dependencies()
        t2_deps = t2.get_dependencies()
        t1_subs = t1.get_all_sub_targets()
        t2_subs = t2.get_all_sub_targets()
        
        t1_subs.append(t1)
        t2_subs.append(t2)
        
        if len(list(set(t1_subs) & set(t2_deps))) > 0:
            if len(list(set(t2_subs) & set(t1_deps))) > 0:
                return target.CONFLICT
            else:
                return target.KEEP
        elif len(list(set(t2_subs) & set(t1_deps))) > 0:
            return target.RESERVE
        else:
            return target.KEEP
