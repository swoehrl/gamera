CC = g++
#CC = clang++
DEBUG = -O0 -g3
RELASE = -O3 
FLAGS = -Wall -fmessage-length=0 -std=c++11 -pthread #-gcc-toolchain /opt/gcc-4.8.0
GTEST_DIR = lib/gtest-1.6.0
LIB_DIR = lib
TEST_FLAGS = $(DEBUG) -I$(GTEST_DIR)/include -Isrc -pthread -lpthread

all: build/mergesort.o build/buffermanager.o

test: build/test1 build/test_mergesort build/test_buffermanager

build/mergesort.o: src/mergesort.cpp
	$(CC) $(DEBUG) $(FLAGS) -c src/mergesort.cpp -o build/mergesort.o

build/buffermanager.o: src/buffermanager.cpp src/buffermanager.hpp
	$(CC) $(DEBUG) $(FLAGS) -c src/buffermanager.cpp -o build/buffermanager.o

lib/gtest-1.6.0.zip:
	wget http://googletest.googlecode.com/files/gtest-1.6.0.zip -O lib/gtest-1.6.0.zip
	unzip lib/gtest-1.6.0.zip -d lib/

build/libgtest.a: lib/gtest-1.6.0.zip
	$(CC) -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc -o build/gtest-all.o
	ar -rv build/libgtest.a build/gtest-all.o

build/test1: build/libgtest.a src/tests/test1.cpp
	$(CC) $(TEST_FLAGS) src/tests/test1.cpp build/libgtest.a -o build/test1 

build/test_mergesort: build/libgtest.a src/tests/test_mergesort.cpp build/mergesort.o 
	$(CC) $(FLAGS) $(TEST_FLAGS) src/tests/test_mergesort.cpp build/mergesort.o build/libgtest.a -o build/test_mergesort

build/test_buffermanager: build/libgtest.a src/tests/test_buffermanager.cpp build/buffermanager.o
	$(CC) $(FLAGS) $(TEST_FLAGS) src/tests/test_buffermanager.cpp build/buffermanager.o build/libgtest.a -o build/test_buffermanager

build/pqtest: src/pqtest.cpp
	$(CC) $(FLAGS) src/pqtest.cpp -o build/pqtest

clean:
	rm build/*

clean-gtest:
	rm build/libgtest.a build/gtest-all.o
	rm lib/gtest-1.6.0.zip
	rm -r lib/gtest-1.6.0

.PHONY: all test clean clean-gtest
