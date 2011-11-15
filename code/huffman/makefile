PROG = huffman
include ../default
objects = huffman.o ../common.o freq_table.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

huffman.o : huffman.h ../common.h freq_table.h
freq_table.o : freq_table.h huffman.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)