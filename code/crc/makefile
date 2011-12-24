PROG = crc
include ../default
objects = main.o 

$(PROG) : $(objects) $(commonobjects)
	cd .. && $(MAKE)
	$(CC)  $(objects) $(commonobjects) -o $(PROG)

main.o : main.c ../data_list.h

clean :
	rm $(PROG) $(objects)