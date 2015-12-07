/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: DCSMultipleAccess.cpp
 *
 * Brief: This file provides the complete implementation of Lamport's
 *     Distributed Critical Section algorithm, which is described in
 *     the textbook: 
 *
 *		Ghosh, Sukumar (2014-07-14). Distributed Systems: An Algorithmic Approach, Second Edition 
 *			(Chapman & Hall/CRC Computer and Information Science Series) (Page 131). Chapman and Hall
 *			/CRC. Kindle Edition. 
 *
 *	 	"LA1: To request entry into its CS, a process sends a time-stamped request to every other 
 *		process in the system and also enters the request in its local Q.
 *
 *		LA2: When a process receives a request, it places it in its Q. If the process is not in its CS, 
 *		then it sends a time-stamped ack to the sender. Otherwise, it defers the sending of the 
 *		ack until its exit from the CS.
 *
 *		LA3: A process enters its CS, when (1) its request is ordered ahead of all other requests 
 *		(i.e., the time stamp of its own request is less than the time stamps of all other requests) 
 *		in its local Q and (2) it has received the acks from every other process in response to 
 *		its current request.
 *
 *		LA4: To exit from the CS, a process (1) deletes the request from its local queue and 
 *		(2)?sends a time-stamped release message to all the other processes.
 *
 *		LA5: When a process receives a release message, it removes the corresponding request 
 *		from its local queue."
 *		
 *        For more information of the question, please refer to the
 *     documents related to this project.
 *
 * Licensed under the Academic Free License version 2.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
using namespace std;
#include <iostream>
#include <algorithm>
#include "LamportLogicalClock.h"
#include "MessageHandler.h"
#include "DistributedCriticalSection.h"
#include "BridgeProtocol.h"

#define DBG printf

void DCSMessageListener::onMessageReceived(int id, int msg, int timestamp)
{
	DBG("DCSMessageListener.onMessageReceived_01(%d, %d, %d):\n", id, msg, timestamp);
	
	if (NULL == dcs) {
		cout << "dcs.msg.listener: fatal error, no dcs bounded!" << endl;
		return;
	}

	DBG("DCSMessageListener.onMessageReceived_02(%d, %d, %d):\n", id, msg, timestamp);

	LamportLogicalClock *clock = dcs->clock;
	assert(clock != NULL);
	
	/* it is the critically important moment to drive the logical clock */
	clock->onTimestampReceived(timestamp);

	switch(msg) {
	case MessageListener::MESSAGE_REQUEST_DCS:
		/**
		 * LA2: When a process receives a request, it places it in its Q. 
		 * If the process is not in its CS, then it sends a time-stamped 
		 * ack to the sender. Otherwise, it defers the sending of the ack 
		 * until its exit from the CS.
		 **/
		dcs->requestQueue[id] = timestamp;
		if (dcs->state != DistributedCriticalSection::DCS_IN) {
			dcs->handler->send(id, MessageListener::MESSAGE_ACK_REQUEST, clock->getTimestamp());
		}
		else {
			// the ack will be sent when i exit from the CS.
			dcs->acksFromMe[id] = true;
		}
		break;
		
	case MessageListener::MESSAGE_ACK_REQUEST:
		/**
		 * LA3: A process enters its CS, when (1) ... and (2) it has received 
		 * the acks from every other process in response to its current request.
		 **/
		if (dcs->state != DistributedCriticalSection::DCS_WAITING) {
			cout << "dcs.msg.listener: warning, why i got an ack?" << endl;
		}
		else {
			dcs->acks[id] = true;
			
			// now it's the time to enter the CS!
			if (dcs->isMyRequestAckedByEveryoneElse() && dcs->isMyRequestAtQueueHead()) {
				DBG ("dcs.onMessageReceived(): i'm in CS with everyone's ack!\n");
				dcs->state = DistributedCriticalSection::DCS_IN;
				clock->selfPlus();
			}
		}
		break;
		
	case MessageListener::MESSAGE_RELEASE_DCS:
		/**
		 * LA5: When a process receives a release message, it removes the 
		 * corresponding request from its local queue.
		 **/
		dcs->requestQueue[id] = -1;
		clock->selfPlus();

		/**
		 * LA3: A process enters its CS, when (1) its request is ordered 
		 * ahead of all other requests .. in its local Q and ...
		 **/
		if (DistributedCriticalSection::DCS_WAITING == dcs->state) {
			// now it's the time to enter the CS!
			if (dcs->isMyRequestAckedByEveryoneElse() && dcs->isMyRequestAtQueueHead()) {
				DBG ("dcs.onMessageReceived(): i'm in CS due to someone release it!\n");
				dcs->state = DistributedCriticalSection::DCS_IN;
				clock->selfPlus();
			}
		}
		break;
		
	default:
		break;
	}
}

