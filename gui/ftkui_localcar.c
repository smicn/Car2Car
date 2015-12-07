/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: ftkui_localcar.c
 *
 * Brief: This module is to simulate the "Distribued Car" by "Local Car".
 *    Why should I need this? Well, this is done using Single-process but
 *    Multi-threading. If my distributed cars work correctly, the behaviors
 *    should be exactly the same. This file provides such an object of 
 *    reference.
 *
 * Licensed under the Academic Free License version 2.1
 */
#ifdef LOCAL_SIMULATOR
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define CAR_CNT 4

static const int COLOR_RED  = 0;
static const int COLOR_BLUE = 1;
static const int NUMBER_ONE = 1;
static const int NUMBER_TWO = 2;

static const int STATE_OUTCS = 0;
static const int STATE_WAITING = 1;
static const int STATE_INCS = 2;

struct FtkCarSignalSimulator {
	pthread_t thread[CAR_CNT];
	int running[CAR_CNT];
	
	int color[CAR_CNT];
	int number[CAR_CNT];
	int x[CAR_CNT], y[CAR_CNT];
	
	int speed, speedOnBridge;

	pthread_mutex_t cs;
	int state[CAR_CNT];
	
} Simulator;

static int isWaitingForCriticalSection(int id)
{
	if (STATE_WAITING == Simulator.state[id]) {
		if (0 == pthread_mutex_trylock(&Simulator.cs)) {
			Simulator.state[id] = STATE_INCS;
			printf("car[%d] is on bridge!\n", id);
		}
	}
	return (STATE_WAITING == Simulator.state[id]) ? 1 : 0;
}

static void requestCriticalSection(int id)
{
	if (0 == pthread_mutex_trylock(&Simulator.cs)) {
		Simulator.state[id] = STATE_INCS;
		printf("car[%d] is on bridge!\n", id);
	}
	else {
		Simulator.state[id] = STATE_WAITING;
	}
}

static void releaseCriticalSection(int id)
{
	if (STATE_INCS == Simulator.state[id]) {
		Simulator.state[id] = STATE_OUTCS;
		pthread_mutex_unlock(&Simulator.cs);
		printf("car[%d] leaves bridge!\n", id);
	}
}

static void moveForwardOneStep(int id) 
{
	const int speed = Simulator.speed;
	const int speedOnBridge = Simulator.speedOnBridge;
	int *x = &Simulator.x[id];
	int *y = &Simulator.y[id];
	
	/* sofar, waiting for CS is the only reason to pause */
	if (isWaitingForCriticalSection(id)) {
		return;
	}
	
	/* the car is at the left side of the bridge */
	if (-20 == *x) {
		if (-5 == *y) {
			*x += speed;
		}
		else if (-5 < *y && *y <= 5) {
			*y -= speed;
		}
		else {
			assert(-5 <= *y && *y <= 5);
		}
	}
	else if (-20 < *x && *x < -10) {
		if (-5 == *y) {
			*x += speed;
		}
		else if (5 == *y) {
			*x -= speed;
		}
		else {
			assert(-5 == *y || 5 == *y);
		}
	}
	else if (-10 == *x) {
		if (5 == *y) {
			*x -= speed;
		}
		else if ((-5 <= *y && *y < -1) || (1 < *y && *y < 5)) {
			*y += speed;
		}
		else if (-1 == *y) {
			/* the right position to request CS */
			requestCriticalSection(id);
			*x += speedOnBridge;
		}
		else if (1 == *y) {
			/* the right position to release CS */
			releaseCriticalSection(id);
			*y += speed;
		}	
	}

	/* the car is now crossing the bridge */
	else if (-10 < *x && *x < 10) {
		//assert(isInCriticalSection());
		
		if (-1 == *y) {
			*x += speedOnBridge;
		}
		else if (1 == *y) {
			*x -= speedOnBridge;
		}
	}

	/* the car is at the right side of the bridge */
	else if (10 == *x) {
		/* just oppsite with what is doing at the left side */
		if (-5 == *y) {
			*x += speed;
		}
		else if ((-5 < *y && *y < -1) || (1 < *y && *y <= 5)) {
			*y -= speed;
		}
		else if (-1 == *y) {
			/* the right position to release CS */
			releaseCriticalSection(id);
			*y -= speed;
		}
		else if (1 == *y) {
			/* the right position to request CS */
			requestCriticalSection(id);
			*x -= speedOnBridge;
		}
	}
	else if (10 < *x && *x < 20) {
		/* same as what is doing at left side */
		if (-5 == *y) {
			*x += speed;
		}
		else if (5 == *y) {
			*x -= speed;
		}
		else {
			assert(-5 == *y || 5 == *y);
		}
	}
	else if (20 == *x) {
		/* somewhat oppsite with what is doing at the left side */
		if (5 == *y) {
			*x -= speed;
		}
		else if (-5 <= *y && *y < 5) {
			*y += speed;
		}
		else {
			assert(-5 <= *y && *y <= 5);
		}
	}
	else {
		assert(-20 <= *x && *x <= 20);
	}
}

static void *thread_proc_ftkui_localcar(void *param)
{
	int id = (int)param;
	
	while (Simulator.running) {
		
		moveForwardOneStep(id);
		
		usleep(50 *1000);
	}
}

void start_simulator(void)
{
	int ret = 0;
	int yes = 0;
	int ii;
	
	memset(&Simulator, 0, sizeof(Simulator));
	Simulator.speed = Simulator.speedOnBridge = 1;
	
	Simulator.x[0] = -15;
	Simulator.y[0] = 5;
	Simulator.color[0] = COLOR_RED;
	Simulator.number[0] = NUMBER_ONE;
	
	Simulator.x[1] = -15;
	Simulator.y[1] = -5;
	Simulator.color[1] = COLOR_RED;
	Simulator.number[1] = NUMBER_TWO;

	Simulator.x[2] = 15;
	Simulator.y[2] = 5;
	Simulator.color[2] = COLOR_BLUE;
	Simulator.number[2] = NUMBER_ONE;
	
	Simulator.x[3] = 15;
	Simulator.y[3] = -5;
	Simulator.color[3] = COLOR_BLUE;
	Simulator.number[3] = NUMBER_TWO;

	ret = pthread_mutex_init(&Simulator.cs, NULL);
	assert(0 == ret);
	
	for (ii = 0; ii < CAR_CNT; ii++) {
		Simulator.running[ii] = 1;
		ret = pthread_create(&Simulator.thread[ii], NULL, thread_proc_ftkui_localcar, (void *)ii);
		assert(0 == ret);
	}
}

void stop_simulator(void)
{
	int ret = -1;
	int ii;

	for (ii = 0; ii < CAR_CNT; ii++) {
		Simulator.running[ii] = 0;
		usleep(500 *1000);
		ret = pthread_join(Simulator.thread[ii], NULL);
		assert(0 == ret);
	}

	ret = pthread_mutex_destroy(&Simulator.cs);
	assert(0 == ret);
}

int get_car_color(int id)
{
	return Simulator.color[id];
}

int get_car_number(int id)
{
	return Simulator.number[id];
}

int get_car_coordinate_x(int id)
{
	return Simulator.x[id];
}

int get_car_coordinate_y(int id)
{
	return Simulator.y[id];
}
#endif