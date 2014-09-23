TARGET=atmega88
ISP=usbasp

CC=avr-gcc
CFLAGS=-Wall -Wextra -Os -std=c99 -I. -mmcu=$(TARGET) -DF_CPU=8000000L

# # clkdiv 8
# HFUSE=0xdf
# LFUSE=0x62
# EFUSE=0x01

# clkdiv 1
HFUSE=0xDF
LFUSE=0xE2
EFUSE=0x01

.PHONY: all
all: main.hex

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *.bin *.hex *.map

main.bin: main.o
	$(CC) $(CFLAGS) -o main.bin main.o

main.hex: main.bin
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex

.PHONY: disasm
disasm:	main.bin
	avr-objdump -d main.bin

.PHONY: flash
flash: main.hex
	avrdude -c ${ISP} -p ${TARGET} -U flash:w:main.hex

.PHONY: setfuse
setfuse:
	avrdude -c ${ISP} -p ${TARGET} -u -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m -U efuse:w:$(EFUSE):m

.PHONY: readfuse
readfuse:
	avrdude -c ${ISP} -p ${TARGET} -u -U hfuse:r:-:h -U lfuse:r:-:h -U efuse:r:-:h

.PHONY: avrdude
avrdude:
	avrdude -c ${ISP} -p ${TARGET} -v

.PHONY: tags
tags:
	ctags -eR
