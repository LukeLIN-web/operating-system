export
CROSS_= riscv64-unknown-elf-
AR=${CROSS_}ar
GCC=${CROSS_}gcc
LD=${CROSS_}ld
OBJCOPY=${CROSS_}objcopy

ISA ?= rv64imafd
ABI ?= lp64

INCLUDE = -I ../include
CF = -g -O1 -march=$(ISA) -mabi=$(ABI) -mcmodel=medany -ffunction-sections \
		-fdata-sections -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections
CFLAG = ${CF} ${INCLUDE} -c -DSJF

RISCV = arch/riscv/
INIT = init/
KERNEL = arch/riscv/kernel/
LIB = lib/

.PHONY : all ${LIB} ${INIT} ${KERNEL} ${RISCV} 
all : ${LIB} ${INIT} ${KERNEL} ${RISCV}

${LIB} : 
	make -C ${LIB}
${INIT} :
	make -C ${INIT}
${KERNEL} :
	make -C ${KERNEL}
${RISCV} :
	make -C ${RISCV}

run : 
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux

debug : 
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -S -s

run-hello:
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin

debug-hello:
	qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin -S -s 

.PHONY : clean
clean :
	rm vmlinux init/main.o init/test.o arch/riscv/kernel/head.o arch/riscv/boot/Image lib/put.o lib/rand.o arch/riscv/kernel/entry.o arch/riscv/kernel/strap.o System.map arch/riscv/kernel/sched.o arch/riscv/kernel/vm.o lib/string.o arch/riscv/kernel/fault.o arch/riscv/kernel/syscall.o arch/riscv/kernel/buddy.o arch/riscv/kernel/slub.o
