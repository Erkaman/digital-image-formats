PROG = gif
include ../default
objects += gif.o

$(PROG) : $(objects)
	$(CC) $(objects) -o  $(PROG) -lm

gif.o : gif.h ../common.h

clean :
	rm $(PROG) $(objects)