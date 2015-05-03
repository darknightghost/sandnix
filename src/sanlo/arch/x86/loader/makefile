AS = as
CC = gcc
ASFLAGS = --32
CCFLAGS = -std=c99 -m32 -nostdlib -fno-builtin -DVERSION=0x0000000001 -c

root_path = ../../../../..

TARGET_DIR = $(root_path)/bin/sanlo/x86
TARGET = $(TARGET_DIR)/sanlo_x86_loader
MAP = $(TARGET_DIR)/sanlo_x86_loader.map

SOURCES = init.s \
			main.c	\
			string/string.c \
			io/io.c \
			io/stdout.c \
			io/keyboard.c \
			memory/memory.c \
			exception/exception.c \
			interrupt/interrupt.c \
			interrupt/interrupt_routines.s \
			hdd/hdd.c \
			hdd/partition.c \
			fs/fs.c \
			fs/ext2.c \
			menu/menu.c \
			k_loader/k_loader.c \
			memtest/memtest.s

OBJS_DIR = $(root_path)/obj/sanlo/x86/loader

OBJS = $(foreach filename,\
		$(SOURCES),\
		$(OBJS_DIR)/$(basename $(filename)).o)
		
.PHONY : all clean delete rebuild

all : $(TARGET)

$(TARGET) : $(OBJS) $(TARGET_DIR)
	ld -m elf_i386 -Ttext 0x0000 -Map $(MAP) -o $(TARGET).elf $(OBJS)
	objcopy -R .note -R .comment -S -O binary $(TARGET).elf $(TARGET)
	rm $(TARGET).elf
$(OBJS) : $(SOURCES) $(OBJS_DIR)
	$(foreach filename,$(SOURCES),\
		$(if $(filter .c,$(suffix $(filename))),\
			$(shell \
			$(CC) $(CCFLAGS) -o $(OBJS_DIR)/$(basename $(filename)).o $(filename))\
		)\
		$(if $(filter .s,$(suffix $(filename))),\
			$(shell \
			$(AS) $(ASFLAGS) -o $(OBJS_DIR)/$(basename $(filename)).o $(filename))\
		)\
		$(if $(filter .S,$(suffix $(filename))),\
			$(shell \
			$(AS) $(ASFLAGS) -o $(OBJS_DIR)/$(basename $(filename)).o $(filename))\
		)\
	)
$(TARGET_DIR) :
	mkdir -p $(TARGET_DIR)
	
$(OBJS_DIR) :
	$(foreach filename,\
		$(OBJS),\
		$(shell mkdir -p $(dir $(filename))))

clean :
	-rm $(OBJS)
	
delete :
	-rm $(OBJS)
	-rm $(TARGET)
	
rebuild : delete all
