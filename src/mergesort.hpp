/*
 * mergesort.h
 *
 *  Created on: 08.03.2013
 *      Author: swoehrl
 */

#ifndef MERGESORT_H_
#define MERGESORT_H_

void externalsort(const char* in, const char* out, uint64_t memsize);

bool checksort(const char* filename, uint64_t memsize);

#endif /* MERGESORT_H_ */
