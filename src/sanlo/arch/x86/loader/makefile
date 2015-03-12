AS = as
ASFLAGS =

root_path = ../../../../..

TARGET_DIR = (root_path)/bin/sanlo/x86

TARGET = $(TARGET_DIR)/sandix_kernel_loader_x86

SOURCES = loader.S

OBJS_DIR = $(root_path)/obj/sanlo/x86

OBJS = $(OBJS_DIR)/loader.o

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(OBJS)
	ld --oformat binary -N -Ttext 0x7c00 -o $@ $<

$(OBJS) : $(SOURCES)
	$(AS) $(ASFLAGS) -o $@ $<  
	
clean :
	rm $(OBJS)
