PROG = rle
include ../default
objects = main.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)