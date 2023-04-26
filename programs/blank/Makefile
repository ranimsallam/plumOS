all:
	nasm -f elf ./blank.asm -o ./build/blank.o
# link object file - create blank.bin file that will be loaded and run as user programs
	i686-elf-gcc -g -T ./linker.ld -o ./blank.elf -ffreestanding -O0 -nostdlib -fpic -g ./build/blank.o

clean:
		rm -rf ./build/blank.o