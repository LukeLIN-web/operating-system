export
CROSS_= riscv64-unknown-elf-
AR=${CROSS_}ar
GCC=${CROSS_}gcc
LD=${CROSS_}ld
OBJCOPY=${CROSS_}objcopy

ISA ?= rv64imafd
ABI ?= lp64

INCLUDE = -I ../include
CF = -g -O3 -march=$(ISA) -mabi=$(ABI) -mcmodel=medany -ffunction-sections \
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
	rm vmlinux init/main.o init/test.o ${KERNEL}/head.o ${RISCV}/boot/Image lib/put.o lib/rand.o ${KERNEL}/entry.o ${KERNEL}/strap.o System.map ${KERNEL}/sched.o ${KERNEL}/vm.o 
