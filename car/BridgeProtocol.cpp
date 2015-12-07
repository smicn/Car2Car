/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: BridgeProtocol.cpp
 *
 * Brief: a singleton that holds other important instances like
 *      DistributedCriticalSection and LamportLogicalClock.
 *
 * Licensed under the Academic Free License version 2.1
 */
#include "BridgeProtocol.h"

static BridgeProtocol * BridgeProtocol::inst = NULL;

BridgeProtocol::BridgeProtocol()
{
	instLLC = NULL;
	instLVC = NULL;
	instDCS = NULL;
	instDCSMA = NULL;
}

BridgeProtocol *BridgeProtocol::getInstance() 
{
	if (NULL == inst) {
		inst = new BridgeProtocol();
		assert(inst != NULL);
	}
	return inst;
}

void BridgeProtocol::initialize(int id, int count) 
{
	assert(count > 0);
	
	inst->id = id;
	inst->count = count;
}

DistributedCriticalSection *BridgeProtocol::createDCS(const char *spec) 
{
	assert(count > 0);
	
	if (0 == strcmp("multiple", spec)) {
		if (NULL == instDCSMA) {
			instDCSMA = new DCSMultipleAccess(id, count);
		}
		return instDCSMA;
	}
	else {
		if (NULL == instDCS) {
			instDCS =  new DistributedCriticalSection(id, count);
		}
		return instDCS;
	}
}

LamportLogicalClock *BridgeProtocol::getLogicalClock() 
{
	if (NULL == instLLC) {
		instLLC = new LamportLogicalClock();
		assert(instLLC != NULL);
	}
	return instLLC;
}

LamportVectorClock *BridgeProtocol::getVectorClock() 
{
	if (NULL == instLVC) {
		instLVC = new LamportVectorClock(id, count);
		assert(instLVC != NULL);
	}
	return instLVC;
}
