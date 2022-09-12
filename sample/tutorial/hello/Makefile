all: hello hello-bitcode hello-assembly hello-native-assembly

hello:
	clang -o hello hello.c

hello-bitcode:
	clang -o hello.bc -O3 -emit-llvm -c hello.c

hello-assembly:
	llvm-dis < hello.bc > hello.dis

hello-native-assembly:
	llc -o hello.s hello.bc

run-hello:
	./hello

run-hello-bitcode:
	lli hello.bc

clean:
	rm -rf hello
	rm -rf hello.bc
	rm -rf hello.dis
	rm -rf hello.s
