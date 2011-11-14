PROG = huffman
include ../default
objects = huffman.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

huffman.o : huffman.c ../common.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)