DistributedCriticalSection::DistributedCriticalSection(int id, int count) 
{
	assert(count > 0);
	
	this->id = id;
	this->count = count;

	state = DCS_OUT;
	
	requestQueue = new int[count];
	assert(requestQueue != NULL);
	for (int ii = 0; ii < count; ii++) {
		requestQueue[ii] = -1;
	}

	acks = new bool[count];
	assert(acks != NULL);
	for (int ii = 0; ii < count; ii++) {
		acks[ii] = false;
	}

	acksFromMe = new bool[count];
	assert(acksFromMe != NULL);
	for (int ii = 0; ii < count; ii++) {
		acksFromMe[ii] = false;
	}
	
	clock = BridgeProtocol::getInstance()->getLogicalClock();
	assert(clock != NULL);
	
	handler = new MessageHandler(id, count);
	assert(handler != NULL);
	listener = new DCSMessageListener(this);
	assert(listener != NULL);
	
	handler->startReceive(listener);
}

DistributedCriticalSection::~DistributedCriticalSection() 
{
	if (requestQueue != NULL) {
		delete[] requestQueue;
		requestQueue = NULL;
	}
	if (acks != NULL) {
		delete[] acks;
		acks = NULL;
	}
	if (acksFromMe != NULL) {
		delete[] acksFromMe;
		acksFromMe = NULL;
	}
	if (handler != NULL) {
		handler->stopReceive();
		delete handler;
		handler = NULL;
	}
	if (listener != NULL) {
		delete listener;
		listener = NULL;
	}
}

void DistributedCriticalSection::request()
{
	DBG ("dcs.request():\n");
		
	if (state != DCS_OUT) {
		cout << "dcs.request() failed! state=" << state << endl;
		return;
	}

	clock->selfPlus();
	
	/**
	 * LA1: To request entry into its CS, a process sends a time-stamped 
	 * request to every other process in the system and also enters the 
	 * request in its local Q.
	 **/
	int ts = clock->getTimestamp();
	
	requestQueue[id] = ts;

	for (int ii = 0; ii < count; ii++) {
		acks[ii] = false;
		acksFromMe[ii] = false;
	}

	state = DCS_WAITING;

	handler->send(MessageListener::MESSAGE_REQUEST_DCS, ts);
}

void DistributedCriticalSection::release()
{
	DBG ("dcs.release():\n");
	
	clock->selfPlus();
	
	/**
	 * LA4: To exit from the CS, a process (1) deletes the request from 
	 * its local queue and (2)?sends a time-stamped release message to  
	 * all the other processes.
	 **/
	requestQueue[id] = -1;

	int ts = clock->getTimestamp();

	state = DCS_OUT;
	
	handler->send(MessageListener::MESSAGE_RELEASE_DCS, ts);

	/**
	 * LA2: ... Otherwise, it defers the sending of the ack 
	 * until its exit from the CS.
	 **/
	for (int ii = 0; ii < count; ii++) {
		if (acksFromMe[ii]) {
			assert(ii != id);
			handler->send(ii, MessageListener::MESSAGE_ACK_REQUEST, ts);
		}
	}
}

bool DistributedCriticalSection::isWaiting()
{
	return (DCS_WAITING == state);
}

bool DistributedCriticalSection::isInCSNow()
{
	return (DCS_IN == state);
}

bool DistributedCriticalSection::isMyRequestAtQueueHead()
{
	int myRequest = requestQueue[id];
	if (-1 == myRequest) return false;
	
	for (int ii = 0;  ii < count; ii++) {
		if (ii != id && (requestQueue[ii] != -1)) {
			/**
			 * what if the timestamps are same?? it will happen!! 
			 * but, guess what? no bad consequence! the process with
			 * smaller id will go first, because here i compare them
			 * by '<' rather than '<='. This is called "Total Order" 
			 * according to Lamport's.
			 **/
			if (requestQueue[ii] < myRequest) {
				return false;
			}
			else if (requestQueue[ii] == myRequest) {
				if (id < ii) {
					return false;
				}
			}
		}
	}
	
	return true;
}

bool DistributedCriticalSection::isMyRequestAckedByEveryoneElse()
{
	for (int ii = 0; ii < count; ii++) {
		if ((ii != id)) {
			if (!acks[ii])
				return false;
		}
	}
	return true;
}