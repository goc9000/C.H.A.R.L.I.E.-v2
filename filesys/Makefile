all: ./out/fsimage.bin

./out/fsimage.bin: ./out/contents
	@../tools/mkrawfsimg.py $^ $@

uploadfs: ./out/fsimage.bin
	@echo -n 'Enter the SD card device (e.g. /dev/sdb) to burn the file system to. ' && \
	echo 'MAKE SURE YOU GET IT RIGHT!' && \
	echo -n 'Device (leave blank to abort): ' && \
	read MMC_DRIVE && \
	if [ -n "$$MMC_DRIVE" ]; then \
	sudo dd if=$^ of=$$(MMC_DRIVE) bs=512 && \
	sudo sync && \
	sudo eject $$(MMC_DRIVE); fi

./out/contents:
	@cd ./src/graphics && make
	@mkdir -p $@/data
	@touch ./out/contents/data/log.bin $@/data/records.bin
	@mkdir -p $@/sys
	@cp ./src/def_config.bin $@/sys/config.bin
	@touch $@/sys/wrtest.bin
	@mkdir -p $@/wwwroot/img
	@cp ./src/graphics/out/sprites.png $@/wwwroot/img/sprites.png
	@cp ./src/graphics/out/favicon.ico $@/wwwroot/favicon.ico
	@cp ./src/graphics/out/favicon.png $@/wwwroot/favicon.png
	@cp `find . -wholename './src/www/*' -type f '!' -name '*.php'` $@/wwwroot

clean:
	@rm -rf ./out
	@cd ./src/graphics && make clean

.PHONY: uploadfs clean
