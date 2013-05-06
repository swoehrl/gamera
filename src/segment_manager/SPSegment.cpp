
#include <cstring>
#include <cstdio>

#include "segment_manager/SPSegment.hpp"


SPSegment::SPSegment(Segment* seg) {
    this->seg = seg;
}

SPSegment::~SPSegment() {
    delete seg;
}


TID SPSegment::insert(const Record& r, BufferFrame& bf, uint32_t pageid, uint8_t recordoffset, uint8_t slot, void* data) {
    //std::cout << "Pageid: " << pageid << ", slot: " << (int)slot << ", recordoffset: " << (int)recordoffset << " was inserted with record len " << r.len  << std::endl;
    uint16_t lenslots = r.len / 64 + (r.len%64 > 0 ? 1 : 0);
    //std::cout << "Number of slots: " << lenslots << std::endl;
    TID tid = (pageid << 8) | slot;
    uint8_t* slots = ((uint8_t*)data) + SLOTSOFFSET;
    uint16_t* records = ((uint16_t*)data) + RECORDSOFFSET/sizeof(uint16_t);
    uint64_t* freespace = ((uint64_t*)data) + FSOFFSET/sizeof(uint64_t);
    uint8_t byte = recordoffset/64;
    uint64_t* bitmap = freespace+byte;
    uint8_t offset = recordoffset%64;
    // mark records as used in freespace bitmap and set recordoffset in slots
    for (uint8_t i=0; i < lenslots; i++) {
        *bitmap |= (1 << offset);
        if (++offset > 63) {
            bitmap++;
            offset = 0;
        }
        slots[slot+i] = recordoffset;
    }
    // insert record data
    uint16_t* record = records+recordoffset*RECORDSIZE/sizeof(uint16_t);
    *record = r.len;
    record++;
    memcpy(record, r.data, r.len);
    seg->forgetPage(bf, true);
    return tid;
}


TID SPSegment::insert(const Record& r) {
    uint16_t lenslots = r.len / 64 + (r.len%64 > 0 ? 1 : 0);
    for (uint32_t pageid=0; pageid < seg->numPages(); pageid++) {
        BufferFrame& bf = seg->getPage(pageid, true);
        void* data = bf.getData();
        uint8_t* slots = ((uint8_t*)data) + SLOTSOFFSET;
        uint64_t* freespace = ((uint64_t*)data) + FSOFFSET/sizeof(uint64_t);
    
        uint16_t freeslots = 0;
        uint8_t byte = 0;
        uint64_t* bitmap = freespace;
        uint8_t offset = 5;
        for (uint8_t i=5; i < NUMBEROFSLOTS; i++) {
            if (((*bitmap & (1 << offset)) >> offset) == 0) {
                if (++freeslots >= lenslots) {
                    //std::cout << "Found lenslots: " << (int)lenslots << " at byte:" << (int)byte << " and offset: " << (int)offset << std::endl;
                    for (int j=5; j < NUMBEROFSLOTS; j++) {
                        if (slots[j] == 0) {
                            //std::cout << "Slot: " << j << std::endl;
                            return insert(r, bf, pageid, byte*64+offset, j, data);
                        }
                    }
                    break;
                }
            } else {
                freeslots = 0;
            }
            if (++offset > 63) {
                bitmap++;
                byte++;
                offset = 0;
            }
        }
        seg->forgetPage(bf, false);
    }
    uint32_t pageid = seg->numPages();
    seg->grow(1);
    BufferFrame& bf = seg->getPage(pageid, true);
    return insert(r, bf, pageid, 0, 5, bf.getData());
}


bool SPSegment::remove(TID tid) {
    uint32_t pageid = tid >> SLOTIDSIZE;
    uint8_t slotid = (tid << PAGEIDSIZE) >> PAGEIDSIZE;
    BufferFrame& bf = seg->getPage(pageid, true);
    void* data = bf.getData();
    uint64_t* freespace = ((uint64_t*)data) + FSOFFSET/sizeof(uint64_t);
    uint8_t* slots = ((uint8_t*)data) + SLOTSOFFSET;
    uint16_t* records = ((uint16_t*)data) + RECORDSOFFSET/sizeof(uint16_t);
    uint8_t slotoffset = slots[slotid];
    if (slotoffset == 0) return nullptr; // slot is empty
    uint16_t len = records[slotoffset];
    len = len / RECORDSIZE + (len%RECORDSIZE > 0 ? 1 : 0);
    uint8_t byte = slotoffset + len/RECORDSIZE;
    uint64_t* bitmap = freespace+byte;
    uint8_t offset = len%RECORDSIZE;
    for (uint16_t i=0; i < len; i++) { 
        slots[slotid+i] = 0;
        *bitmap &= ~(1 << offset);
        if (++offset > 63) {
            bitmap++;
            offset = 0;
        }
    }
    seg->forgetPage(bf, true);
    return true;
}


Record* SPSegment::lookup(TID tid) {
    uint32_t pageid = tid >> 8;
    uint8_t slotid = (tid << 24) >> 24;
    //std::cout << "Looking up: pageid: " << pageid << ", slotid: " << (int)slotid << std::endl;
    BufferFrame& bf = seg->getPage(pageid, false);
    void* data = bf.getData(); 
    uint8_t* slots = ((uint8_t*)data) + 64;
    uint16_t* records = ((uint16_t*)data) + 5*64/sizeof(uint16_t);
    uint8_t recordoffset = slots[slotid];
    //std::cout << "Recordoffset: " << (int)recordoffset << std::endl;
    if (recordoffset == 0) return nullptr; // slot is empty
    uint16_t* record = records+recordoffset*RECORDSIZE/sizeof(uint16_t);
    uint16_t len = *record;
    char* recorddata = new char[len];
    memcpy(recorddata, record+1, len);
    Record* r = new Record(len, recorddata);
    seg->forgetPage(bf, false);
    return r;
}


bool SPSegment::update(TID tid, const Record& r) {
    uint32_t pageid = tid >> 8;
    uint8_t slotid = (tid << 24) >> 24;
    BufferFrame& bf = seg->getPage(pageid, true);
    void* data = bf.getData(); 
    uint8_t* slots = ((uint8_t*)data) + 64 ;
    uint16_t* records = ((uint16_t*)data) + 5*64/sizeof(uint16_t);
    uint8_t recordoffset = slots[slotid];
    if (recordoffset == 0) return false; // slot is empty
    uint16_t* record = records+recordoffset*RECORDSIZE/sizeof(uint16_t);
    uint16_t oldlen = *record;
    if (r.len / 64 + (r.len%64 > 0 ? 1 : 0) > oldlen / 64 + (oldlen%64 > 0 ? 1 : 0)) // resizing is not implemented
        return false;
    *record = r.len;
    record++;
    memcpy(record, r.data, r.len);
    seg->forgetPage(bf, true);
    return true;
}


