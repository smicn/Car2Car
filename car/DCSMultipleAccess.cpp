/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: DCSMultipleAccess.cpp
 *
 * Brief: DCSMultipleAccess means this strategy allows the cars from
 *     the same directions enter the bridge -- Distributed Critical
 *     Section -- at the same time.
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
#include "DCSMultipleAccess.h"
#include "BridgeProtocol.h"

#define DBG printf

void DCSMAMessageListener::onMessageReceived(int id, int msg, int timestamp)
{
	DBG ("dcsma.msg.listener: %d %d %d\n", id, msg, timestamp);
	
	if (NULL == dcs) {
		cout << "dcsma.msg.listener: fatal error, no dcs bounded!" << endl;
		return;
	}

	LamportLogicalClock *clock = dcs->clock;
	assert(clock != NULL);
	
	/* it is the critically important moment to drive the logical clock */
	clock->onTimestampReceived(timestamp);

	switch(msg) {
	case MessageListener::MESSAGE_REQUEST_DCS:
		cout << "Oh no! it's already deprecated in this class!" << endl;
		break;

	case DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMLEFT:
	case DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMRIGHT:
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
			/**
			 * The request from same direction with me will be acked right now,
			 * so that the cars with same direction can enter CS also.
			 */
			bool ackNow = false;
			if ((DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMLEFT == msg) && 
				(DCSMultipleAccess::DIRECTION_FROMLEFT == dcs->direction)) {
				ackNow = true;
			}
			else if ((DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMRIGHT == msg) && 
				(DCSMultipleAccess::DIRECTION_FROMRIGHT == dcs->direction)) {
				ackNow = true;
			}

			if (ackNow) {
				dcs->handler->send(id, DCSMAMessageListener::MESSAGE_ACKFROM_SAMEDIRECTION, clock->getTimestamp());
			}
			else {
				// the ack will be sent when i exit from the CS.
				dcs->acksFromMe[id] = true;
				if (DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMLEFT == msg) {
					dcs->requestDirections[id] = DCSMultipleAccess::DIRECTION_FROMLEFT;
				}
				else if (DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMRIGHT == msg) {
					dcs->requestDirections[id] = DCSMultipleAccess::DIRECTION_FROMRIGHT;
				}
			}
		}
		break;

	case DCSMAMessageListener::MESSAGE_ACKFROM_SAMEDIRECTION:
		if (dcs->state == DistributedCriticalSection::DCS_WAITING) {
			DBG ("dcsma.onMessageReceived(): i'm in CS with same direction car's ack!\n");
			dcs->state = DistributedCriticalSection::DCS_IN;
			clock->selfPlus();
		}
		break;
		
	case MessageListener::MESSAGE_ACK_REQUEST:
		/**
		 * LA3: A process enters its CS, when (1) ... and (2) it has received 
		 * the acks from every other process in response to its current request.
		 **/
		if (dcs->state != DistributedCriticalSection::DCS_WAITING) {
			cout << "dcsma.msg.listener: warning, why i got an ack?" << endl;
		}
		else {
			dcs->acks[id] = true;
			
			// now it's the time to enter the CS!
			if (dcs->isMyRequestAckedByEveryoneElse() && dcs->isMyRequestAtQueueHead()) {
				DBG ("dcsma.onMessageReceived(): i'm in CS with everyone's ack!\n");
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
				DBG ("dcsma.onMessageReceived(): i'm in CS due to someone release it!\n");
				dcs->state = DistributedCriticalSection::DCS_IN;
				clock->selfPlus();
			}
		}
		break;
		
	default:
		break;
	}
}

DCSMultipleAccess::DCSMultipleAccess(int id, int count) 
	: DistributedCriticalSection(id, count)
{
	requestDirections = new int[count];
	assert(requestDirections != NULL);
	for (int ii = 0; ii < count; ii++) {
		requestDirections[ii] = -1;
	}
	
	if (listener != NULL) {
		delete listener;
	}
	
	listener = new DCSMAMessageListener(this);
	assert(listener != NULL);

	assert(handler != NULL);
	handler->startReceive(listener);
}

DCSMultipleAccess::~DCSMultipleAccess()
{
	if (requestDirections != NULL) {
		delete[] requestDirections;
		requestDirections = NULL;
	}
}

void DCSMultipleAccess::request() 
{
	cout << "Sorry, already deprecated in this class!" << endl;
}

void DCSMultipleAccess::release() 
{
	DBG ("dcsma.release():\n");
	
	clock->selfPlus();
	
	requestQueue[id] = -1;

	int ts = clock->getTimestamp();

	state = DCS_OUT;
	
	handler->send(MessageListener::MESSAGE_RELEASE_DCS, ts);

	bool hasRequestFromSameDirection = false;
	/*
	 * if request(s) from same direction exist, ack to them.
	 */
	for (int ii = 0; ii < count; ii++) {
		if (acksFromMe[ii]) {
			if (requestDirections[ii] == this->direction) {
				hasRequestFromSameDirection = true;
				
				assert(ii != id);
				handler->send(ii, MessageListener::MESSAGE_ACK_REQUEST, ts);
			}
		}
	}
	/* 
	 * and the requests from the other direction are only acked
	 * when there is no same direction any more. */
	if (!hasRequestFromSameDirection) {
		for (int ii = 0; ii < count; ii++) {
			if (acksFromMe[ii]) {
				assert(ii != id);
				handler->send(ii, MessageListener::MESSAGE_ACK_REQUEST, ts);
			}
		}
	}
}

bool DCSMultipleAccess::isWaiting() 
{
	return DistributedCriticalSection::isWaiting();
}

bool DCSMultipleAccess::isInCSNow() 
{
	return DistributedCriticalSection::isInCSNow();
}

void DCSMultipleAccess::requestEx(int direction)
{
	DBG ("dcsma.requestEx(%d):\n", direction);

	assert((DIRECTION_FROMLEFT == direction) || (DIRECTION_FROMRIGHT == direction));
		
	if (state != DCS_OUT) {
		cout << "dcsma.requestEx() failed! state=" << state << endl;
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
		requestDirections[ii] = -1;
	}

	state = DCS_WAITING;

	this->direction = direction;

	if (DIRECTION_FROMLEFT == direction) {
		handler->send(DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMLEFT, ts);
	}
	else if (DIRECTION_FROMRIGHT == direction) {
		handler->send(DCSMAMessageListener::MESSAGE_REQUEST_DCSMS_FROMRIGHT, ts);
	}
	else {
		cout << "dcsma.requestEx(): unexpected direction!" << endl;
	}
}

bool DCSMultipleAccess::isMyRequestAtQueueHead()
{
	int myRequest = requestQueue[id];
	if (-1 == myRequest) return false;
	
	for (int ii = 0;  ii < count; ii++) {
		if (ii != id && (requestQueue[ii] != -1)) {
			/*
			 * only check the ones from different direction
			 */
			if (requestDirections[ii] != this->direction) {
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
	}
	
	return true;
}

bool DCSMultipleAccess::isMyRequestAckedByEveryoneElse()
{
	return DistributedCriticalSection::isMyRequestAckedByEveryoneElse();
}