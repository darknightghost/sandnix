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

from cfg import *
from cfg_excpt import *
import os

def configure(path):
	configure = cfg(path)
	target_list = []
	arch = configure.get_build_options(target_list)
	
	for t in target_list:
		create_target_makefile(t,arch)

	print("Creating %s/Makefile..."%(os.path.dirname(path)))
	try:
		make_file = open("Makefile","x")
	except:
		make_file = open("Makefile","w")
	
	make_file.write(".PHONY : all clean delete rebuild\n")
	#all
	make_file.write("all :\n")
	for t in target_list:
		make_file.write("\tcd %s;make all\n"%(t[0]))
	
	#clean
	make_file.write("clean :\n")
	for t in target_list:
		make_file.write("\tcd %s;make clean\n"%(t[0]))
		
	#delete
	make_file.write("delete :\n")
	for t in target_list:
		make_file.write("\tcd %s;make delete\n"%(t[0]))
		
	#rebuild
	make_file.write("rebuild :\n")
	for t in target_list:
		make_file.write("\tcd %s;make rebuild\n"%(t[0]))
	return

def create_target_makefile(target,arch):
	#Get config
	path = target[0]
	options = target[1]
	old_path = os.getcwd()
	os.chdir(path)
	print("Creating %s/Makefile..."%(path))
	try:
		make_file = open("Makefile","x")
	except:
		make_file = open("Makefile","w")
	sources = get_sources(arch)
	
	#Create makefile
	make_file.write(options)
	
	make_file.write(".PHONY : $(TARGET) all clean delete rebuild\n")
	make_file.write("all : $(TARGET)\n")
	
	#Target
	objs = ""
	deps = ""
	#Sources
	for s in sources:
		base_name = os.path.splitext(s)[0]
		ext_name = os.path.splitext(s)[1]
		make_file.write("sinclude $(OBJDIR)/$(ARCH)/%s.dep\n"%(base_name))
		#.dep
		make_file.write("$(OBJDIR)/$(ARCH)/%s.dep : %s\n"%(base_name,s))
		make_file.write("\tmkdir -p $(dir $@)\n\t$(DEPRULE)\n")
		objs = "%s$(OBJDIR)/$(ARCH)/%s.o "%(objs,base_name)
		make_file.write("$(OBJDIR)/$(ARCH)/%s.o : %s\n"%(base_name,s))
		deps = deps + " " + "$(OBJDIR)/$(ARCH)/%s.dep"%(base_name)
		if ext_name in [".c"]:
			#.c
			make_file.write("\t$(CCRULE)\n")
		elif ext_name in [".s",".S"]:
			#.s,.S
			make_file.write("\t$(ASRULE)\n")
			
	#Subtargets
	subtargets = target[3]
	
	#Link
	make_file.write("$(OUTPUT) : %s\n"%(objs))
	make_file.write("\tmkdir -p $(dir $(OUTPUT))\n")
	make_file.write("\t$(LDRULE)\n")
	
	#Target
	make_file.write("$(TARGET) : $(OUTPUT)\n")
	for t in subtargets:
		make_file.write("\tcd %s;make all\n"%(t[0]))
		create_target_makefile(t,arch)
	make_file.write("\t$(AFTER)\n")
	make_file.write("\trm -f %s\n"%(deps))
	
	#clean
	make_file.write("clean :\n")
	for t in subtargets:
		make_file.write("\tcd %s;make clean\n"%(t[0]))
	make_file.write("\trm -rf $(OBJDIR)/$(ARCH)\n")
	
	#delete
	make_file.write("delete :\n")
	for t in subtargets:
		make_file.write("\tcd %s;make delete\n"%(t[0]))
	make_file.write("\trm -rf $(OBJDIR)/$(ARCH)\n")
	make_file.write("\trm -rf $(OUTPUT)\n")
	
	#rebuild
	make_file.write("rebuild :\n")
	make_file.write("\tmake delete\n")
	make_file.write("\tmake all\n")
	
	make_file.close()
	os.chdir(old_path)
	return
	
def get_sources(arch):
	f = open("sources","r")
	ret = f.readlines()
	f.close()
	f = open("sources.%s"%(arch),"r")
	ret = ret + f.readlines()
	f.close()
	for i in range(0,len(ret)):
		ret[i] = ret[i].split()[0]
		print("Checking source file \"%s\"..."%(ret[i]))
		if not os.access(ret[i],os.F_OK):
			raise FileNotFoundError("Source file \"%s\" missing."%(ret[i]))
	return ret
