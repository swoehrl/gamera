

#include "segment_manager/Segment.hpp"

Segment::Segment(SegmentID id, BufferManager* bm, SegmentManager* sm, ExtentList* extents) {
    this->bm = bm;
    this->sm = sm;
    this->extents = extents;
    this->id = id;
}

void Segment::grow(uint16_t deltapages) {
    sm->growSegment(id, deltapages); 
}


uint64_t Segment::size() {
    uint64_t size = 0;
    for (auto it: *extents) {
        size += it.second;
    }
    return size*LOCAL_PAGESIZE;
}

uint32_t Segment::numPages() {
    uint32_t num = 0;
    for (auto it: *extents) {
        num += it.second;
    }
    return num;
}


BufferFrame& Segment::getPage(uint16_t index, bool exclusive=true) {
    uint16_t curindex = 0;
    for (Extent e: *extents) {
        if (index < curindex+e.second) {
            uint16_t newindex = e.first + index-curindex;
            //std::cout << "getPage(" << (int)index << ") translates to fixPage(" << (int)newindex << ")\n";
            return bm->fixPage(newindex, exclusive);
        } else
            curindex += e.second;
    }
    // Should never happen
    throw "Index out of range";
}

void Segment::forgetPage(BufferFrame& bf, bool dirty) {
    bm->unfixPage(bf, dirty);
}


void* Segment::operator*() {
    return nullptr;
}


void* Segment::operator++() {
    return nullptr;

}


void Segment::reset() {

}

