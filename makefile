
CC = i686-deltaos-gcc
LD = i686-deltaos-ld
CFLAGS = -ffreestanding -fno-stack-protector -fno-omit-frame-pointer -Wall -g
LDFLAGS =

all: kernel

kernel: boot.asm deltaos.h main.c acpi.c apic.c console.c screen.c gdt.c ide.c switch.asm ext2.c keyboard.c memory.c namei.c pit.c printk.c proc.c serial.c string.c syscall.c
	nasm -f elf32 boot.asm -o boot.o
	nasm -f elf32 switch.asm -o switch.o
	${CC} ${CFLAGS} -c main.c -o main.o
	${CC} ${CFLAGS} -c apic.c -o apic.o
	${CC} ${CFLAGS} -c acpi.c -o acpi.o
	${CC} ${CFLAGS} -c console.c -o console.o
	${CC} ${CFLAGS} -c screen.c -o screen.o
	${CC} ${CFLAGS} -c ext2.c -o ext2.o
	${CC} ${CFLAGS} -c ide.c -o ide.o
	${CC} ${CFLAGS} -c keyboard.c -o keyboard.o
	${CC} ${CFLAGS} -c gdt.c -o gdt.o
	${CC} ${CFLAGS} -c memory.c -o memory.o
	${CC} ${CFLAGS} -c namei.c -o namei.o
	${CC} ${CFLAGS} -c pit.c -o pit.o
	${CC} ${CFLAGS} -c proc.c -o proc.o
	${CC} ${CFLAGS} -c printk.c -o printk.o
	${CC} ${CFLAGS} -c serial.c -o serial.o
	${CC} ${CFLAGS} -c string.c -o string.o
	${CC} ${CFLAGS} -c syscall.c -o syscall.o
	${LD} ${LDFLAGS} -T kernel.ld -o kernel main.o boot.o acpi.o apic.o console.o screen.o ext2.o ide.o keyboard.o gdt.o memory.o namei.o pit.o proc.o printk.o serial.o string.o switch.o syscall.o
	cp kernel sysroot/target/boot


user: usys.asm sbrk.c strstrip.c crt0.c a.c b.c dir.c stat.c cat.c cd.c init.c
	nasm -f elf32 usys.asm
	${CC} -c -Isysroot/i686-deltaos/include -g sbrk.c
	${CC} -c -Isysroot/i686-deltaos/include -g strstrip.c
	${CC} -c -Isysroot/i686-deltaos/include crt0.c
	${CC} -c -Isysroot/i686-deltaos/include -g a.c
	${CC} -c -Isysroot/i686-deltaos/include -g b.c
	${CC} -c -Isysroot/i686-deltaos/include -g dir.c
	${CC} -c -Isysroot/i686-deltaos/include -g stat.c
	${CC} -c -Isysroot/i686-deltaos/include -g cat.c
	${CC} -c -Isysroot/i686-deltaos/include -g cd.c
	${CC} -c -Isysroot/i686-deltaos/include -g init.c
	${CC} -c -Isysroot/i686-deltaos/include sbrk.c
	${LD} -Lsysroot/i686-deltaos/lib crt0.o a.o -lc usys.o sbrk.o -o sysroot/target/bin/a
	${LD} -Lsysroot/i686-deltaos/lib crt0.o b.o -lc usys.o sbrk.o -o sysroot/target/bin/b
	${LD} -Lsysroot/i686-deltaos/lib crt0.o dir.o -lc usys.o sbrk.o -o sysroot/target/bin/dir
	${LD} -Lsysroot/i686-deltaos/lib crt0.o stat.o -lc usys.o sbrk.o -o sysroot/target/bin/stat
	${LD} -Lsysroot/i686-deltaos/lib crt0.o cat.o -lc usys.o sbrk.o -o sysroot/target/bin/cat
	${LD} -Lsysroot/i686-deltaos/lib crt0.o cd.o -lc usys.o sbrk.o -o sysroot/target/bin/cd
	${LD} -Lsysroot/i686-deltaos/lib crt0.o init.o -lc usys.o sbrk.o strstrip.o -o sysroot/target/bin/init

qemu: kernel user
	qemu-system-i386 -name deltaos -kernel kernel -display gtk,zoom-to-fit=on -hda disk.img -no-reboot

clean:
	rm -f *.o *.out kernel init
