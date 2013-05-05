#ifndef SEGMENTINVENTORY_HPP_
#define SEGMENTINVENTORY_HPP_

#include <map>

#include "segment_manager/SegmentBase.hpp"
#include "segment_manager/SegmentManager.hpp"
#include "segment_manager/Segment.hpp"
#include "segment_manager/FreeSpaceSegment.hpp"

const int MAXSEGMENTS = 256;

// structure of metadata:
// for every segment:
//   uint16_t segmentid
//   uint16_t 0 (signals new segment)
//   for every extent:
//     uint16_t start page
//     uint16_t number of pages


class SegmentInventory {
private:
    void* pagedata;
    std::map<SegmentID, ExtentList*> extents;
    uint16_t nextsegmentid = 1; // next free segment id
    FreeSpaceSegment* fss;

    SegmentInventory(void* page, FreeSpaceSegment* pfss);
    ExtentList* getExtents(SegmentID id);
public:
    ~SegmentInventory();
    void flush();
    friend class SegmentManager;
    SegmentID newSegment(uint16_t numpages);
    void growSegment(SegmentID id, uint16_t numnewpages);
    void deleteSegment(SegmentID id);
}; 

#endif

