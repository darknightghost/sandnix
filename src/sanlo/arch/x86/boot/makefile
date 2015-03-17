AS = as
ASFLAGS =

root_path = ../../../../..

TARGET_DIR = $(root_path)/bin/sanlo/x86
TARGET = $(TARGET_DIR)/sanlo_x86_boot

SOURCES = boot.s
OBJS_DIR = $(root_path)/obj/sanlo/x86
OBJS = $(OBJS_DIR)/boot.o

.PHONY : all clean delete

all : $(TARGET)

$(TARGET) : $(OBJS)
	mkdir -p $(TARGET_DIR)
	ld --oformat binary -N -Ttext 0x7c00 -o $@ $<

$(OBJS) : $(SOURCES)
	mkdir -p $(OBJS_DIR)
	$(AS) $(ASFLAGS) -o $@ $<  
	
clean :
	-rm $(OBJS)
	
delete :
	-rm $(OBJS)
	-rm $(TARGET)
