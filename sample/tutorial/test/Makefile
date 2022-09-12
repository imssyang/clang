all: test1 test2 test3 test4 test5

build-test1:
	clang++ -o test1 test1.cpp

build-test2:
	clang++ -o test2 test2.cpp

build-test3:
	clang++ -o test3 test3.cpp

build-test4:
	clang++ -o test4 test4.cpp

build-test5:
	clang++ -o test5 test5.cpp

test1: build-test1
	./test1
	rm -rf test1

test2: build-test2
	./test2
	rm -rf test2

test3: build-test3
	./test3
	rm -rf test3

test4: build-test4
	./test4
	rm -rf test4

test5: build-test5
	./test5
	rm -rf test5
