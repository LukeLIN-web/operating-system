.PHONY : all
all : head.o entry.o strap.o syscall.o sched.o vm.o slub.o buddy.o fault.o

head.o : head.S 
	$(GCC) $(CFLAG) head.S

entry.o : entry.S
	$(GCC) $(CFLAG) entry.S

strap.o : strap.c ../include/put.h ../include/syscall.h
	$(GCC) $(CFLAG) strap.c

sched.o : sched.c ../include/sched.h ../include/rand.h
	$(GCC) $(CFLAG) sched.c

vm.o : vm.c ../include/vm.h ../include/put.h
	$(GCC) $(CFLAG) vm.c

slub.o : slub.c ../include/slub.h ../include/string.h
	$(GCC) $(CFLAG) slub.c

buddy.o : buddy.c ../include/buddy.h
	$(GCC) $(CFLAG) buddy.c

syscall.o : syscall.c ../include/syscall.h
	$(GCC) $(CFLAG) syscall.c

fault.o : fault.c ../include/fault.h
	$(GCC) $(CFLAG) fault.c
