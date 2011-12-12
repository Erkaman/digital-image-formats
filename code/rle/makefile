PROG = rle
include ../default
objects += main.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h ../common.h

clean :
	rm $(PROG) $(objects)