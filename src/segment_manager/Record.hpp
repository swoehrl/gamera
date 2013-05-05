#ifndef RECORD_HPP_
#define RECORD_HPP_


class Record {
public:
   uint16_t len;
   char* data;
   Record& operator=(Record& rhs) = delete;
   Record(Record& t) = delete;
   Record(Record&& t) : len(t.len), data(t.data) {
      t.data = 0;
      t.len = 0;
   }
   explicit Record(uint16_t len, const char* const ptr) : len(len) {
      data = new char[len];
      memcpy(data, ptr, len);
   }
   explicit Record(uint16_t len, char* ptr) : len(len) {
    data = ptr;
   }
   const char* getData() const {
      return data;
   }
   unsigned getLen() const {
      return len;
   }
   ~Record() {
       delete[] data;
   }
};

/*
class Record {
public:
    uint16_t len;
    void* data;
    Record(uint16_t l, const void* const d) {
        len = l;
        data = d;
    };
};

*/

#endif


