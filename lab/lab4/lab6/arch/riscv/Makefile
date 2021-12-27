
VMLINUX = ../../vmlinux
INIT = ../../init
SYSTEMMAP = ../../System.map
LIB = ../../lib

.PHONY : all
all : ${VMLINUX} boot/image ${SYSTEMMAP}

${VMLINUX} : ${INIT}/main.o ${INIT}/test.o kernel/head.o kernel/entry.o kernel/strap.o ${LIB}/put.o ${LIB}/rand.o kernel/sched.o kernel/vm.o kernel/slub.o kernel/buddy.o ${LIB}/string.o kernel/syscall.o kernel/fault.o
	$(LD) ${INIT}/main.o ${INIT}/test.o kernel/head.o kernel/entry.o kernel/strap.o kernel/sched.o kernel/vm.o kernel/slub.o kernel/buddy.o kernel/syscall.o kernel/fault.o ${LIB}/put.o ${LIB}/rand.o ${LIB}/string.o -T kernel/vmlinux.lds -o ${VMLINUX}

boot/image : ${VMLINUX}
	$(OBJCOPY) -O binary ${VMLINUX} boot/Image --strip-all

${SYSTEMMAP} : ${VMLINUX}
	nm ${VMLINUX} > ${SYSTEMMAP}