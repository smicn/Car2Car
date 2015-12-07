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
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../config"

#define DBG printf

static const char *ADDR_DEFAULT = ADDR_ALL;
static const int   PORT_DEFAULT = PORT_REPORT;

struct FtkCarSignalReceiver {
	struct sockaddr_in sad;
	int socket;

	pthread_t thread;
	int running;
	
	int color[4];
	int number[4];
	int x[4], y[4];
} Receiver;

static void *thread_proc_ftkui_receiver(void *param)
{
	while (Receiver.running) {
		char message[64];
		int len = sizeof(struct sockaddr_in);
		int ret = -1;

		memset(message, 0, sizeof(message));
		ret = recvfrom(Receiver.socket, message, 
			sizeof(message), 0, (struct sockaddr *)&Receiver.sad, (socklen_t *)&len);
		if (ret > 0)
			DBG ("FtkUI.receiver()_o: %s\n", message);
		if (ret > 0) {
			int id = -1;
			char *str = strtok(message, " ");
			while (str != NULL) {
				//
				// for its corresponding code in sender side is like:
				//
				// sprintf(message, "car[%d]: color=%d number=%d x=%d y=%d",
				//	 &id, &color, &number, &x, &y);
				// 
				// we have to parse the message like this way.
				
				if (0 == strncmp(str, "car[", strlen("car["))) {
					char carid[8] = {0};
					char *dest = carid;
					char *src = &str[strlen("car[")];
					while (*src != ']') {
						*dest++ = *src++;
					}
					id = atoi(carid);
					DBG ("parser: car_id: %s, %s, so id=%d\n", str, carid, id);
				}
				else if (0 == strncmp(str, "color=", strlen("color="))) {
					Receiver.color[id] = atoi(&str[strlen("color=")]);
					DBG ("parser: color: %s, id=%d, so c=%d\n", str, id, Receiver.color[id]);
				}
				else if (0 == strncmp(str, "number=", strlen("number="))) {
					Receiver.number[id] = atoi(&str[strlen("number=")]);
					DBG ("parser: number: %s, id=%d, so n=%d\n", str, id, Receiver.number[id]);
				}
				else if (0 == strncmp(str, "x=", strlen("x="))) {
					Receiver.x[id] = atoi(&str[strlen("x=")]);
					DBG ("parser: xx: %s, id=%d, so x=%d\n", str, id, Receiver.x[id]);
				}
				else if (0 == strncmp(str, "y=", strlen("y="))) {
					Receiver.y[id] = atoi(&str[strlen("y=")]);
					DBG ("parser: yy: %s, id=%d, so y=%d\n", str, id, Receiver.y[id]);
				}
				else {
					assert(0);
				}
				str = strtok(NULL, " ");
			}

			DBG ("FtkUI.receiver()_p: car_%d: color=%d number=%d x=%d y=%d\n", \
				id, Receiver.color[id], Receiver.number[id], Receiver.x[id], Receiver.y[id]);
			// it's time to post it to GUI and let it show,
			// or, let GUI pull it by itself.
		}
	}
}

void start_receive_heartbeat(void)
{
	int ret = 0;
	int yes = 1;
	
	memset(&Receiver, 0, sizeof(Receiver));
	
	bzero(&Receiver.sad, sizeof(Receiver.sad));
	Receiver.sad.sin_family = AF_INET;
	Receiver.sad.sin_addr.s_addr = htonl(INADDR_ANY);
	Receiver.sad.sin_port = htons(PORT_DEFAULT);

	Receiver.socket = socket(AF_INET, SOCK_DGRAM, 0);
	assert(Receiver.socket != -1);

	// allow multiple sockets to use the same PORT number
	ret = setsockopt(Receiver.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	assert(0 == ret);
	//assert(1 == yes);

	ret = bind(Receiver.socket, (struct sockaddr *)&Receiver.sad, sizeof(Receiver.sad));
	assert(0 == ret);

	printf("[ftkui_receiver] heartbeat reciver is ready!\n");

	Receiver.running = 1;
	
	ret = pthread_create(&Receiver.thread, NULL, thread_proc_ftkui_receiver, NULL);
	assert(0 == ret);
}

void stop_receive_heartbeat(void)
{
	int ret = -1;
	
	Receiver.running = 0;

	ret = pthread_join(Receiver.thread, NULL);
	assert(0 == ret);
}

int get_car_color(int id)
{
	return Receiver.color[id];
}

int get_car_number(int id)
{
	return Receiver.number[id];
}

int get_car_coordinate_x(int id)
{
	return Receiver.x[id];
}

int get_car_coordinate_y(int id)
{
	return Receiver.y[id];
}
#endif