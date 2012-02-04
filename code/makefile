include default

commonobjects =  data_stream.o io.o bitwise.o bits.o stlutil.o

build : $(commonobjects)

data_stream.o : data_stream.h defs.h bitwise.h
io.o : io.h defs.h
bitwise.o : bitwise.h defs.h
bits.o : bits.h
stlutil.o : stlutil.h
clean :
	rm $(PROG) $(objects)