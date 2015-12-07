/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: MessageHandler.cpp
 *
 * Brief: This module provides the basic multicast messaging feature
 *     for the distributed cars based on UDP socket programming.
 *
 * Licensed under the Academic Free License version 2.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
#include <iostream>
#include "MessageHandler.h"
#include "../config"

#define DBG //printf

void *thread_proc_receiver(void *param)
{
	MessageHandler *thiz = (MessageHandler *)param;
	assert(thiz != NULL);
	int recvid = thiz->id;

	while (!thiz->isThreadExit) {
		char message[32];
		int len = sizeof(struct sockaddr_in);
		int ret = recvfrom(thiz->sockets[recvid], message, 
			sizeof(message), 0, (struct sockaddr *)&thiz->sads[recvid], &len);
		if (ret > 0) {
			int id = -1, msg = -1, timestamp = -1;
			sscanf(message, "%d%d%d", &id, &msg, &timestamp);

			DBG("[car%d]MessageHandler.onReceived(from=%d %d %d)\n", recvid, id, msg, timestamp);
			
			MessageListener *listener = thiz->listeners[id];
			assert(listener != NULL);
			if (listener != NULL) {
				listener->onMessageReceived(id, msg, timestamp);
			}
		}
	}
}

static const char * MessageHandler::ADDR_DEFAULT    = ADDR_ALL;
static const int MessageHandler::PORT_DEFAULT_START = PORT_P2P;
	
MessageHandler::MessageHandler(int id, int count)
{
	assert(count > 0);
	
	this->id = id;
	this->count = count;
	
	listeners = new MessageListener*[count];
	assert(listeners != NULL);
	for (int ii = 0; ii < count; ii++) {
		listeners[ii] = NULL;
	}

	sockets = new int[count];
	assert(sockets != NULL);
	sads = (struct sockaddr_in *)malloc(count * sizeof(struct sockaddr_in));
	assert(sads != NULL);
	for (int ii = 0; ii < count; ii++) {
		sockets[ii] = socket(AF_INET, SOCK_DGRAM, 0);
		assert(sockets[ii] != -1);
		
		bzero(&sads[ii], sizeof(sads[ii]));
		sads[ii].sin_family = AF_INET;
		if (ii != id) { // for clients
			sads[ii].sin_addr.s_addr = inet_addr(ADDR_DEFAULT);
		}
		else {          // for server
			sads[ii].sin_addr.s_addr = htonl(INADDR_ANY);
		}
		sads[ii].sin_port = htons(PORT_DEFAULT_START + ii);

		// the socket channel for myself as the receiver.
		if (ii == id) {
			int yes = 1;
			/* allow multiple sockets to use the same PORT number */
			int ret = setsockopt(sockets[ii], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
			if (-1 == ret) {
				cout << "message.handler: socket set sharing port failed!" << endl;
				exit(0);
			}
			
			/* bind address and port to socket */
			ret = bind(sockets[ii], (struct sockaddr *)&sads[ii], sizeof(sads[ii]));
			if (-1 == ret) {
				cout << "message.handler: socket bind failed!" << endl;
				exit(0);
			}
		}
	}

	isThreadExit = false;
	int ret = pthread_create(&receiver, NULL, thread_proc_receiver, (void *)this);
	assert(0 == ret);
}

MessageHandler::~MessageHandler()
{
	isThreadExit = true;
	pthread_join(receiver, NULL);

	for (int ii = 0; ii < count; ii++) {
		close(sockets[ii]);
	}

	free(sads);
	delete[] sockets;
	delete[] listeners;
}

void MessageHandler::send(int id, int msg, int timestamp)
{
	assert(id != this->id);
	assert(sockets[id] != NULL);
	
	char message[32];
	memset(message, 0, sizeof(message));
	sprintf(message, "%d %d %d", this->id, msg, timestamp);

	int len = sizeof(struct sockaddr_in);
	int ret = sendto(sockets[id], message, 
		sizeof(message), 0, (struct sockaddr *)&sads[id], len);
	assert(ret >= 0);

	DBG("[car%d]MessageHandler.sendto_car%d(%s)\n", this->id, id, message);
}

void MessageHandler::send(int msg, int timestamp)
{
	for (int ii = 0; ii < count; ii++) {
		assert(sockets[ii] != NULL);
		if (ii != id) {
			char message[32];
			memset(message, 0, sizeof(message));
			sprintf(message, "%d %d %d", this->id, msg, timestamp);

			int len = sizeof(struct sockaddr_in);
			int ret = sendto(sockets[ii], message, 
				strlen(message), 0, (struct sockaddr *)&sads[ii], len);
			assert(ret >= 0);
		}
	}

	DBG("[car%d]MessageHandler.sendBroadcast(%d %d)\n", this->id, msg, timestamp);
}

void MessageHandler::startReceive(int id, MessageListener *listener)
{
	assert(listener != NULL);
	assert(id != this->id);
	assert(sockets[id] != NULL);

	listeners[id] = listener;
}

void MessageHandler::stopReceive(int id)
{
	assert(id != this->id);
	assert(sockets[id] != NULL);

	listeners[id] = NULL;
}

void MessageHandler::startReceive(MessageListener *listener)
{
	for (int ii = 0; ii < count; ii++) {
		assert(sockets[ii] != NULL);
		if (ii != id) {
			listeners[ii] = listener;
			assert(listeners[ii] != NULL);
			DBG("[car%d]MessageHandler.registerListener(%d) successfully!\n", id, ii);
		}
	}
}

void MessageHandler::stopReceive(void)
{
	for (int ii = 0; ii < count; ii++) {
		listeners[ii] = NULL;
	}
}
