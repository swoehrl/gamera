CC = g++
#CC = clang++
DEBUG = -O0 -g3
RELASE = -O3
TBBFLAGS = -Ilib/tbb/include -ltbb -Llib/tbb/lib/intel64/gcc4.4
FLAGS = -Wall -fmessage-length=0 -std=c++11 -pthread #-gcc-toolchain /opt/gcc-4.8.0
GTEST_DIR = lib/gtest-1.6.0
LIB_DIR = lib
TEST_FLAGS = $(DEBUG) -I$(GTEST_DIR)/include -Isrc -pthread -lpthread
build_dir = @mkdir -p $(dir $@)
COMP = $(CC) $(DEBUG) $(FLAGS) -Isrc 
objDir:= bin/
srcDir:= src/

#obj_files := $(addprefix $(objDir),$(src_files))

all: build/mergesort.o build/buffermanager.o

test: build/test1 build/test_mergesort build/test_buffermanager build/test_segmentmanager

build/mergesort.o: src/mergesort.cpp
	$(CC) $(DEBUG) $(FLAGS) -c src/mergesort.cpp -o build/mergesort.o

build/buffermanager.o: src/buffermanager.cpp src/buffermanager.hpp
	$(CC) $(DEBUG) $(FLAGS) -c src/buffermanager.cpp -o build/buffermanager.o

SEGMENT_SRC = src/segment_manager/*.cpp
SEGMENT_OBJ = $(SEGMENT_SRC:src/segment_manager/%.cpp=build/%.o)

build/SegmentManager.o: src/segment_manager/*.cpp src/segment_manager/*.hpp
	$(COMP) -c src/segment_manager/SegmentManager.cpp -o build/SegmentManager.o
	$(COMP) -c src/segment_manager/Segment.cpp -o build/Segment.o
	$(COMP) -c src/segment_manager/SegmentInventory.cpp -o build/SegmentInventory.o
	$(COMP) -c src/segment_manager/FreeSpaceSegment.cpp -o build/FreeSpaceSegment.o
	$(COMP) -c src/segment_manager/SPSegment.cpp -o build/SPSegment.o


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


build/test_segmentmanager: build/libgtest.a src/tests/SegmentManagerTest.cpp build/SegmentManager.o build/buffermanager.o
	$(CC) $(FLAGS) $(TEST_FLAGS) src/tests/SegmentManagerTest.cpp $(SEGMENT_OBJ) build/libgtest.a -Isrc -o build/test_segmentmanager


init:
	mkdir -p build lib
	dd if=/dev/zero of=database.file bs=1024 count=102400
	dd if=/dev/zero of=segmentdatabase.file bs=1024 count=256000

clean:
	rm build/*

clean-gtest:
	rm build/libgtest.a build/gtest-all.o
	rm lib/gtest-1.6.0.zip
	rm -r lib/gtest-1.6.0

.PHONY: all test clean clean-gtest
