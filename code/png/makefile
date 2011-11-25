PROG = png
include ../default
objects = main.o ../common.o png.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h png.h
png.o : png.h ../common.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)