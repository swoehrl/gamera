#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <iostream>
#include "buffermanager.hpp"

using namespace std;

BufferManager* bm;
unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;
unsigned* threadSeed;
volatile bool stop=false;

unsigned randomPage(unsigned threadNum) {
   // pseudo-gaussian, causes skewed access pattern
   unsigned page=0;
   for (unsigned  i=0; i<20; i++)
      page+=rand_r(&threadSeed[threadNum])%pagesOnDisk;
   return page/20;
}

static void* scan(void *arg) {
   // scan all pages and check if the counters are not decreasing
   unsigned counters[pagesOnDisk];
   for (unsigned i=0; i<pagesOnDisk; i++)
      counters[i]=0;

   while (!stop) {
      unsigned start = random()%(pagesOnDisk-10);
      for (unsigned page=start; page<start+10; page++) {
    	  //printf("scan page= %u\n", page);
    	  BufferFrame& bf = bm->fixPage(page, false);
    	  unsigned newcount = reinterpret_cast<unsigned*>(bf.getData())[0];
    	  assert(counters[page]<=newcount);
    	  counters[page]=newcount;
    	  bm->unfixPage(bf, false);
      }
   }

   return NULL;
}

static void* readWrite(void *arg) {
   // read or write random pages
   uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);

   uintptr_t count = 0;
   for (unsigned i=0; i<100000/threadCount; i++) {
      bool isWrite = rand_r(&threadSeed[threadNum])%128<10;
      BufferFrame& bf = bm->fixPage(randomPage(threadNum), isWrite);

      if (isWrite) {
         count++;
         reinterpret_cast<unsigned*>(bf.getData())[0]++;
      }
      bm->unfixPage(bf, isWrite);
   }
   //printf("readWrite leaving function\n");
   return reinterpret_cast<void*>(count);
}

int runtest(int pod, int pir, int tc, const char* dbfile) {
	pagesOnDisk = pod; //100;
	pagesInRAM = pir; //20;
	threadCount = tc; //8;


   threadSeed = new unsigned[threadCount];
   for (unsigned i=0; i<threadCount; i++)
      threadSeed[i] = i*97134;

   bm = new BufferManager(dbfile, pagesInRAM);

   pthread_t threads[threadCount];
   pthread_attr_t pattr;
   pthread_attr_init(&pattr);

   // set all counters to 0
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i, true);
      reinterpret_cast<unsigned*>(bf.getData())[0]=0;
      bm->unfixPage(bf, true);
   }

   // start scan thread
   pthread_t scanThread;
   pthread_create(&scanThread, &pattr, scan, NULL);

   // start read/write threads
   for (unsigned i=0; i<threadCount; i++)
      pthread_create(&threads[i], &pattr, readWrite, reinterpret_cast<void*>(i));

   // wait for read/write threads
   unsigned totalCount = 0;
   for (unsigned i=0; i<threadCount; i++) {
      void *ret;
      pthread_join(threads[i], &ret);
      totalCount+=reinterpret_cast<uintptr_t>(ret);
   }

   // wait for scan thread
   stop=true;
   pthread_join(scanThread, NULL);

   // restart buffer manager
   //delete bm;
   //bm->~BufferManager();
   delete bm;
   bm = new BufferManager(dbfile, pagesInRAM);
   
   // check counter
   unsigned totalCountOnDisk = 0;
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i,false);
      totalCountOnDisk+=reinterpret_cast<unsigned*>(bf.getData())[0];
      bm->unfixPage(bf, false);
   }
   delete bm;
   delete[] threadSeed;
   if (totalCount==totalCountOnDisk) {
      std::cout << "test successful" << endl;
      return 0;
   } else {
      std::cerr << "error: expected " << totalCount << " but got " << totalCountOnDisk << endl;
      return 1;
   }
}

/*
TEST(BufferManagerTest, SingleThreadTest) {
    EXPECT_EQ(runtest(100, 20, 1, "database.file"), 1);
}
*/

TEST(BufferManagerTest, 8ThreadTest) {
    EXPECT_EQ(runtest(100, 20, 8, "database.file"), 0);
}

TEST(BufferManagerTest, Simple) {
    BufferManager* bm = new BufferManager("database.file", 20);
    for (unsigned i=0; i < 100; i++) {
        BufferFrame& bf = bm->fixPage(i, false);
        bm->unfixPage(bf, true);
    }
    //bm->~BufferManager();
    delete bm;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}

