#! /usr/bin/python

import sys
import os

g_target=("x86",);

def show_help():
	global g_target

	print("Usage:\n\t./build.py build|clean|rebuild all|targets\n\t./build.py help")
	print("Targets:")
	for t in g_target:
		print("\t%s"%t)
	return

def build(argv):
	global g_target

	if argv[0] == "all":
		for t in g_target:
			if os.system("make -C ./arch/%s all"%t)!=0:
				print("Fail to build target \"%s\"."%t)
				return False
	else:
		for t in argv:
			if not t in g_target:
				print("Unknow target:\"%s\""%t)
				return False
			if os.system("make -C ./arch/%s all"%t)!=0:
				print("Fail to build target:\"%s\"."%t)
				return False
	return True
	
def clean(argv):
	global g_target

	if argv[0] == "all":
		for t in g_target:
			if os.system("make -C ./arch/%s clean"%t)!=0:
				print("Fail to build target \"%s\"."%t)
				return False
	else:
		for t in argv:
			if not t in g_target:
				print("Unknow target:\"%s\""%t)
				return False
			if os.system("make -C ./arch/%s clean"%t)!=0:
				print("Fail to build target:\"%s\"."%t)
				return False
	return True
	
def rebuild(argv):
	global g_target

	if argv[0] == "all":
		for t in g_target:
			if os.system("make -C ./arch/%s rebuild"%t)!=0:
				print("Fail to build target \"%s\"."%t)
				return False
	else:
		for t in argv:
			if not t in g_target:
				print("Unknow target:\"%s\""%t)
				return False
			if os.system("make -C ./arch/%s rebuild"%t)!=0:
				print("Fail to build target:\"%s\"."%t)
				return False
	return True

def main(argv):

	os.chdir(sys.path[0])

	if len(argv) < 3:
		show_help()
		if len(argv) < 2:
			return -1
		if argv[1] != "help":
			return -1;
	elif argv[1] == "build":
		if not build(argv[2 : ]):
			return -1
	elif argv[1] == "clean":
		if not clean(argv[2 : ]):
			return -1
	elif argv[1] == "rebuild":
		if not rebuild(argv[2 : ]):
			return -1
	else:
		show_help()
		return -1;
	return 0

ret = main(sys.argv)

exit(ret)
