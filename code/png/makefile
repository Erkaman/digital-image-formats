PROG = png
include ../default
objects += main.o  png.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG) -lm

main.o : main.h png.h
png.o :  png.h ../common.h

clean :
	rm $(PROG) $(objects)