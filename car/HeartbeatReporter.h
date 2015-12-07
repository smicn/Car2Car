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
#ifndef HEARTBEAT_REPORTER_H
#define HEARTBEAT_REPORTER_H

class HeartbeatReporter {
public:
	HeartbeatReporter(Car *car);
	~HeartbeatReporter();

private:
	const int HEARTBEAT_INTERVAL = 50;//ms
	
	Car *car;
	
	friend void *thread_proc_reporter(void *param);
	pthread_t thread;
	bool isThreadExit;

	struct sockaddr_in sad;
	int sock;
	
	static const char *ADDR_DEFAULT;
	static const int PORT_DEFAULT;
};

#endif
