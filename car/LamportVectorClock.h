#ifndef LAMPORT_VECTORT_CLOCK_H
#define LAMPORT_VECTORT_CLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;
#include <limits.h>
#include <algorithm>

class LamportVectorClock {
public:
	static const int VTCMP_LESS = -1;
	static const int VTCMP_EQUAL = 0;
	static const int VTCMP_LARGER = 1;
	static const int VTCMP_CONCURRENT = (int)INT_MAX;

	LamportVectorClock(int id, int count) {
		assert(0 <= id && id < count);
		this->id = id;
		this->count = count;

		vts = new int[count];
		assert(vts != NULL);
		memset(vts, 0, count*sizeof(int));
	}

	~LamportVectorClock() {
		if (vts != NULL) {
			delete[] vts;
			vts = NULL;
		}
	}

	int getSelf() {
		return vts[id];
	}

	int getTimestamp(int id) {
		return vts[id];
	}

	const int *getTimestamps() {
		return (const int *)vts;
	}

	void selfPlus() {
		vts[id]++;
	}

	void onTimestampReceived(int id, int timestamp) {
		vts[id] = max(vts[id], timestamp);
	}

	int compare(const LamportVectorClock *other) {
		int ret0 = compareTimestamp(this->vts[0], other->vts[0]);
		for (int ii = 1; ii < count; ii++) {
			int ret = compareTimestamp(this->vts[ii], other->vts[ii]);
			if (ret != ret0) {
				if (VTCMP_EQUAL == ret0) {
					ret0 = ret;
				}
				else {
					if ((VTCMP_LESS == ret0 && VTCMP_LARGER == ret) ||
						(VTCMP_LESS == ret && VTCMP_LARGER == ret0)) {
						return VTCMP_CONCURRENT;
					}
				}
			}
		}
		return ret0;
	}

private:
	int compareTimestamp(int left, int right) {
		if (left < right)  return VTCMP_LESS;
		if (left == right) return VTCMP_EQUAL;
		if (left > right)  return VTCMP_LARGER;
	}
		
private:
	int *vts;
	int count;
	int id;
};

#endif
