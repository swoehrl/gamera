

#include "segment_manager/SegmentManager.hpp"

SegmentManager::SegmentManager(BufferManager* bm) : segments() {
    this->bm = bm;
    siframe = &bm->fixPage(0, true);
    fssframe = &bm->fixPage(1, true);
    fss = new FreeSpaceSegment(fssframe->getData());
    si = new SegmentInventory(siframe->getData(), fss);
}

SegmentManager::~SegmentManager() {
    fss->flush();
    si->flush();
    bm->unfixPage(*fssframe, true);
    bm->unfixPage(*siframe, true);
    delete si;
    delete fss;
}

SegmentID SegmentManager::createSegment(SegmentType type, uint16_t numpages) {
    return si->newSegment(numpages);
}

Segment* SegmentManager::retrieveSegment(SegmentID id) {
    auto it = segments.find(id);
    if (it == segments.end()) {
        Segment* s = new Segment(id, bm, this, si->getExtents(id));
        segments.insert({id, s});
        return s;
    } else {
        return (it->second);
    }
}


template <typename S>
S& SegmentManager::getSegment(SegmentID id) {
    Segment* seg = retrieveSegment(id);
    S* s = new S(seg);
    return *s;
}

void SegmentManager::dropSegment(SegmentID id) {

}

void SegmentManager::growSegment(SegmentID id, uint16_t deltanumpages) {
    si->growSegment(id, deltanumpages);
}



