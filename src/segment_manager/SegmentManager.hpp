#ifndef SEGMENTMANAGER_HPP_
#define SEGMENTMANAGER_HPP_

#include <unordered_map>

#include "segment_manager/SegmentBase.hpp"
#include "buffer_manager/BufferManager.hpp"
#include "segment_manager/Segment.hpp"
#include "segment_manager/SegmentInventory.hpp"

typedef uint32_t SegmentID;

class SegmentManager {
private:
    BufferManager* bm;
    BufferFrame* siframe;
    BufferFrame* fssframe;
    SegmentInventory* si;
    FreeSpaceSegment* fss;
    std::unordered_map<SegmentID, Segment*> segments;
public:
    SegmentManager(BufferManager* bm);
    ~SegmentManager();
    SegmentID createSegment(SegmentType type, uint16_t numpages);
    Segment* retrieveSegment(SegmentID);
    template <typename S>
    S& getSegment(SegmentID id);
    void dropSegment(SegmentID id);
    void growSegment(SegmentID id, uint16_t deltanumpages);

};



#endif
