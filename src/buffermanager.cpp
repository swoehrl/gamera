/*
 * buffermanager.cpp
 *
 *  Created on: 09.03.2013
 *      Author: swoehrl
 */


#include <iostream>
#include <cstdio>
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
    delete[] frames;
    delete file;
	//std::cout << "Closed file" << std::endl;
}

BufferFrame& BufferManager::fixPage(unsigned pageId, bool exclusive) {
	//printf("fixPage %d\n", pageId);
	if (pageId >= numberofpages) {
		std::cout << "Error: PageId is invalid\n";
		throw "pageId is invalid";
	}

	while (true) {
		std::unique_lock<std::mutex> gl(*globlock);
		BufferFrame* f = frames[pageId];
		if (f != nullptr) {
			if ((exclusive || f->exclusive) && f->threadcount > 0) {
				f->threadwaiters++;
				f->waiter->wait(gl);
			} else {
				if (f->threadwaiters > 0) f->threadwaiters--;
				f->threadcount++;
                if (f->fixes == 1)
                    fifolist.remove(f);
                else
                    lrulist.remove(f);
                f->fixes++;
				f->isUsed = true;
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
                char* data = readFrame(pageId);
				BufferFrame* f = new BufferFrame(pageId, data, exclusive, std::this_thread::get_id());
				frames[pageId] = f;
				return *f;
			}
		}
	}

	/*
	std::unique_lock<std::mutex> gl(*globlock);
	BufferFrame* f = frames[pageId];
	if (f != nullptr) {
		//std::unique_lock<std::mutex> l(*f->lock);
		//gl.unlock();
		if ((exclusive || f->exclusive) && f->threadcount > 0) {
			f->threadwaiters++;
			f->waiter->wait(gl, [this, pageId](){return frames[pageId] != nullptr & !frames[pageId]->exclusive;});
		}
		f->threadcount++;
		f->isUsed = true;
		f->exclusive = exclusive;
		return *f;
	} else {
		//std::unique_lock<std::mutex> l(*lock);
		if (frames[pageId] != nullptr) {
			printf("Error: page installed while locking\n");
			throw "page installed while locking";
		}
		if (freeframes <= 0 && !freeFrame()) {
			printf("Waiting for frame (pageId = %d)\n", pageId);
			//std::cout << "Waiting for frame (pageId = " << pageId << ")\n";
			waiter.wait(gl, [this](){freeFrame(); return freeframes >= 1;});
			//freeFrame();
			if (freeframes <= 0) {
				std::cout << "Error: no free frames\n";
				throw "no freeframes";
			}
		}
		freeframes--;
		BufferFrame* f = new BufferFrame(pageId, readFrame(pageId), exclusive, std::this_thread::get_id());
		frames[pageId] = f;
		return *f;
	}
	*/
}


void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	std::unique_lock<std::mutex> l(*globlock);
	BufferFrame* f2 = frames[frame.pageId];
	if (f2 == nullptr) {
		std::cout << "Error: Invalid BufferFrame\n";
		throw "Invalid BufferFrame";
	}
    //std::unique_lock<std::mutex> locallock(*f2->lock);
    //l.unlock();
	f2->isDirty = (isDirty || f2->isDirty);
	if (f2->threadcount <= 1) {
		//std::cout << "Unlocking frame for pageId " << f2->pageId << "\n";
		//if (f2->isDirty) writeFrame(f2);
		//f2->isDirty = false;
        if (f2->fixes == 1)
            fifolist.push_back(f2);
        else
            lrulist.push_back(f2);
            //fifolist.push_back(f2);
		f2->isUsed = false;
		f2->waiter->notify_all();
		//std::cout << "Notifying one waiting thread\n";
		waiter.notify_one();
	}
	f2->threadcount--;
}


void BufferManager::writeFrame(BufferFrame* frame) {
	//std::cout << "Writing page with id " << frame.pageId << std::endl;
	file->seekp(frame->pageId*PAGESIZE);
	file->write(frame->data, PAGESIZE);
}

char* BufferManager::readFrame(uint pageId) {
	char* data = new char[PAGESIZE];
	file->seekg(pageId*PAGESIZE);
	file->read(data, PAGESIZE);
	return data;
}


bool BufferManager::freeFrame() {
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
    if (frame->isDirty) writeFrame(frame);
    frames[frame->pageId] = nullptr;
    delete frame;
    freeframes++;
    return true; 
    /*
	for (uint i=0; i < numberofpages; i++) {
		BufferFrame* frame = frames[i];
		if (frame == nullptr) continue;
		//std::unique_lock<std::mutex> l(*frame->lock);
		//std::cout << frame->pageId <<" : " << frame->isUsed << "\n";
		if (!frame->isUsed && frame->threadcount == 0 && frame->threadwaiters == 0) {
			if (frame->isDirty) writeFrame(frame);
			//printf("Freeing page %u\n", frame->pageId);
			frames[i] = nullptr;
			fifolist.remove(frame);
            //frame->~BufferFrame();
			//frame->waiter->notify_all();
			//l.unlock();
			delete frame;
			freeframes++;
			//std::cout << "Found frame to clear up\n";
			return true;
		}
	}
	return false;
    */
}


BufferFrame::BufferFrame(uint pageId, char* data, bool exclusive, std::thread::id thread1) {
	this->pageId = pageId;
	this->data = data;
	this->exclusive = exclusive;
	this->lock = new std::mutex;
	this->waiter = new std::condition_variable;
}


BufferFrame::~BufferFrame() {
    //std::cout << "Calling BufferFrame destructor\n";
	delete[] data;
    delete this->lock;
    delete this->waiter;
}

void* BufferFrame::getData() {
	return (void*)data;
}

uint BufferFrame::getPageid() {
	return pageId;
}



