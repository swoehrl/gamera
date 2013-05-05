#ifndef SEGMENTBASE_HPP_
#define SEGMENTBASE_HPP_

#include <cstdint>
#include <iostream>
#include <vector>

class Segment;
class SegmentInventory;
class FreeSpaceSegment;
class SegmentManager;
class SPSegment;
class Record;

const uint16_t MAXNUMPAGES = 16*1024;
const int LOCAL_PAGESIZE = 16*1024; //16kb

// 24 Bit PageID, 8 Bit SlotID
typedef uint32_t TID;

typedef uint32_t SegmentID;

// pair<startpage, length>
typedef std::pair<uint16_t, uint16_t> Extent;
typedef std::vector<Extent> ExtentList;


enum class SegmentType {
    SI,
    SP,
};

#endif

