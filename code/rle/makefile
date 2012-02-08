PROG = rle
include ../default
objects = main.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.cpp ../io.h

clean :
	rm $(PROG) $(objects)