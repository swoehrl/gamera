/*
 * mergesort.cpp
 *
 *  Created on: 08.03.2013
 *      Author: swoehrl
 */
#include <inttypes.h>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <iostream>

using namespace std;
const int INT64SIZE = 8;

bool checkchunk(uint64_t* chunk, uint64_t numsinchunk, uint64_t& maxval) {
	if (chunk[0] < maxval)
		return false;
	for (uint64_t i=0; i < numsinchunk-1; i++) {
		//cout << chunk[i] << endl;
		if (chunk[i] > chunk[i+1])
			return false;
	}
	maxval = chunk[numsinchunk-1];
	//cout << chunk[numsinchunk-1] << endl;
	return true;
}


bool checkchunks(ifstream* file, uint64_t numnums, uint64_t memsize, char* buffer = nullptr) {
	if (buffer == nullptr)
		buffer = new char[memsize];
	uint64_t* ibuf = (uint64_t*)buffer;
	uint64_t maxval = 0;
	for (uint64_t i=0; i < (numnums*INT64SIZE/memsize); i++) {
		memset(buffer, 0, memsize);
		file->read(buffer, memsize);
		if (!checkchunk(ibuf, memsize/INT64SIZE, maxval))
			return false;
	}
	return true;
}





void sortchunk(fstream *file, uint64_t pos, uint64_t memsize, char* buffer) {
	uint64_t* ibuf = (uint64_t*)buffer;
	file->seekg(pos);
	file->read(buffer, memsize);
	sort(ibuf, ibuf+(memsize/INT64SIZE));
	file->seekp(pos);
	file->write(buffer, memsize);
}

void loadchunk(fstream *infile, uint64_t &offset, uint64_t memsize, uint64_t mchunksize, uint64_t chunk, char *buffer) {
	memset(buffer, 0, mchunksize*8);
	if (offset >= memsize)
		return;
	infile->seekg(chunk*memsize+offset);
	uint64_t readsize = mchunksize*8;
	if (offset+mchunksize*8 > memsize)
		readsize = memsize-offset;
	infile->read(buffer, readsize);
	if ((uint64_t)infile->gcount() < readsize)
		readsize = infile->gcount();
	offset += readsize;
}


void mergechunks(fstream *infile, ofstream *outfile, uint64_t numchunks, uint64_t num, uint64_t memsize, char* buffer) {
	uint64_t mchunksize = memsize/numchunks / 8;
	uint64_t* ibuf = (uint64_t*)buffer;
	char** mchunkp = new char*[numchunks];
	uint64_t** mchunkip = new uint64_t*[numchunks];
	for (uint64_t i=0; i < numchunks; i++) {
		mchunkip[i] = ibuf+(mchunksize*i);
		mchunkp[i] = buffer+(mchunksize*8*i);
	}
	uint64_t* chunkoffsets = new uint64_t[numchunks]; // which part of the chunk is already read from the file
    memset(chunkoffsets, 0, numchunks*sizeof(uint64_t));
    char* outbuf = new char[mchunksize*8];
    memset(outbuf, 0, mchunksize*sizeof(char));
	uint64_t* ioutbuf = (uint64_t*)outbuf;
	uint64_t* outip = ioutbuf;
	for (uint64_t i=0; i < numchunks; i++) {
		loadchunk(infile, chunkoffsets[i], memsize, mchunksize, i, mchunkp[i]);
	}
	for (uint64_t i=0; i < num; i++) {
		// find minimal element
		int minchunk = 0;
		uint64_t minval = 0;
		for (uint64_t j=0; j < numchunks; j++) {
			//if (mchunkip[j] == -1) continue;
			uint64_t val = *(mchunkip[j]);
			if (val == 0) continue;
			if (*(mchunkip[j]) < minval || minval == 0) {
				minval = *mchunkip[j];
				minchunk = j;
			}
		}
		//cout << "Minchunk: " << minchunk << endl;
		*outip = minval;
		//cout << minval << endl;
		outip++;
		mchunkip[minchunk]++;
		//mchunkp[minchunk] += INT64SIZE;
		if (ioutbuf+mchunksize == outip) {// buffer full -> dump into file
			outfile->write(outbuf, mchunksize*8);
			outip = ioutbuf;
			memset(ioutbuf, 0, mchunksize*8);
		}
		if (mchunkip[minchunk] - mchunksize == (uint64_t*)mchunkp[minchunk]) {
			loadchunk(infile, chunkoffsets[minchunk], memsize, mchunksize, minchunk, mchunkp[minchunk]);
			mchunkip[minchunk] = (uint64_t*)mchunkp[minchunk];
		}
	}
	// output buffer is not empty -> dump rest into file
	if (ioutbuf != outip) {
		//uint64_t l = outip-ioutbuf;
		outfile->write(outbuf, (outip-ioutbuf)*8);
	}
    delete[] outbuf;
    delete[] mchunkp;
    delete[] mchunkip;
    delete[] chunkoffsets;
}


void externalsort(const char* in, const char* out, uint64_t memsize) {
	char * buffer = new char[memsize];
	fstream infile;
	infile.open(in, ios::binary | ios::in | ios::out);
    infile.seekg (0, infile.end);
    int length = infile.tellg();
    infile.seekg (0, infile.beg);
    uint64_t num = length/8;
    std::cout << "Number of entries to sort: " << num << std::endl;

	ofstream outfile;
	outfile.open(out, ios::binary | ios::out);

	for (uint64_t i = 0; i < num*INT64SIZE; i+=memsize) {
		sortchunk(&infile, i, memsize, buffer);
	}

	mergechunks(&infile, &outfile, num*INT64SIZE / memsize, num, memsize, buffer);
	infile.close();
	outfile.flush();
	outfile.close();
	delete[] buffer;
}


bool checksort(const char* filename, uint64_t memsize) {
	char * buffer = new char[memsize];
	ifstream file;
	file.open(filename, ios::binary | ios::in);
    file.seekg (0, file.end);
    int length = file.tellg();
    file.seekg (0, file.beg);
    uint64_t num = length/8;
	bool result = checkchunks(&file, num, memsize, buffer);
    delete[] buffer;
    file.close();
    return result;
}

/*
int main() {
    externalsort("test.num", "testout.num", 100*1024);
    return 0;
}
*/
