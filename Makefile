NAME = main
OBJECTS = textures.o raycaster.o world.o

# Edit to point to CS107E folder (needed for .h files)
CS107E=../cs107e.github.io/cs107e
CFLAGS  = -I$(CS107E)/include -g -Wall -Wpointer-arith 
CFLAGS += -Og -std=c99 -ffreestanding 
CFLAGS += -mapcs-frame -fno-omit-frame-pointer -mpoke-function-name
LDFLAGS = -nostdlib -T memmap -L. -L$(CS107E)/lib 
LDLIBS  = -lmypi -lc_pi -lpi -lc_pi -lgcc -lpiextra

all : $(NAME).bin

%.bin: %.elf
	arm-none-eabi-objcopy $< -O binary $@

%.elf: %.o $(OBJECTS) start.o cstart.o
	arm-none-eabi-gcc $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c $< -o $@

%.o: %.s
	arm-none-eabi-as $(ASFLAGS) $< -o $@

%.list: %.o
	arm-none-eabi-objdump  --no-show-raw-insn -d $< > $@

install: $(NAME).bin
	rpi-install.py -p $<

clean:
	rm -f *.o *.bin *.elf *.list *~

.PHONY: all clean install

.PRECIOUS: %.o %.elf

%:%.o
%:%.c