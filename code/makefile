include default

commonobjects =  data_stream.o io.o bits.o stlutil.o frequency_table.o

build : $(commonobjects)

data_stream.o : data_stream.h defs.h
io.o : io.h defs.h
bits.o : bits.h
stlutil.o : stlutil.h defs.h
frequency_table.o : frequency_table.h defs.h stlutil.h

clean :
	rm -f $(PROG) $(commonobjects)