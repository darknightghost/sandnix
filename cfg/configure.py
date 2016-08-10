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

from analyser.target import *
from analyser.arch import *
from analyser.target_exceptions import *
from cfg.err import *
import os
import stat

'''
    Data structure of build tree:
        [target, [sub-targets]]
'''
def configure(root_target):
    ret = []

    #Get build tree
    print("\nScaning targets...")
    missing_deps = root_target.get_dependencies()
    if len(missing_deps) != 0:
        raise MissingDepecncency(missing_deps)
    ret = get_build_tree(root_target)
    
    #Sort build tree
    print("\nComputing build order...")
    sort_build_tree(ret)

    return ret

def create_makefile(build_tree):
    cur_target = build_tree[0]
    print("\n" + ("*" * 80))
    print("Configuring target : \"%s\"...\npath = \"%s\""%(cur_target.name, cur_target.path))
    
    #Change working directory
    old_dir = os.path.abspath(".")
    cur_dir = os.path.dirname(cur_target.path)
    os.chdir(cur_dir)
    
    #Get build options
    if cur_target.build_type == "build":
        #build
        print("Target type : build")
        
        cfg_settings = cur_target.configure()
        
        #Scan source files
        sources = scan_sources(cur_target.arch_name)
        #filelist = [[source, obj, dep], [source, obj, dep], ...]
        filelist = []
        linkfile = "$(MIDDIR)/$(NAME).$(ARCH).linked"
        for s in sources:
            basename = os.path.splitext(s)[0]
            filelist.append([os.path.abspath(s),
                os.path.abspath("$(MIDDIR)/" + basename + ".o"),
                os.path.abspath("$(MIDDIR)/" + basename + ".dep")])

        #Create Makefile
        print("Create Makefile...")
        makefile = open("Makefile", "w")
        
        #Build options
        print("Writing building options...")
        for line in cfg_settings:
            makefile.write(line + "\n")
        makefile.write("LINKED = %s\n"%(linkfile))
        makefile.write("\n")
        
        print("Writing building rules...")
        makefile.write(".PHONY : all clean delete rebuild subtarget target\n\n")
        
        #all
        makefile.write("all : target\n\n")
        for f in filelist:
            makefile.write("\trm -f %s\n"%(f[2].strip()))
        
        #clean
        makefile.write("clean : \n")
        if len(build_tree[1]) > 0:
            for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make clean\n"
                makefile.write(cmd)

        for f in filelist:
            makefile.write("\trm -f %s\n"%(f[1].strip()))
            makefile.write("\trm -f %s\n"%(f[2].strip()))
        makefile.write("\trm -f %s\n"%(linkfile.strip()))
        makefile.write("\n")
        
        #delete
        makefile.write("delete : \n")
        if len(build_tree[1]) > 0:
            for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make clean\n"
                makefile.write(cmd)

        for f in filelist:
            makefile.write("\trm -f %s\n"%(f[1].strip()))
            makefile.write("\trm -f %s\n"%(f[2].strip()))
        makefile.write("\trm -f %s\n"%(linkfile.strip()))
        makefile.write("\trm -f $(OUTDIR)/$(OUTPUT)\n")
        makefile.write("\n")
        
        #rebuild
        makefile.write("rebuild : \n")
        if len(build_tree[1]) > 0:
            for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make clean\n"
                makefile.write(cmd)
        makefile.write("\tmake delete\n")
        makefile.write("\tmake all\n")
        makefile.write("\n")
        
        #target
        if len(build_tree[1]) > 0:
            makefile.write("target : subtarget\n")
        else:
            makefile.write("target :\n")
        makefile.write("\tmkdir -p $(OUTDIR)\n")
        makefile.write("\t$(PREV)\n")
        makefile.write("\tmake $(LINKED)\n")
        makefile.write("\t$(AFTER)\n")
        makefile.write("\n")
        
        #subtarget
        if len(build_tree[1]) > 0:
            makefile.write("subtarget :\n")
            for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make clean\n"
                makefile.write(cmd)
            makefile.write("\n")

        #link
        link_deps = ""
        for f in filelist:
            if len(link_deps) == 0:
                link_deps = link_deps + f[1]
            else:
                link_deps = link_deps + " \\\n\t" + f[1]
        link_deps = link_deps.strip()
        makefile.write("$(LINKED) : %s\n"%(link_deps))
        makefile.write("\t$(LDRULE)\n")
        makefile.write("\n")
        
        #sources
        for f in filelist:
            makefile.write("include %s\n"%(f[2]))
            makefile.write("%s : %s\n"%(f[2], f[0]))
            makefile.write("\tmkdir -p $(dir $@)\n")
            makefile.write("\t$(DEPRULE)\n")

            makefile.write("%s : %s\n"%(f[1], f[0]))
            makefile.write("\tmkdir -p $(dir $@)\n")
            if os.path.splitext(f[0])[1] in (".s", ".S"):
                makefile.write("\t$(ASRULE)")
            elif os.path.splitext(f[0])[1] in (".c", ".cc", ".C", ".cpp"):
                makefile.write("\t$(CCRULE)")

            makefile.write("\n")
        
    else:
        #virtual
        print("Target type : virtual")
        
        cfg_settings = cur_target.configure()

        #Create Makefile
        print("Create Makefile...")
        makefile = open("Makefile", "w")
        for line in cfg_settings:
            makefile.write(line + "\n")
        makefile.write("\n")
        
        print("Writing building rules...")
        makefile.write(".PHONY : all clean delete rebuild\n\n")
        
        #all
        makefile.write("all :\n")
        makefile.write("\t$(PREV)\n")
        if len(build_tree[1]) > 0:
            for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make all\n"
                makefile.write(cmd)
        makefile.write("\t$(AFTER)\n")
        makefile.write("\n")
        
        #clean
        makefile.write("clean :\n")
        if len(build_tree[1]) > 0:
            for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make clean\n"
                makefile.write(cmd)
        makefile.write("\n")
        
        #delete
        makefile.write("delete :\n")
        for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make delete\n"
                makefile.write(cmd)
        makefile.write("\n")
        
        #rebuild
        makefile.write("rebuild :\n")
        for s in build_tree[1]:
                cmd = "\tcd " + os.path.dirname(s[0].path) + ";make rebuild\n"
                makefile.write(cmd)
        makefile.write("\n")
    
    makefile.close()
    
    #Change working directory
    os.chdir(old_dir)
    
    for s in build_tree[1]:
        create_makefile(s)
    return

