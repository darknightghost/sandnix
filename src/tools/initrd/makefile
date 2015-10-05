AS = as
CC = gcc
LINKER = gcc 
ASFLAGS =
CCFLAGS = -O2
LDFLAGS = 

current_path = $(shell pwd)

root_path = $(current_path)/../../..

TARGET_DIR = $(root_path)/tools
TARGET = $(TARGET_DIR)/initrd

SOURCES = initrd.c

OBJS_DIR = $(root_path)/obj/tools/initrd

OBJS = $(foreach filename,\
		$(SOURCES),\
		$(OBJS_DIR)/$(basename $(filename)).o)
		
.PHONY : all clean delete rebuild

all : $(TARGET)

$(TARGET) : $(OBJS) $(TARGET_DIR)
	$(LINKER) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(OBJS) : $(SOURCES) $(OBJS_DIR)
	$(foreach filename,$(SOURCES),\
		$(if $(filter .c,$(suffix $(filename))),\
			$(shell \
			$(CC) $(CCFLAGS) -c -o $(OBJS_DIR)/$(basename $(filename)).o $(current_path)/$(filename))\
		)\
		$(if $(filter .s,$(suffix $(filename))),\
			$(shell \
			$(AS) $(ASFLAGS) -o $(OBJS_DIR)/$(basename $(filename)).o $(current_path)/$(filename))\
		)\
		$(if $(filter .S,$(suffix $(filename))),\
			$(shell \
			$(AS) $(ASFLAGS) -o $(OBJS_DIR)/$(basename $(filename)).o $(current_path)/$(filename))\
		)\
	)

$(TARGET_DIR) :
	mkdir -p $(TARGET_DIR)

$(OBJS_DIR) :
	$(foreach filename,\
		$(OBJS),\
		$(shell mkdir -p $(dir $(filename))))

clean :
	-rm -r $(OBJS_DIR)
	
delete :
	-rm -r $(OBJS_DIR)
	-rm -r $(TARGET_DIR)
	
rebuild : delete all

