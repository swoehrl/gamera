//============================================================================
// Name        : CDBImpl.cpp
// Author      : Sebastian Woehrl
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
//#include <fstream>
#include <thread>
#include <chrono>
//#include "mergesort.hpp"
//#include "buffermanager.hpp"
#include "segments.hpp"

using namespace std;


int main() {
	Extent e;
	cout << "sizeof(e) = " << sizeof(e) << endl;
	/*
	m = new BufferManager("database.file", 100);
	for (int i=0; i < 100; i++) {
		BufferFrame f = m->newPage(true);
		uint* d = (uint*)f.getData();
		d[0] = 0;
		//std::cout << "d[1]: " << d[1] << std::endl;
		m->unfixPage(f, true);
	}

	m->finish();
	*/
	//delete m;
	//std::thread t1(dosomething);
	//t1.join();
	//externalsort("test.num", 12800, "testout.num", 10*1024);
	return 0;
}


