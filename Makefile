# its a must to have kernel.asm.o as the first file so it will be linked first and when we jmp to kernel code it will be the first to run
# in linker.ld we decided that we will start loading at 1MB (address 0x100000), since we are linking kernel.asm.o as the first object file,
# kernel.asm.o will be loaded at address 0x100000
FILES = ./build/kernel.asm.o ./build/kernel.o ./build/idt/idt.asm.o ./build/memory/memory.o ./build/idt/idt.o

# change the include dir to ./src
INCLUDES = -I./src

FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

# when all command is run we depend on ./bin/boot.bin: and ./bin/kernel.bin: so it goes through all the labels and resolve them then executes the commands under 'all' label
# remove ./bin/os.bin at each run since we are appending the bootloader (./bin/boot.bin) and the kernel
# after appending the kernel, append zero to get it to a sector (sectos size is 512 bytes) append 100 sectors just to be on the safe side
# so we dont need to calculate kernel_size/512 to know how much sectors the kernel is and append zeros with the same number in order to make sure that all the kernel is loaded - this way we make sure that the whole kernel will be loaded
all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin	
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(FILES)
# use the linker to link all the object files into one ./build/kernelfull.o
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
# tell GCC to use linker.ld script
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o


./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
# invoke the C compiler
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
# invoke the C compiler
	i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/memory/memory.c
# invoke the C compiler
	i686-elf-gcc $(INCLUDES) -I./src/memory $(FLAGS) -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ${FIILES}
	rm -rf ./build/kernelfull.o