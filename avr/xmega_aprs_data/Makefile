################################################################################
# AVR XMEGA example of APRS data packet sending using DAC and DMA
# by SO6AGJ
################################################################################

DEVICE = atxmega128a3u
CLOCK = 32000000
PROGRAMMER = -c avrisp2 -P usb -v
OBJECTS    = main.o 
SOURCES    = ./src/osc.c ./src/nvm.c ./src/dac.c ./src/data_aprs.c main.c 
#INCLUDE    = -Wl,-u,vfprintf -lprintf_flt -lm

######################################################################

# Tune the lines below only if you know what you are doing:
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) 
COMPILE = avr-gcc -Wall -Wextra -Wpedantic -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@

flash:
	$(AVRDUDE) -U flash:w:main.hex:i

clean:
	rm -f main.hex main.elf main.o

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(SOURCES) $(INCLUDE)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --mcu=$(DEVICE) --format=avr main.elf

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf
