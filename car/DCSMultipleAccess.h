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
#ifndef DCS_MULTIPLEACCESS_H
#define DCS_MULTIPLEACCESS_H

class DCSMultipleAccess;

class DCSMAMessageListener : public MessageListener {
public:
	static const int MESSAGE_REQUEST_DCSMS_FROMLEFT  = 8;
	static const int MESSAGE_REQUEST_DCSMS_FROMRIGHT = 9;
	static const int MESSAGE_ACKFROM_SAMEDIRECTION   = 10;
	
	DCSMAMessageListener(DCSMultipleAccess *dcs) {
		this->dcs = dcs;
	}
	
	virtual void onMessageReceived(int id, int msg, int timestamp);

private:
	DCSMultipleAccess *dcs;
};

class DCSMultipleAccess : public DistributedCriticalSection {
public:
	DCSMultipleAccess(int id, int count);
	~DCSMultipleAccess();

	virtual void request();
	virtual void release();
	virtual bool isWaiting();
	virtual bool isInCSNow();

	static const int DIRECTION_FROMLEFT  = 1;
	static const int DIRECTION_FROMRIGHT = 2;
	
	void requestEx(int direction);

	friend class DCSMAMessageListener;

protected:
	bool isMyRequestAtQueueHead();
	bool isMyRequestAckedByEveryoneElse();
	
private:
	int direction;

	int *requestDirections;
};

#endif
