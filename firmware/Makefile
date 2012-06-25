CFLAGS   := -Os -g3 -gdwarf-2  -mcpu=cortex-m3 -mthumb -march=armv7-m \
		   -nostdlib -ffunction-sections -fdata-sections	     \
		   -Wl,--gc-sections -DBOARD_safecast -DMCU_STM32F101RE -DSTM32_HIGH_DENSITY -DVECT_TAB_BASE -I. -I./libmaple -I./display
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -Wall 
ASFLAGS  := -mcpu=cortex-m3 -march=armv7-m -mthumb		     \
		   -x assembler-with-cpp 
LDFLAGS  = -mcpu=cortex-m3 -mthumb -Xlinker     \
            --gc-sections --print-gc-sections --march=armv7-m -Wall

CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++

LIBMAPLEOBJS = libmaple/spi.o libmaple/rcc.o libmaple/timer.o libmaple/adc.o libmaple/util.o libmaple/syscalls.o libmaple/usart.o libmaple/gpio.o libmaple/i2c.o libmaple/systick.o libmaple/nvic.o libmaple/exti.o libmaple/flash.o

OUROBJS = ./display/spi_aux.o ./display/oled.o ./power.o ./wirish_boards.o ./safecast_config.o ./main.o

firmware: main.cpp $(LIBMAPLEOBJS) $(OUROBJS) to565
	convert safemed128.png -depth 8 RGB:image_data888
	./to565 < image_data888 > image_data
	arm-none-eabi-ld -r -b binary -o image_data.o image_data
	# Final linker step
	arm-none-eabi-gcc -T./linker_files/jtag.ld -L./linker_files -mcpu=cortex-m3 -mthumb -Xlinker --gc-sections --print-gc-sections --march=armv7-m -Wall $(OUROBJS) $(LIBMAPLEOBJS) image_data.o -o safecast.elf -Wl,-Map,safecast.map 
	# Convert elf to flat binary for programming.
	arm-none-eabi-objcopy -v -Obinary safecast.elf safecast.bin

to565: to565.c
	gcc to565.c -o to565

clean:
	rm safecast.bin safecast.elf safecast.map $(LIBMAPLEOBJS) $(OUROBJS)