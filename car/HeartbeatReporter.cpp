/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: HeartbeatReporter.cpp
 *
 * Brief: This module is responsible to reporting the heart beat of
 *     the distributed car (a single process) to the frontend system.
 *     It is actually a background thread using socket communication
 *     with the frontend process.
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
#include <iostream>
using namespace std;
#include "LamportLogicalClock.h"
#include "MessageHandler.h"
#include "DistributedCriticalSection.h"
#include "BridgeProtocol.h"
#include "Car.h"
#include "HeartbeatReporter.h"
#include "../config"

#define DBG //printf

void *thread_proc_reporter(void *param)
{
	HeartbeatReporter *thiz = (HeartbeatReporter *)param;
	assert(thiz != NULL);

	Car *car = thiz->car;
	assert(car != NULL);

	while (!thiz->isThreadExit) {
		usleep(thiz->HEARTBEAT_INTERVAL * 1000);

		int id = car->getID();
		int color = car->getColor();
		int number = car->getNumber();
		
		int x = car->getX();
		int y = car->getY();

		char message[64];
		sprintf(message, "car[%d]: color=%d number=%d x=%d y=%d",
			id, color, number, x, y);

		DBG("Heartbeat-Reporter.send: %s\n", message);
		
		int len = sizeof(struct sockaddr_in);
		int ret = sendto(thiz->sock, message, 
			strlen(message), 0, (struct sockaddr *)&thiz->sad, len);
		assert(ret >= 0);
	}
}

static const char *HeartbeatReporter::ADDR_DEFAULT = ADDR_ALL;
static const int   HeartbeatReporter::PORT_DEFAULT = PORT_REPORT;

HeartbeatReporter::HeartbeatReporter(Car *car)
{
	this->car = car;
	isThreadExit = false;

	bzero(&sad, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(ADDR_DEFAULT);
	sad.sin_port = htons(PORT_DEFAULT);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(socket != -1);

	printf("[Car_%d] heartbeat sender is ready!\n", car->getID());
	
	int ret = pthread_create(&thread, NULL, thread_proc_reporter, (void *)this);
	assert(0 == ret);
}

HeartbeatReporter::~HeartbeatReporter()
{
	isThreadExit = true;
	pthread_join(thread, NULL);
	close(sock);
}
