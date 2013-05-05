#include <iostream>

#include <gtest/gtest.h>

#include "segment_manager/SegmentManager.hpp"
#include "segment_manager/SPSegment.hpp"
#include "segment_manager/Record.hpp"

/*
TEST(SegmentManagerTest, SimpleTest) {
    BufferManager* bm = new BufferManager("segmentdatabase.file", 100);
    SegmentManager* sm = new SegmentManager(bm);
    SegmentID id = sm->createSegment(SegmentType::SP, 100);
    EXPECT_EQ(id, (uint16_t)4); 
    EXPECT_ANY_THROW(sm->createSegment(SegmentType::SP, 18000));
    delete sm;
    //std::cout << "Calling bm destructor\n";
    delete bm;
}
*/

TEST(SegmentManagerTest, SP) {
    BufferManager* bm = new BufferManager("segmentdatabase.file", 100);
    SegmentManager* sm = new SegmentManager(bm);
    SegmentID id = (uint32_t)8;//sm->createSegment(SegmentType::SP, 10);
    SPSegment sp(sm->retrieveSegment(id));
    std::string s1 = "Hello World";
    std::string s2 = "Hello World!";
    TID tid = sp.insert(Record(12, s1.c_str()));
    Record* r = sp.lookup(tid);
    ASSERT_EQ(12, r->len);
    ASSERT_STREQ(s1.c_str(), r->data);
    delete r;
    r = new Record(13, s2.c_str());
    sp.update(tid, *r);
    ASSERT_EQ(13, r->len);
    ASSERT_STREQ(s2.c_str(), r->data);
    delete r;
    char* data = new char[129];
    data[0] = 1;
    data[64] = 1;
    data[128] = 1;
    r = new Record(129, data);
    tid = sp.insert(*r);
    delete r;
    r = sp.lookup(tid);
    ASSERT_EQ(129, r->len);
    ASSERT_EQ(1, r->data[0]);
    ASSERT_EQ(1, r->data[64]);
    ASSERT_EQ(1, r->data[128]);
    delete r;
    delete sm;
    delete bm;
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
