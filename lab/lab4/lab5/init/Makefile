.PHONY : all
all : main.o test.o

main.o : main.c ../include/test.h
	$(GCC) $(CFLAG) main.c

test.o : test.c ../include/test.h ../include/put.h
	$(GCC) $(CFLAG) test.c