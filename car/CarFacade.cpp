/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: CarFacade.cpp
 *
 * Brief: the facade for a distributed car, actually each single process. 
 *     It is the place where the main() function is located.
 *
 * Licensed under the Academic Free License version 2.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
using namespace std;
#include <iostream>
#include <algorithm>
#include "LamportLogicalClock.h"
#include "MessageHandler.h"
#include "DistributedCriticalSection.h"
#include "DCSMultipleAccess.h"
#include "BridgeProtocol.h"
#include "Car.h"
#include "HeartbeatReporter.h"
#include "CarFacade.h"

#define DBG printf

CarFacade::CarFacade(int count, int id, int color, int number, int direction)
{
	if (direction) {
		const int initxy[][2] = {
			{-10, 5}, {-10, -5}, {10, 5}, {10, -5}
		};
		car = new Car(id, color, number, initxy[id][0], initxy[id][1]);
		assert(car != NULL);
	}
	else {
		car = new Car(id, color, number);
		assert(car != NULL);
	}

	BridgeProtocol *bp = BridgeProtocol::getInstance();
	assert(bp != NULL);
	
	bp->initialize(id, count);
	printf("[Car_%d]Welcome to Car2Car! (%d/%d)\n", id, id, count);

	car->ready(direction);
	car->setSpeed(1, 1);

	reporter = new HeartbeatReporter(car);
	assert(reporter != NULL);
}

CarFacade::~CarFacade()
{
	delete reporter;
	delete car;
}

void CarFacade::run(void)
{
	cout << "Car is runing.." << endl;
	
	while (true) {
		usleep(INTERVAL_STEP *1000);

		car->moveForwardOneStep();
	}
}

int main(int argc, char *argv[])
{
	int count = 0;
	int id = -1;
	int color = -1;
	int number = -1;
	int direction = -1;
	
	if (argc != 6) {
		printf("usage: %s <count> <id> <color> <number> <direction_enable>\n", argv[0]);
		exit(0);
	}
	else {
		count = atoi(argv[1]);
		id = atoi(argv[2]);
		
		if (0 == strcmp("RED", argv[3]) || 0 == strcmp("red", argv[3])) {
			color = Car::COLOR_RED;
		}
		else if (0 == strcmp("BLUE", argv[3]) || 0 == strcmp("blue", argv[3])) {
			color = Car::COLOR_BLUE;
		}
		else {
			printf("%s(): color=%s cannot be supported!\n", argv[0], argv[3]);
			exit(0);
		}

		number = atoi(argv[4]);
		if ((number != Car::NUMBER_ONE) && (number != Car::NUMBER_TWO)) {
			printf("%s(): number=%d cannot be supported!\n", argv[0], argv[4]);
			exit(0);
		}

		direction = atoi(argv[5]);
	}

	CarFacade *cf = new CarFacade(count, id, color, number, direction);

	sleep(3);
	
	cf->run();
	
	return 0;
}
