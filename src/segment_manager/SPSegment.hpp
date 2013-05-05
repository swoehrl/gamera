#ifndef SPSEGMENT_HPP_
#define SPSEGMENT_HPP_

#include "segment_manager/SegmentBase.hpp"
#include "segment_manager/Segment.hpp"
#include "segment_manager/Record.hpp"

// Every page is organized into slots with 64 Bytes per Slot (minimum) -> every pages has 256 slots
// Layout of a page:
// 32 bytes header
// 32 bytes freespace bitmap
// 256 bytes slots
//
// slot ids 0-4 are reserved for metadata

const int RECORDSIZE = 64; // bytes
const int SLOTIDSIZE = 8; // bit
const int PAGEIDSIZE = 24; // bit
const int SLOTSOFFSET = 64; // bytes
const int FSOFFSET = 32; // bytes
const int RECORDSOFFSET = 5*RECORDSIZE; // bytes
const uint16_t NUMBEROFSLOTS = 256;

class SPSegment {
private:
    Segment* seg;
    TID insert(const Record& r, BufferFrame& bf, uint32_t pageid, uint8_t recordoffset, uint8_t slot, void* data);
public:
    SPSegment(Segment* seg);
    ~SPSegment();
    TID insert(const Record& r);
    bool remove(TID tid);
    Record* lookup(TID tid);
    bool update(TID tid, const Record& r);
};


#endif
