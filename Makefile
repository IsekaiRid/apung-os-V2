# 1. Deteksi semua file sumber
C_SOURCES = $(wildcard kernel/*.c driver/*.c cpu/*.c libc/*c)
HEADERS = $(wildcard kernel/*.h driver/*.h cpu/*.h libc/*h)
# Mengubah .c menjadi .o
OBJ = ${C_SOURCES:.c=.o}

# 2. Tambahkan file assembly tambahan ke daftar object
# Pastikan cpu/interrupt.asm sudah Anda buat
OBJ += cpu/interrupt.o cpu/paging_asm.o

CC = gcc
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -c
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary -no-pie

# Target Utama
os-image.bin: boot/boot.bin kernel.bin
	cat $^ > $@

# Linker - kernel_entry.o HARUS paling depan
kernel.bin: boot/kernel_entry.o ${OBJ}
	ld $(LDFLAGS) -o $@ $^

# Rule Kompilasi untuk file C (Bergantung pada Headers)
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} $< -o $@

# Rule Kompilasi untuk file Assembly (kernel_entry & interrupt)
%.o: %.asm
	nasm $< -f elf -o $@

boot/boot.bin: boot/boot.asm
	nasm $< -f bin -o $@

run: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin

clean:
	rm -rf kernel/*.o driver/*.o cpu/*.o boot/*.o boot/*.bin *.bin os-image.bin