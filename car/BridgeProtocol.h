#ifndef BRIDGE_PROTOCOL_H
#define BRIDGE_PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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
#include <pthread.h>
using namespace std;
#include <iostream>
#include <algorithm>
#include "LamportLogicalClock.h"
#include "LamportVectorClock.h"
#include "MessageHandler.h"
#include "DistributedCriticalSection.h"
#include "DCSMultipleAccess.h"

class BridgeProtocol {
public:
	static BridgeProtocol *getInstance();

	void initialize(int id, int count);

	DistributedCriticalSection *createDCS(const char *spec);

	LamportLogicalClock *getLogicalClock();

	LamportVectorClock *getVectorClock();
	
private:
	BridgeProtocol();

	int id;
	int count;
	
	static BridgeProtocol *inst;

	LamportLogicalClock *instLLC;

	LamportVectorClock *instLVC;

	DistributedCriticalSection *instDCS;

	DistributedCriticalSection *instDCSMA;
};

#endif
