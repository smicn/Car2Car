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
#ifndef DCS_H
#define DCS_H

class DistributedCriticalSection;

class DCSMessageListener : public MessageListener {
public:
	DCSMessageListener(DistributedCriticalSection *dcs) {
		this->dcs = dcs;
	}
	
	virtual void onMessageReceived(int id, int msg, int timestamp);

private:
	DistributedCriticalSection *dcs;
};

class DistributedCriticalSection {
public:
	DistributedCriticalSection(int id, int count);
	~DistributedCriticalSection();
	
	virtual void request();
	
	virtual void release();

	virtual bool isWaiting();

	virtual bool isInCSNow();

	friend class DCSMessageListener;
	
protected:
	bool isMyRequestAtQueueHead();
	bool isMyRequestAckedByEveryoneElse();
	
protected:
	int id;
	int count;

	int *requestQueue;
	bool *acks;
	bool *acksFromMe;
		
	LamportLogicalClock *clock;
	MessageHandler *handler;
	MessageListener *listener;

	static const int DCS_OUT = 0;
	static const int DCS_WAITING = 1;
	static const int DCS_IN = 2;

	int state;
};

#endif
