
INCLUDEE =  ../include

.PHONY : all
all : main.o test.o

main.o : main.c ${INCLUDEE}/test.h
	$(GCC) $(CFLAG) main.c

test.o : test.c ${INCLUDEE}/test.h ${INCLUDEE}/put.h
	$(GCC) $(CFLAG) test.c