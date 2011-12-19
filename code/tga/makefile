PROG = tga
include ../default
objects = main.o

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC)  $(objects) $(commonobjects)  -o $(PROG)

main.o : main.h ../io.h

clean :
	rm $(PROG) $(objects)