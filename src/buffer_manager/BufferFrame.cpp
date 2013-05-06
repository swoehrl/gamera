

#include "buffer_manager/BufferFrame.hpp"


BufferFrame::BufferFrame(uint pageId, char* data, bool exclusive, std::thread::id thread1) {
	this->pageId = pageId;
	this->data = data;
	this->exclusive = exclusive;
	this->lock = new std::mutex;
	this->waiter = new std::condition_variable;
}


BufferFrame::BufferFrame(uint pageId, bool exclusive, std::thread::id thread1) {
	this->pageId = pageId;
	this->exclusive = exclusive;
	this->lock = new std::mutex;
	this->waiter = new std::condition_variable;
}

BufferFrame::~BufferFrame() {
	delete[] data;
    delete this->lock;
    delete this->waiter;
}

void* BufferFrame::getData() {
	return (void*)data;
}

