
INCLUDEE =  ../include

.PHONY : all
all : head.o entry.o strap.o syscall.o sched.o vm.o slub.o buddy.o fault.o

head.o : head.S 
	$(GCC) $(CFLAG) head.S

entry.o : entry.S
	$(GCC) $(CFLAG) entry.S

strap.o : strap.c ${INCLUDEE}/put.h ${INCLUDEE}/syscall.h
	$(GCC) $(CFLAG) strap.c

sched.o : sched.c ${INCLUDEE}/sched.h ${INCLUDEE}/rand.h
	$(GCC) $(CFLAG) sched.c

vm.o : vm.c ${INCLUDEE}/vm.h ${INCLUDEE}/put.h
	$(GCC) $(CFLAG) vm.c

slub.o : slub.c ${INCLUDEE}/slub.h ${INCLUDEE}/string.h
	$(GCC) $(CFLAG) slub.c

buddy.o : buddy.c ${INCLUDEE}/buddy.h
	$(GCC) $(CFLAG) buddy.c

syscall.o : syscall.c ${INCLUDEE}/syscall.h
	$(GCC) $(CFLAG) syscall.c

fault.o : fault.c ${INCLUDEE}/fault.h
	$(GCC) $(CFLAG) fault.c
