@echo off
avr-gcc -g -Os -mmcu=atmega8 -D F_CPU=8000000UL -D VOLTAGE=3300 -c audio.c i2c_master.c  lcd.c main.c menu.c nec.c radio.c save.c timeout.c ui.c &&^
avr-gcc -g -mmcu=atmega8 -o main.elf audio.o i2c_master.o lcd.o main.o menu.o nec.o radio.o save.o timeout.o ui.o &&^
avr-objcopy -j .text -j .data -O ihex main.elf main.hex &&^
avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex main.elf main_eeprom.hex &&^
avrdude -c usbasp -p m8 -U flash:w:main.hex:i &&^
avrdude -c usbasp -p m8 -U eeprom:w:main_eeprom.hex:i
REM nm main.elf -S --size-sort -t d
avr-size main.hex
echo Compiled on %DATE% %TIME%
