PROG = rle
include ../default
objects = main.o ../io.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h ../io.h ../defs.h
../io.o : ../io.h ../defs.h

clean :
	rm $(PROG) $(objects)