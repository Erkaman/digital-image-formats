PROG = png
include ../default
objects += main.o  png.o

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h png.h
png.o :  png.h ../common.h

clean :
	rm $(PROG) $(objects)