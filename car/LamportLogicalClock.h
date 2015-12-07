/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * This is a programming project in my Distributed Systems course. The purpose 
 * of this project is to better understand the algorithms for Distributed 
 * Mutual Exclusion. Lamport's algorithm for mutual exclusion is chosen for my 
 * program. 
 * This program consists of two parts. The "Car" runs at backend and its modules 
 * include several socket message channels, Lamport's logical clock, Lamport's 
 * Mutual Exclusion algorithm and its modified version for the second part of 
 * this question, and car's basic functionality movement. The other part is a GUI 
 * application based on a 3rdparty GUI system called FTK, which is an open source 
 * cross-platform GUI tool that I was once familiar. These two parts of programs 
 * are actually independent but highly integrated together. This program can run
 * on any Linux distribution and it was test on Ubuntu. 
 *
 * Licensed under the Academic Free License version 2.1
 */
#ifndef LAMPORT_LOGICAL_CLOCK_H
#define LAMPORT_LOGICAL_CLOCK_H

class LamportLogicalClock {
public:
	static const int LTCMP_LESS   = -1;
	static const int LTCMP_EQUAL  = 0;
	static const int LTCMP_LARGER = 1;
	
	LamportLogicalClock() {
		timestamp = 0;
	}

	int getSelf() {
		return timestamp;
	}

	int getTimestamp() {
		return timestamp;
	}
	
	void selfPlus() {
		timestamp++;
	}

	void onTimestampReceived(int timestamp) {
		this->timestamp = max(this->timestamp, timestamp) + 1;
	}

	int compare(int timestamp) {
		if (this->timestamp < timestamp)  return LTCMP_LESS;
		if (this->timestamp == timestamp) return LTCMP_EQUAL;
		if (this->timestamp > timestamp)  return LTCMP_LARGER;
	}
		
private:
	int timestamp;
};

#endif
