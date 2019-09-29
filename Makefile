BIN = ledcube
SRC = ledcube.c hd44780.c ui.c cube.c uart.c

MCU = atmega8
CFLAGS = -s -Os -Wall -Wextra -std=gnu99 -DF_CPU=$(F_CPU) -mmcu=$(MCU)
AVRDUDE_MCU = m8
AVRDUDE_PROG = usbasp

HFUSE = 0xc9

# 8 MHz internal osc
# F_CPU = 8000000
# LFUSE = 0xe4

# crystal
F_CPU = 16000000
LFUSE = 0xde

HEX = $(BIN).hex
OBJ = $(SRC:%.c=%.o)

.PHONY: all flash fuses clean

all: $(HEX)

$(BIN): $(OBJ)
	avr-gcc $(CFLAGS) -o $(BIN) $^

%.o: %.c
	avr-gcc $(CFLAGS) -c -o $@ $^

$(HEX): $(BIN)
	avr-objcopy -O ihex $(BIN) $(HEX)

flash: $(HEX)
	avrdude -p $(AVRDUDE_MCU) -c $(AVRDUDE_PROG) -eU flash:w:$(HEX)

fuses:
	avrdude -p $(AVRDUDE_MCU) -c $(AVRDUDE_PROG) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

clean:
	rm -f $(BIN) $(HEX) $(OBJ)
