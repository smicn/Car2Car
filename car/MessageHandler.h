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
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

class MessageListener {
public:
	static const int MESSAGE_REQUEST_DCS = 1;
	static const int MESSAGE_ACK_REQUEST = 2;
	static const int MESSAGE_RELEASE_DCS = 3;
	
	virtual void onMessageReceived(int id, int msg, int timestamp) = 0;
};

class MessageHandler {
public:
	MessageHandler(int id, int count);
	~MessageHandler();
	
	virtual void send(int id, int msg, int timestamp);
	virtual void send(int msg, int timestamp);

	virtual void startReceive(int id, MessageListener *listener);
	virtual void stopReceive(int id);
	
	virtual void startReceive(MessageListener *listener);
	virtual void stopReceive(void);

private:
	friend void *thread_proc_receiver(void *param);
	bool isThreadExit;
	pthread_t receiver;

	int *sockets;
	struct sockaddr_in *sads;
	static const char *ADDR_DEFAULT;
	static const int PORT_DEFAULT_START;
	
protected:
	int id;
	int count;

	MessageListener *listenerMulticast;
	MessageListener **listeners;
};

#endif
