CPP = g++
CFLAGS = -O2 -Wshadow -Wall -Werror -Wextra -std=c++11 -I./src -I./test
 
EXE = bin/huffman
TEST = bin/test
EXE_OBJECTS = bin/huffman.o bin/main.o
TEST_OBJECTS = bin/test.o bin/huffman.o bin/huffman_test.o
GTEST_LIBS = -lgtest -lgtest_main

all : bin $(EXE)
test: bin $(TEST)

bin:
	mkdir -p bin

$(EXE) : $(EXE_OBJECTS)
	$(CPP) $(EXE_OBJECTS) -o $(EXE) 

$(TEST) : $(TEST_OBJECTS) 
	$(CPP) $(TEST_OBJECTS) -o $(TEST) $(GTEST_LIBS)

bin/%.o : src/%.cpp
	$(CPP) $(CFLAGS) -c -o $@ $<

bin/%.o : test/%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@ $(GTEST_LIBS)

clean:
	rm bin/*.o $(EXE) $(TEST)

.PHONY: clean all test

# additional rules to take headers into account
bin/huffman.o: src/huffman.h
bin/test.o: test/huffman_test.h
bin/huffman_test.o: test/huffman_test.h src/huffman.h