avr-gcc -g -Os -mmcu=atmega8 -DF_CPU=8000000LU -c *.cpp -std=c++11
avr-gcc -g -mmcu=atmega8 -o radio.elf *.o
avr-objcopy -j .text -j .data -O ihex radio.elf radio.hex
avrdude -cusbasp -pm8 -Uflash:w:radio.hex:i