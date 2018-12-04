@REM ghetto win32 "makefile" for proximityled
@echo Compiling...
avr-gcc -std=gnu99 -fpack-struct -mint8 -mtiny-stack -mmcu=attiny2313a -Os -DF_CPU=8000000UL main.c usitwislave.c -o binary.elf

@REM create hex file
@echo Creating hex file...
avr-objcopy -j .text -j .data -O ihex binary.elf i2ctest.hex