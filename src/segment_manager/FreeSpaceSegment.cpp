
#include <cstring>
#include <cassert>

#include "segment_manager/SegmentBase.hpp"
#include "segment_manager/FreeSpaceSegment.hpp"


FreeSpaceSegment::FreeSpaceSegment(void* page) {
    assert(page != nullptr);
    pagedata = page;
    uint16_t* pstartpage = (uint16_t*)pagedata;
    uint16_t* end = pstartpage+(LOCAL_PAGESIZE/sizeof(uint16_t));
    uint16_t* plength = pstartpage+1;
    if (*plength == 0) {
        //std::cout << "Reading empty freespacesegment\n";
        // no entries -> assume all pages are free
        extents.insert({MAXNUMPAGES-2, 2});
        return;
    }
    while(*plength > 0 && plength < end) {
        //std::cout << "FreeSpaceExtent: startpage: " << *pstartpage << " , length: " << *plength << "\n";
        extents.insert({*plength, *pstartpage});
        pstartpage += 2;
        plength += 2;
    }
}


void FreeSpaceSegment::flush() {
    assert(pagedata != nullptr);
    memset(pagedata, 0, LOCAL_PAGESIZE);
    assert(pagedata != nullptr);
    uint16_t* pstartpage = (uint16_t*)pagedata;
    uint16_t* end = pstartpage+(LOCAL_PAGESIZE/sizeof(uint16_t));
    uint16_t* plength = pstartpage+1;
    for (auto it : extents) {
        if (plength >= end)
            throw "Too many extents to store";
        *pstartpage = it.second;
        *plength = it.first;
        pstartpage += 2;
        plength += 2;
    }
}

bool FreeSpaceSegment::getFreePages(uint16_t desiredlength, uint16_t& startpage, uint16_t& length) {
    auto it = extents.lower_bound(desiredlength);
    if (it != extents.end()) {
        if ((*it).first > desiredlength) {
            length = desiredlength;
            startpage = (*it).second;
            uint16_t freelength = (*it).first;
            extents.erase(it);
            extents.insert({freelength-desiredlength, startpage+desiredlength});
        } else {
            length = (*it).first;
            startpage = (*it).second;
            extents.erase(it);
        }
        return true;
    } else {
        length = 0;
        startpage = 0;
        return false;
    }
}


void FreeSpaceSegment::freePages(uint16_t startpage, uint16_t length) {
    assert(length > 0);
    extents.insert({length, startpage});
}


