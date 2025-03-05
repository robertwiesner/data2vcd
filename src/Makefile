
OBJ = cmodule.o  \
	cwire.o      \
	coutput.o    \
	cvcdoutput.o \
	cjson.o      \
	cbitfield.o

ifdef SHARED
CXXFLAGS = -g -Wall -Wno-nonnull-compare -fpic
OUTPUT=libdata2vcd.so
else
CXXFLAGS = -g -Wall -Wno-nonnull-compare
OUTPUT=data2vcd.a
endif

all: $(OUTPUT)
	@echo $(OUTPUT) generated

data2vcd.a: $(OBJ)
	ar rcs  data2vcd.a $(OBJ)

libdata2vcd.so: $(OBJ)
	gcc -shared -o libdata2vcd.so $(OBJ)

clean:
	-rm $(OBJ)
	-rm data2vcd.a libdata2vcd.so
