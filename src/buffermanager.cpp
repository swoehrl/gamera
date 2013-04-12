/*
 * buffermanager.cpp
 *
 *  Created on: 09.03.2013
 *      Author: swoehrl
 */


#include <iostream>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>
#include <algorithm>

#include "buffermanager.hpp"


BufferManager::BufferManager(const std::string& filename, unsigned size) {
	std::fstream* f = new(std::fstream);
	f->open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);
	file = f;
    file->seekg (0, file->end);
    int length = file->tellg();
    file->seekg (0, file->beg);
	maxframes = size;
	freeframes = size;
	numberofpages = length / PAGESIZE;
	this->lock = new std::mutex;
	this->globlock = new std::mutex;
	this->filelock = new std::mutex;
	this->frames = new BufferFrame*[numberofpages];
	for (uint i=0; i < numberofpages; i++) frames[i] = nullptr;
}

BufferManager::~BufferManager() {
	finish();
}

void BufferManager::finish() {
	for (uint i=0; i<numberofpages; i++) {
		BufferFrame* frame = frames[i];
		if (frame != nullptr) {
            if (frame->isDirty)
			    writeFrame(frame);
		    delete frame;
        }
    }
    file->close();
    delete lock;
    delete globlock;
    delete filelock;
    delete[] frames;
    delete file;
	//std::cout << "Closed file" << std::endl;
}

BufferFrame& BufferManager::fixPage(unsigned pageId, bool exclusive) {
	if (pageId >= numberofpages) {
		std::cout << "Error: PageId is invalid\n";
		throw "pageId is invalid";
	}
    bool waiting = false;
	while (true) {
		std::unique_lock<std::mutex> gl(*globlock);
		BufferFrame* f = frames[pageId];
		if (f != nullptr) {
		    std::unique_lock<std::mutex> locallock(*f->lock);
            if (f->fixes == 1)
                fifolist.remove(f);
            else
                lrulist.remove(f);
            gl.unlock();
			if ((exclusive || f->exclusive) && f->threadcount > 0) {
				f->threadwaiters++;
                waiting = true;
				f->waiter->wait(locallock);
				//f->waiter->wait(gl);
			} else {
				//if (f->threadwaiters > 0) f->threadwaiters--;
				if (waiting) f->threadwaiters--;
				f->threadcount++;
                f->fixes++;
				f->exclusive = exclusive;
				return *f;
			}
		} else {
			if (freeframes <= 0 && !freeFrame()) {
				waiter.wait(gl, [this](){freeFrame(); return freeframes >= 1;});
				if (freeframes <= 0) {
					std::cout << "Error: no free frames\n";
					throw "no freeframes";
				}
			} else {
				freeframes--;
                BufferFrame* f = new BufferFrame(pageId, exclusive, std::this_thread::get_id());
		        std::unique_lock<std::mutex> locallock(*f->lock);
				frames[pageId] = f;
                gl.unlock();
                char* data = readFrame(pageId);
                f->data = data;
				return *f;
			}
		}
	}
}


void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	std::unique_lock<std::mutex> gl(*globlock);
	BufferFrame* f2 = frames[frame.pageId];
	if (f2 == nullptr) {
		std::cout << "Error: Invalid BufferFrame\n";
		throw "Invalid BufferFrame";
	}
    std::unique_lock<std::mutex> locallock(*f2->lock);
	f2->isDirty = (isDirty || f2->isDirty);
	if (f2->threadcount <= 1 && f2->threadwaiters == 0) {
        if (f2->fixes == 1)
            fifolist.push_back(f2);
        else
            lrulist.push_back(f2);
        waiter.notify_one();
	} else if (f2->threadcount <= 1 && f2->threadwaiters > 0) {
		f2->waiter->notify_one();
    }
    gl.unlock();
	f2->threadcount--;
}


void BufferManager::writeFrame(BufferFrame* frame) {
    std::unique_lock<std::mutex> l(*filelock);
	file->seekp(frame->pageId*PAGESIZE);
	file->write(frame->data, PAGESIZE);
}

char* BufferManager::readFrame(uint pageId) {
    std::unique_lock<std::mutex> l(*filelock);
	char* data = new char[PAGESIZE];
	file->seekg(pageId*PAGESIZE);
    memset(data, 0, PAGESIZE*sizeof(char));
	file->read(data, PAGESIZE);
	return data;
}


bool BufferManager::freeFrame() {
    //std::cout << "freeFrame called\n";
    BufferFrame* frame;
    if (fifolist.empty()) { // take element from lrulist
        if (lrulist.empty())
            return false;
        frame = lrulist.front();
        lrulist.pop_front();
    } else {
        frame = fifolist.front();
        fifolist.pop_front();
    }
    std::unique_lock<std::mutex> l(*frame->lock);
    frames[frame->pageId] = nullptr;
    if (frame->isDirty) writeFrame(frame);
    l.unlock();
    delete frame;
    freeframes++;
    return true; 
}


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

