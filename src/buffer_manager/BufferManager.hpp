
#ifndef BUFFERMANAGER_HPP_
#define BUFFERMANAGER_HPP_

#include <unordered_map>
#include <list>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


#include "buffer_manager/BufferFrame.hpp"


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

#endif 
