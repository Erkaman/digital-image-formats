PROG = huffman
include ../default
objects = main.o ../common.o freq_table.o tree.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h ../common.h freq_table.h
freq_table.o : freq_table.h main.h
tree.o : tree.h main.h freq_table.h ../common.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)