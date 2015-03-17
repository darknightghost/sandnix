AS = as
CC = gcc
ASFLAGS =
CCFLAGS = -c

root_path = ../../../../..

TARGET_DIR = $(root_path)/bin/sanlo/x86
TARGET = $(TARGET_DIR)/sanlo_x86_loader

AS_SOURCES = loader.s

C_SOURCES = main.c

OBJS_DIR = $(root_path)/obj/sanlo/x86

OBJS = $(OBJS_DIR)/loader.o \
		$(OBJS_DIR)/main.o

.PHONY : all clean delete

all : $(TARGET)

$(TARGET) : $(OBJS)
	mkdir -p $(TARGET_DIR)
	ld --oformat binary -N -Ttext 0x0000 -o $@ $<

$(OBJS) : $(AS_SOURCES) $(C_SOURCES)
	mkdir -p $(OBJS_DIR)
	$(AS) $(ASFLAGS) -o $@ $<
	$(CC) $(CCFLAGS) -o $@ $<
	
clean :
	-rm $(OBJS)
	
delete :
	-rm $(OBJS)
	-rm $(TARGET)
