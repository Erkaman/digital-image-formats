PROG = gif
include ../default
objects = gif.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

gif.o : gif.c

clean :
	rm $(PROG) $(objects)