# ProximityLED Makefile
OUTPUT = proximityled
SRC = main.c
MCU = attiny13a
FORMAT = ihex
OBJDIR = obj
GCC_FLAGS = -fpack-struct -fshort-enums -Wall -Wstrict-prototypes -std=gnu99

# time for work
avr-gcc $(GCC_FLAGS) -mint8 -mtiny-stack -mmcu=$(MCU) -Os -o=$(OUTPUT) $(SRC)