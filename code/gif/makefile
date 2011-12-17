PROG = gif
include ../default
objects = gif.o ../io.o ../bitwise.o

$(PROG) : $(objects)
	$(CC) $(objects) -o  $(PROG) -lm

gif.o : gif.h ../io.h ../defs.h ../bitwise.h
../io.o : ../io.h ../defs.h
../bitwise.o : ../bitwise.h ../defs.h

clean :
	rm $(PROG) $(objects)