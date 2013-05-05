/*
 * buffermanager.hpp
 *
 *  Created on: 09.03.2013
 *      Author: swoehrl
 */

#ifndef BUFFERMANAGER_HPP_
#define BUFFERMANAGER_HPP_

#include <unordered_map>
#include <list>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

const int PAGESIZE = 16*1024;



class BufferManager;

class BufferFrame {
	uint pageId;
	bool exclusive;
	char* data;
    std::atomic_uint threadcount{1}; 
    std::atomic_uint threadwaiters{0};
    volatile uint fixes = 1;
	std::condition_variable* waiter;
	std::mutex* lock;
	bool isDirty = false;
	friend BufferManager;
	BufferFrame(uint pageId, char* data, bool exclusive, std::thread::id thread1);
	BufferFrame(uint pageId, bool exclusive, std::thread::id thread1);
	~BufferFrame();
public:
	void* getData();
    bool isExclusive() {return exclusive;};
};

class BufferManager {
	BufferFrame** frames;
    uint* pagefixes;
    std::list<BufferFrame*> fifolist;
    std::list<BufferFrame*> lrulist;
    std::list<BufferFrame*> seclrulist;
	std::fstream *file;
	uint numberofpages;
	volatile uint freeframes;
	uint maxframes;
	std::condition_variable waiter;
	std::mutex* lock;
	std::mutex* globlock;
	std::mutex* filelock;
	void writeFrame(BufferFrame* frame);
	char* readFrame(uint pageId);
	bool freeFrame();
public:
	BufferManager(const std::string& filename, unsigned size);
	~BufferManager();
	BufferFrame& fixPage(unsigned pageId, bool exclusive);
	void unfixPage(BufferFrame& frame, bool isDirty);
	BufferFrame& newPage(bool exclusive);
};

#endif /* BUFFERMANAGER_HPP_ */
