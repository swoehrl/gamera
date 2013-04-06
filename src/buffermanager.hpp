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

const int PAGESIZE = 4096;



class BufferManager;

class BufferFrame {
	uint pageId;
	bool exclusive;
	char* data;
	volatile uint threadcount = 1;
	volatile uint threadwaiters = 0;
    volatile uint fixes = 1;
	//std::unordered_map<std::thread::id, bool> threads;
	//std::list<std::condition_variable*> waitingthreads;
	std::condition_variable* waiter;
	std::mutex* lock;
	bool isDirty = false;
	bool isUsed = true;
	friend BufferManager;
	BufferFrame(uint pageId, char* data, bool exclusive, std::thread::id thread1);
public:
	~BufferFrame();
	void* getData();
	uint getPageid();
};

class BufferManager {
	BufferFrame** frames;
    std::list<BufferFrame*> fifolist;
    std::list<BufferFrame*> lrulist;
	std::fstream *file;
	uint numberofpages;
	volatile uint freeframes;
	uint maxframes;
	std::condition_variable waiter;
	std::mutex* lock;
	std::mutex* globlock;
	void writeFrame(BufferFrame* frame);
	char* readFrame(uint pageId);
	bool freeFrame();
public:
	BufferManager(const std::string& filename, unsigned size);
	~BufferManager();
	BufferFrame& fixPage(unsigned pageId, bool exclusive);
	void unfixPage(BufferFrame& frame, bool isDirty);
	BufferFrame& newPage(bool exclusive);
	void finish();
};

#endif /* BUFFERMANAGER_HPP_ */
