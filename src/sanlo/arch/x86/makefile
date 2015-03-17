
root_path = ../../../..

TARGET_DIR = $(root_path)/bin/sanlo/x86
TARGET = $(TARGET_DIR)/sanlo_x86.img

BOOT_SECTOR = $(TARGET_DIR)/sanlo_x86_boot

KERNEL_LOADER = $(TARGET_DIR)/sanlo_x86_loader

BINARYS = $(BOOT_SECTOR) \
			$(KERNEL_LOADER)

IMG = floppy.img

.PHONY : all clean delete

all : $(TARGET)

$(TARGET) : $(BINARYS)
	mkdir -p $(TARGET_DIR)
	cp $(IMG) $(TARGET)
	dd if=$(BOOT_SECTOR) of=$(TARGET) bs=512 count=1 conv=notrunc
	dd if=$(KERNEL_LOADER) of=$(TARGET) bs=512 seek=1 count=8 conv=notrunc
$(BINARYS):
	$(MAKE) -C boot all
	$(MAKE) -C loader all

clean :
	$(MAKE) -C boot clean
	$(MAKE) -C loader clean
	-rm $(BINARYS)

delete :
	$(MAKE) -C boot delete
	$(MAKE) -C loader delete
	-rm $(TARGET)
