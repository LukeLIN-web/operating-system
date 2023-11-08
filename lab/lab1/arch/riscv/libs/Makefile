# YOUR MAKEFILE CODE
C_SRC       = $(sort $(wildcard *.c))
OBJ		    = $(patsubst %.c,%.o,$(C_SRC))

all : $(OBJ)

%.o:%.c
	${CC}  ${CFLAG}  -c $<

clean:
	$(shell rm *.o 2>/dev/null)

