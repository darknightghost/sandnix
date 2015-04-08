AS = as
CC = gcc
ASFLAGS = --32
CCFLAGS = -m32 -c

root_path = ../../../../..

TARGET_DIR = $(root_path)/bin/sanlo/x86
TARGET = $(TARGET_DIR)/sanlo_x86_boot

SOURCES = boot.s

OBJS_DIR = $(root_path)/obj/sanlo/x86/boot

OBJS = $(foreach filename,\
		$(SOURCES),\
		$(OBJS_DIR)/$(basename $(filename)).o)
		
.PHONY : all clean delete rebuild

all : $(TARGET)

$(TARGET) : $(OBJS) $(TARGET_DIR)
	ld --oformat binary -m elf_i386 -N -Ttext 0x0000 -o $(TARGET) $(OBJS)

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
