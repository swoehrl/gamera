/*
 * segments.hpp
 *
 *  Created on: 12.03.2013
 *      Author: swoehrl
 */

#ifndef SEGMENTS_HPP_
#define SEGMENTS_HPP_
#include <inttypes.h>
#include "buffermanager.hpp"


class Extent {
public:
	uint pages[100];
};


class Segment {
	Extent extents[100];
};

class SegmentInventory {
	Segment* segments;
};

class SegmentManager {
	BufferManager* bm;
public:
	Segment& create();



};


#endif /* SEGMENTS_HPP_ */
