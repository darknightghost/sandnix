AS = as
CC = gcc
ASFLAGS =
CCFLAGS = 

root_path = ../../../../..

TARGET_DIR = $(root_path)/bin/sanlo/x86
TARGET = $(TARGET_DIR)/sanlo_x86_loader

AS_SOURCES = loader.S
OBJS_DIR = $(root_path)/obj/sanlo/x86
OBJS = $(OBJS_DIR)/loader.o

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(OBJS)
	mkdir -p $(TARGET_DIR)
	ld --oformat binary -N -Ttext 0x0100 -o $@ $<

$(OBJS) : $(AS_SOURCES)
	mkdir -p $(OBJS_DIR)
	$(AS) $(ASFLAGS) -o $@ $<  
	
clean :
	rm $(OBJS)
