BIN = ./bin
SRC = ./src
INCLUDES = -I./src

MCU = atmega32
CC = avr-gcc

CFLAGS = -mmcu=$(MCU) -Wall -Wextra -Wuninitialized -std=c99 -DF_CPU=16000000UL -Os
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -mcall-prologues -mrelax -mstrict-X
CFLAGS += -flto -fwhole-program
LDFLAGS = -Wl,-gc-sections

HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

APP_FILES = $(wildcard $(patsubst %,$(SRC)/%,*.* */*.* */*/*.* */*/*/*.* */*/*/*/*.*)) $(SRC)/net/php_code.h
APP_SOURCES = $(filter %.c,$(APP_FILES))

all: charliev2

charliev2: $(BIN)/charliev2.elf $(BIN)/charliev2.hex checksize

$(BIN)/charliev2.elf: $(APP_FILES)
	@mkdir -p $(BIN) && $(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(APP_SOURCES)

$(SRC)/net/php_code.h: $(wildcard ../filesys/src/www/*.php)
	@../tools/compile_php.py ../filesys/src/www $@

%.hex: %.elf
	@avr-objcopy -O ihex $(HEX_FLASH_FLAGS) $< $@

checksize: $(BIN)/charliev2.elf
	@avr-size -B $^ | tail -n+2 | (read text data bss etc; \
	totFlash=$$((text+data)); totSram=$$((data+bss)); \
	percFlash=$$((100*totFlash/32768)); \
	percSram=$$((100*totSram/2048)); \
	echo "Flash: $$totFlash bytes ($$percFlash%)"; \
	echo "SRAM : $$totSram bytes ($$percSram%)"; \
	)

program: $(BIN)/charliev2.hex
	avrdude -c usbtiny -p m32 -B 1 -U flash:w:$^

.PHONY: clean checksize program

clean:
	rm -rf $(BIN)/*
