FIRMWARE=./firmware/bin/charliev2.elf
FSIMAGE=./filesys/out/fsimage.bin

all: $(FIRMWARE) $(FSIMAGE)

$(FIRMWARE):
	make -C ./firmware

$(FSIMAGE):
	make -C ./filesys

simulate: $(FIRMWARE) $(FSIMAGE)
	@cp -u $(FSIMAGE) ./simulation/fsimage.bin && \
	cd ./simulation && \
	sudo megas2 charliev2.msd

.PHONY: clean simulate

clean:
	make -C ./firmware clean
	make -C ./filesys clean
