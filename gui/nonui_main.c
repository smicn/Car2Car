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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#define CAR_CNT 4

static const int COLOR_RED  = 0;
static const int COLOR_BLUE = 1;
static const int NUMBER_ONE = 1;
static const int NUMBER_TWO = 2;

extern int get_car_color(int id);
extern int get_car_number(int id);
extern int get_car_coordinate_x(int id);
extern int get_car_coordinate_y(int id);
#ifdef LOCAL_SIMULATOR
extern void start_simulator(void);
extern void stop_simulator(void);
#else
extern void launch_remote_cars(int direction);
extern void start_receive_heartbeat(void);
extern void stop_receive_heartbeat(void);
#endif

int main(int argc, char *argv[])
{
	int direction = 0;
	if (argc > 1) {
		direction = 1;
	}
	
#ifndef LOCAL_SIMULATOR
	start_receive_heartbeat();
	launch_remote_cars(direction);
#else
	start_simulator();
#endif

	while (1) {
		int ii;
		
		for (ii = 0; ii < CAR_CNT; ii++) {
			int color, number, x, y;

			color = get_car_color(ii);
			number = get_car_number(ii);
			x = get_car_coordinate_x(ii);
			y = get_car_coordinate_y(ii);

			printf("car[%d]%s_%02d_(%d,%d) ", ii, 
				(color == COLOR_RED) ? "Red" : "Blue", number, x, y);
		}

		printf("\n");
		usleep(500*1000);
	}

#ifndef LOCAL_SIMULATOR
	stop_receive_heartbeat();
#else
	stop_simulator();
#endif

	return 0;
}