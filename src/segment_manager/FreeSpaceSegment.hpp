#ifndef FREESPACESEGMENT_HPP_
#define FREEPSACESEGMENT_HPP_

#include <map>
#include <cstdint>

//#include "segment_manager/SegmentBase.hpp"
//#include "segment_manager/Segment.hpp"

// format in rawdata:
// 2-tupels of (startpage, length)


class FreeSpaceSegment {
private:
    void* pagedata;
    std::multimap<uint16_t, uint16_t> extents;

public:
    FreeSpaceSegment(void* page);
    bool getFreePages(uint16_t desiredlength, uint16_t& startpage, uint16_t& length);
    void freePages(uint16_t startpage, uint16_t length);
    void flush();
}; 

#endif

