PROG = huffman
include ../default
objects = main.o tree.o huffman.o $(commonobjects)

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.cpp  ../frequency_table.h tree.h ../bits.h huffman.h ../stlutil.h
tree.o : tree.h ../io.h
huffman.o : ../frequency_table.h tree.h ../io.h ../bits.h

clean :
	rm -f $(PROG) $(objects)