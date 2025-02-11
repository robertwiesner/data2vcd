
OBJ = cmodule.o  \
	cwire.o      \
	coutput.o    \
	cvcdoutput.o \
	cjson.o      \
	cbitfield.o  \
	main.o

CXXFLAGS = -g 

json2vcd: $(OBJ)
	g++ -g -o json2vcd $(OBJ)

clean:
	-rm $(OBJ)
	-rm json2vcd