include default

commonobjects = data_list.o data_stream.o io.o bitwise.o

build : $(commonobjects)

data_list.o : data_list.h defs.h
data_stream.o : data_stream.h defs.h data_list.h bitwise.h
io.o : io.h defs.h
bitwise.o : bitwise.h defs.h

clean :
	rm $(PROG) $(objects)