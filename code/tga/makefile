PROG = tga.exe
CFLAGS = -Wall -c
objects = tga.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h

clean :
	rm $(PROG) $(objects)