TARGET=edifier
MCU=atmega8
SOURCES=main.c irsnd/irsnd.c

PROGRAMMER=usbasp
F_CPU=4000000UL
#auskommentieren f�r automatische Wahl
#PORT=-P/dev/ttyS0
#BAUD=-B115200

#Ab hier nichts ver�ndern
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-c -Os -DF_CPU=$(F_CPU)
LDFLAGS=

all: hex eeprom

hex: $(TARGET).hex

eeprom: $(TARGET)_eeprom.hex

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -j .data -j .text $(TARGET).elf $(TARGET).hex

$(TARGET)_eeprom.hex: $(TARGET).elf
	avr-objcopy -O ihex -j .eeprom --change-section-lma .eeprom=1 $(TARGET).elf $(TARGET)_eeprom.hex

$(TARGET).elf: $(OBJECTS)
	avr-gcc $(LDFLAGS) -mmcu=$(MCU) $(OBJECTS) -o $(TARGET).elf

.c.o:
	avr-gcc $(CFLAGS) -mmcu=$(MCU) $< -o $@

size:
	avr-size --mcu=$(MCU) -C $(TARGET).elf

program: all
	avrdude -p$(MCU) $(PORT) $(BAUD) -c$(PROGRAMMER) -Uflash:w:$(TARGET).hex:a

fuse:
# 	avrdude -p$(MCU) $(PORT) -c$(PROGRAMMER) -U lfuse:w:0xe4:m -U hfuse:w:0xd9:m
# 	avrdude -p$(MCU) $(PORT) -c$(PROGRAMMER) -U lfuse:w:0xa4:m -U hfuse:w:0xd9:m
# 	avrdude -p$(MCU) $(PORT) -c$(PROGRAMMER) -U lfuse:w:0xc3:m -U hfuse:w:0xd9:m # 4mhz 0ms startup
	avrdude -p$(MCU) $(PORT) -c$(PROGRAMMER) -U lfuse:w:0x83:m -U hfuse:w:0xd9:m # 8mhz 0ms startup brown out

clean_tmp:
	rm -rf *.o
	rm -rf *.elf

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.hex