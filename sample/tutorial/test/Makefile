CC = clang
CXX = clang++
CFLAGS = -O3
CXXFLAGS = $(CFLAGS)

all: all_c all_cxx
all_c: c/test1 c/test2 c/test3 c/test4 c/test5
all_cxx: cxx/test1 cxx/test2 cxx/test3 cxx/test4 cxx/test5

c cxx:
	mkdir -p $@

c/%: %.c | c
	$(CC) -o $@ $^ $(CFLAGS)
	./$@

cxx/%: %.cpp | cxx
	$(CXX) -o $@ $^ $(CXXFLAGS)
	./$@

clean:
	@rm -rf c cxx
