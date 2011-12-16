PROG = test
include ../default
objects += test.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

test.o : test.c ../common.h

clean :
	rm $(PROG) $(objects)