PROG = tga
include ../default
objects = main.o ../io.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

../io.o : ../io.h ../defs.h
main.o : main.h ../io.h

clean :
	rm $(PROG) $(objects)