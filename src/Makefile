
OBJ = cmodule.o  \
	cwire.o      \
	coutput.o    \
	cvcdoutput.o \
	cjson.o      \
	cbitfield.o

CXXFLAGS = -g
libdata2vcd.so: $(OBJ)
	ar rcs  data2vcd.a $(OBJ)

clean:
	-rm $(OBJ)
	-rm libdata2vcd.so