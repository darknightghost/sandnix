AS = as
ASFLAGS =

root_path = ../../../../..

TARGET = $(root_path)/bin/sanlo/x86/sanlo_x86

SOURCES = boot.S

OBJS = $(root_path)/obj/sanlo/x86/boot.o

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(OBJS)
	ld --oformat binary -N -Ttext 0x7c00 -o $@ $<

$(OBJS) : $(SOURCES)
	$(AS) $(ASFLAGS) -o $@ $<  
	
clean :
	rm $(OBJS)
