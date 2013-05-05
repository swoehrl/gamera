
#include <cstdlib>
#include <cassert>

#include "segment_manager/SegmentInventory.hpp"



SegmentInventory::SegmentInventory(void* page, FreeSpaceSegment* pfss) {
    pagedata = page;
    fss = pfss;
    uint16_t* pstartpage = (uint16_t*)pagedata;
    uint16_t* plength = pstartpage+1;
    uint16_t* end = pstartpage+(LOCAL_PAGESIZE/sizeof(uint16_t));
    ExtentList* l = nullptr;
    SegmentID segmentid = 0;
    while (pstartpage < end && plength < end && !(*pstartpage == 0 && *plength == 0)) {
        if (*plength == 0) {// new segment
            if (l != nullptr)
                extents.insert({segmentid, l});
            l = new ExtentList();
            segmentid = *pstartpage;
            if (nextsegmentid <= segmentid)
                nextsegmentid = segmentid+1;
            //std::cout << "Reading new segment with id " << segmentid << std::endl;
        } else {
            //std::cout << "Reading extent for segment with startpage: " << *pstartpage << " , length: " << *plength << "\n";
            l->push_back({*pstartpage, *plength});
        }
        pstartpage += 2;
        plength += 2;
    }
    if (l != nullptr)
        extents.insert({segmentid, l});
}

SegmentInventory::~SegmentInventory() {
    flush();
    for (auto it : extents) {
        delete it.second;
    }
}

void SegmentInventory::flush() {
    uint16_t* pstartpage = (uint16_t*)pagedata;
    uint16_t* plength = pstartpage+1;
    uint16_t* end = pstartpage+(LOCAL_PAGESIZE/sizeof(uint16_t));
    ExtentList* l = nullptr;
    SegmentID segmentid = 0;
    for (auto segment : extents) {
        if (pstartpage >= end || plength >= end)
            throw "Too many extents to store";
        l = segment.second;
        segmentid = segment.first;
        *pstartpage = segmentid;
        *plength = 0;
        pstartpage += 2;
        plength += 2;
        for (auto ext : *l) {
            if (pstartpage >= end || plength >= end)
                throw "Too many extents to store";
            *pstartpage = ext.first;
            *plength = ext.second;
            pstartpage += 2;
            plength += 2;
        }
    }
}

SegmentID SegmentInventory::newSegment(uint16_t numpages) {
    assert(numpages > 0);
    SegmentID newid = nextsegmentid++;
    ExtentList* l = new ExtentList();
    uint16_t remainingpages = numpages;
    uint16_t startpage = 0;
    uint16_t length = 0;
    while (remainingpages > 0) {
        fss->getFreePages(remainingpages, startpage, length);
        if (length == 0) {
            delete l;
            std::cout << "Error: no remaining free pages" << std::endl;
            throw "no remaining free pages";
        }
        l->push_back({startpage, length});
        remainingpages -= length;
    }
    extents.insert({newid, l});
    return newid;
}

ExtentList* SegmentInventory::getExtents(SegmentID id) {
    return extents[id];
}

void SegmentInventory::growSegment(SegmentID id, uint16_t numnewpages) {
    assert(id < nextsegmentid);
    auto it = extents.find(id);
    assert(it != extents.end());
    ExtentList* l = (*it).second;
    uint16_t remainingpages = numnewpages;
    uint16_t startpage = 0;
    uint16_t length = 0;
    while (remainingpages > 0) {
        fss->getFreePages(remainingpages, startpage, length);
        if (length == 0)
            throw "no remaining free pages";
        l->push_back({startpage, length});
        remainingpages -= length;
    }
}

void SegmentInventory::deleteSegment(SegmentID id) {
    assert(id < nextsegmentid);
    auto it = extents.find(id);
    assert(it != extents.end());
    ExtentList* l = (*it).second;
    for (auto block : *l) {
        fss->freePages(block.first, block.second);
    }
    l->clear();
}

