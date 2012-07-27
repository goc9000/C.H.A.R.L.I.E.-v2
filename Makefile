FIRMWARE=./firmware/bin/charliev2.elf
FSIMAGE=./filesys/out/fsimage.bin

all: $(FIRMWARE) $(FSIMAGE)

$(FIRMWARE):
	make -C ./firmware

$(FSIMAGE):
	make -C ./filesys

.PHONY: clean

clean:
	make -C ./firmware clean
	make -C ./filesys clean
