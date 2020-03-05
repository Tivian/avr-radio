avr-gcc -g -Os -mmcu=atmega8 -DF_CPU=8000000UL -c *.cpp -std=c++17
avr-gcc -g -mmcu=atmega8 -o radio.elf *.o
avr-objcopy -j .text -j .data -O ihex radio.elf radio.hex
avr-size radio.hex radio.elf
