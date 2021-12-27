
INCLUDEE =  ../include


.PHONY : all
all : head.o entry.o strap.o sched.o vm.o

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
