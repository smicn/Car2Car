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
#ifndef LOCAL_SIMULATOR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void launch_remote_cars(int direction)
{
	struct {
		const char *count;
		const char *id;
		const char *color;
		const char *number;
	} cars[] = {
		{ "4", "0", "RED",  "1" },
		{ "4", "1", "RED",  "2" },
		{ "4", "2", "BLUE", "1" },
		{ "4", "3", "BLUE", "2" }
	};
	const char *directions[] = {"0", "1"};
	int ii;

	for (ii = 0; ii < sizeof(cars)/sizeof(cars[0]); ii++) {
		int ret = fork();
		if (0 == ret) {
			execlp("./car", "car", cars[ii].count, cars[ii].id, cars[ii].color, cars[ii].number, directions[direction], NULL);
			printf("fatal error: process car[%d] might crash!\n", ii);
			exit(-1);
		}
	}
}

void kill_remote_cars(void) 
{
	int ret = fork();
	if (0 == ret) {
		printf("it is to kill all the remote cars!\n");
		execlp("/usr/bin/killall", "killall", "-9", "car", NULL);
		exit(0);
	}
	sleep(1);
}
#endif
