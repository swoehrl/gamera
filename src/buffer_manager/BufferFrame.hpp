
#ifndef BUFFERFRAME_HPP_
#define BUFFERFRAME_HPP_

#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class BufferManager;

const int PAGESIZE = 16*1024;

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


#endif
