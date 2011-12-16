include default
objects = common.o data_list.o print_funcs.o data_stream.o

all : $(objects) ;

common.o : common.h data_list.h defs.h print_funcs.h data_stream.h
data_list.o : data_list.h
data_stream.o : data_stream.h defs.h
print_funcs.o : print_funcs.h defs.h

clean :
	rm $(PROG) $(objects)