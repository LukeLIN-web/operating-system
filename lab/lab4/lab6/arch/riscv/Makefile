.PHONY : all
all : ../../vmlinux boot/image ../../System.map

../../vmlinux : ../../init/main.o ../../init/test.o kernel/head.o kernel/entry.o kernel/strap.o ../../lib/put.o ../../lib/rand.o kernel/sched.o kernel/vm.o kernel/slub.o kernel/buddy.o ../../lib/string.o kernel/syscall.o kernel/fault.o
	$(LD) ../../init/main.o ../../init/test.o kernel/head.o kernel/entry.o kernel/strap.o kernel/sched.o kernel/vm.o kernel/slub.o kernel/buddy.o kernel/syscall.o kernel/fault.o ../../lib/put.o ../../lib/rand.o ../../lib/string.o -T kernel/vmlinux.lds -o ../../vmlinux

boot/image : ../../vmlinux
	$(OBJCOPY) -O binary ../../vmlinux boot/Image --strip-all

../../System.map : ../../vmlinux
	nm ../../vmlinux > ../../System.map