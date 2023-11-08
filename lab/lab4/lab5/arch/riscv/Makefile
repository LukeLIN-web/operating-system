
VMLINUX = ../../vmlinux
INIT = ../../init
LIB = ../../lib
SYSTEMMAP = ../../System.map

.PHONY : all
all : ${VMLINUX} boot/image ${SYSTEMMAP}

${VMLINUX} : ${INIT}/main.o ${INIT}/test.o kernel/head.o kernel/entry.o kernel/strap.o ${LIB}/put.o ${LIB}/rand.o kernel/sched.o kernel/vm.o
	$(LD) ${INIT}/main.o ${INIT}/test.o kernel/head.o kernel/entry.o kernel/strap.o kernel/sched.o kernel/vm.o ${LIB}/put.o ${LIB}/rand.o -T kernel/vmlinux.lds -o ${VMLINUX}

boot/image : ${VMLINUX}
	$(OBJCOPY) -O binary ${VMLINUX} boot/Image --strip-all

${SYSTEMMAP} : ${VMLINUX}
	nm ${VMLINUX} > ${SYSTEMMAP}