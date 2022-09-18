echo:
	echo "Hello, World"

hello: hello.c
	cc -o hello hello.c

blah: blah.o
	cc -o blah blah.o

blah.o: blah.c
	cc -o blah.o -c blah.c

blah.c:
	echo "int main() { return 0; }" > blah.c

clean:
	rm -rf hello
	rm -rf blah blah.o