def get_build_tree(root):
    print("Target found : \"%s\".\npath = \"%s\"."%(root.name, root.path))
    sub_targets = []
    for t in root.get_sub_targets():
        sub_targets.append(get_build_tree(t))
    return [root, sub_targets]

def sort_build_tree(build_tree):
    sub_targets = build_tree[1]
    for t in sub_targets:
        sort_build_tree(t)
    while True:
        flag = False
        for i in range(0, len(sub_targets) - 1):
            order = target.check_order(sub_targets[i][0], sub_targets[i + 1][0])
            if order == target.RESERVE:
                sub_targets[i], sub_targets[i + 1] = sub_targets[i + 1], sub_targets[i]
                flag = True
            elif order == target.CONFLICT:
                raise ConfilctDepecncency(sub_targets[i][0].name, sub_targets[i + 1][0].name)
        if flag == False:
            break
    return

def scan_sources(arch_name):
    name_list = [""] + arch_name.split(".")
    source_list = []
    for n in range(0, len(name_list)):
        #Open source list file
        file_name = "sources"
        for i in range(0, len(name_list) - n):
            if i > 0:
                file_name = file_name + "." + name_list[i]
        try:
            print("\nTrying to open file : \"%s\"."%(file_name))
            source_file = open(file_name, "r")
        except FileNotFoundError:
            print("File does not exists.")
            continue
        
        #Get source list
        for line in source_file.readlines():
            line = line.strip().split('#')[0].strip()
            if line != "":
                print("Checking source file : \"%s\"."%(line))
                if not os.path.exists(line):
                    raise MissingSourceFile(line)
                
                if os.path.isdir(line):
                    raise SourceFileIsDir(line)
                
                source_list.append(line)
        source_file.close()
        
    return source_list
