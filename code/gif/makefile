PROG = gif
include ../default
objects = gif.o

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(commonobjects) $(objects) -o  $(PROG) -lm

gif.o : gif.h ../io.h ../defs.h ../bitwise.h

clean :
	rm $(PROG) $(objects) $(commonobjects)