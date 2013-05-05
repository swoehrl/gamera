#ifndef SEGMENT_HPP_
#define SEGMENT_HPP_

#include <cstdint>
#include <vector>
#include <map>

#include "segment_manager/SegmentBase.hpp"
#include "segment_manager/SegmentManager.hpp"
#include "buffermanager.hpp"


class Segment {
private:
    BufferManager* bm;
    SegmentManager* sm;
    ExtentList* extents;
    SegmentID id;
public:
    Segment(SegmentID id, BufferManager* bm, SegmentManager* sm, ExtentList* extents);
    void grow(uint16_t deltapages);
    uint64_t size();
    uint32_t numPages();
    BufferFrame& getPage(uint16_t index, bool exclusive);
    void forgetPage(BufferFrame& bf, bool dirty);
    void* operator*();
    void* operator++();
    void reset();
};

#endif
