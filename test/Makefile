

test: ../src/data2vcd.a test1 test_ela600
	./test1 ela.json output_n.vcd
	./test_ela600 ela.json output_ela600.vcd

../src/data2vcd.a:
	(cd ../src; make)

test1: test1.cpp ../src/data2vcd.a
	g++ -o test1 test1.cpp -I../src ../src/data2vcd.a


test_ela600: test_ela600.cpp ../src/data2vcd.a
	g++ -o test_ela600 test_ela600.cpp -I../src ../src/data2vcd.a
