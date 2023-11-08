.PHONY : all
all : put.o rand.o string.o

put.o : put.c ../include/put.h
	$(GCC) $(CFLAG) put.c

rand.o : rand.c ../include/rand.h
	$(GCC) $(CFLAG) rand.c

string.o : string.c ../include/string.h
	$(GCC) $(CFLAG) -O0 string.c
