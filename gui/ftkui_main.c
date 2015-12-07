/* Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
 *
 * File: ftkui_main.c
 *
 * Brief: This is the file where GUI main function is located.
 *
 * Licensed under the Academic Free License version 2.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "ftk.h"

#ifndef HAS_MAIN
#define HAS_MAIN
#endif

#define CAR_CNT 4
#define PATH_RES "../gui/res/"

struct
{
	FtkWidget* win;
	FtkWidget* text[CAR_CNT];
	
	FtkBitmap* map;
	FtkWidget* imgmap;

	FtkBitmap* car_red_h;
	FtkBitmap* car_red_v;
	FtkBitmap* car_blue_h;
	FtkBitmap* car_blue_v;
	FtkWidget* imgcar[CAR_CNT];
	FtkWidget* number[CAR_CNT];

	FtkWidget* button[3];

	pthread_t  thread;
	int        exit;
	
} FtkCar;

static const int SCREEN_WIDTH  = 800;
static const int SCREEN_HEIGHT = 800;

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
extern void kill_remote_cars(void);
extern void start_receive_heartbeat(void);
extern void stop_receive_heartbeat(void);
#endif

static void ftk_carui_draw_car(int id, int color, int number, int x, int y)
{
	FtkRect rect;
	int xx, yy;

	memset(&rect, 0, sizeof(rect));
	rect.x = ftk_widget_left(FtkCar.win);
	rect.y = ftk_widget_top(FtkCar.win);
	rect.width = ftk_widget_width(FtkCar.win);
	rect.height = ftk_widget_height(FtkCar.win);
	
	xx = (rect.width / 2) + 10 * x;
	yy = (rect.height/ 2) - 15 * y + 60;
	assert(0 < xx && xx < SCREEN_WIDTH);
	assert(0 < yy && yy < SCREEN_HEIGHT);

	if (0 == x && 0 == y) {
		return;
	}
	else if ((-20 == x) || (-10 == x) || (10 == x) || (20 == x)) {
		ftk_widget_move_resize(FtkCar.imgcar[id], xx, yy, 20, 30);
		
		if (COLOR_BLUE == color) {
			assert(FtkCar.car_blue_v != NULL);
			ftk_bitmap_ref(FtkCar.car_blue_v);
			ftk_image_set_image(FtkCar.imgcar[id], FtkCar.car_blue_v);
		}
		else {
			assert(FtkCar.car_red_v != NULL);
			ftk_bitmap_ref(FtkCar.car_red_v);
			ftk_image_set_image(FtkCar.imgcar[id], FtkCar.car_red_v);
		}
	}
	else {
		ftk_widget_move_resize(FtkCar.imgcar[id], xx, yy, 30, 20);
		
		if (COLOR_BLUE == color) {
			assert(FtkCar.car_blue_h != NULL);
			ftk_bitmap_ref(FtkCar.car_blue_h);
			ftk_image_set_image(FtkCar.imgcar[id], FtkCar.car_blue_h);
		}
		else {
			assert(FtkCar.car_red_h != NULL);
			ftk_bitmap_ref(FtkCar.car_red_h);
			ftk_image_set_image(FtkCar.imgcar[id], FtkCar.car_red_h);
		}
	}

	if (NUMBER_TWO == number) {
		ftk_text_view_set_text(FtkCar.number[id], "2", 1);
	}
	else if (NUMBER_ONE == number) {
		ftk_text_view_set_text(FtkCar.number[id], "1", 1);
	}
	else {
		assert(0);
	}
	ftk_widget_move(FtkCar.number[id], xx+20, yy-20);

	ftk_widget_show(FtkCar.imgcar[id], 1);
	ftk_widget_show(FtkCar.number[id], 1);
}

static void* ftk_carui_thread(void *param)
{
	assert(FtkCar.win != NULL);

	while (!FtkCar.exit) {
		int ii;
		
		for (ii = 0; ii < CAR_CNT; ii++) {
			char text[64];
			int color, number, x, y;

			color = get_car_color(ii);
			number = get_car_number(ii);
			x = get_car_coordinate_x(ii);
			y = get_car_coordinate_y(ii);
			
			memset(text, 0, sizeof(text));
			sprintf(text, " car[%d]: %s_%d (%d, %d) ", ii, 
				(color == COLOR_RED) ? "Red" : "Blue", number, x, y);

			ftk_widget_ref(FtkCar.text[ii]);
			ftk_text_view_set_text(FtkCar.text[ii], text, strlen(text));
			ftk_widget_show(FtkCar.text[ii], 1);

			ftk_carui_draw_car(ii, color, number, x, y);
		}

		usleep(100 *1000);
	}
}

static void ftk_app_carui_destroy(void)
{
	int ret = -1;
	
#ifndef LOCAL_SIMULATOR
	stop_receive_heartbeat();
	kill_remote_cars();
#else
	stop_simulator();
#endif
	FtkCar.exit = 1;
	ret = pthread_join(FtkCar.thread, NULL);
	assert(0 == ret);
}

static void ftk_app_carui_run(int direction)
{
	int ret = -1;

#ifndef LOCAL_SIMULATOR
	start_receive_heartbeat();
	launch_remote_cars(direction);
#else
	start_simulator();
#endif

	FtkCar.exit = 0;
	ret = pthread_create(&FtkCar.thread, NULL, ftk_carui_thread, NULL);
	assert(0 == ret);
}

static Ret on_button_clicked(void *ctx, void *obj)
{
	int btnID = (int)ctx;

	static int stop = 1;

	switch(btnID) {
	case 0:
		if (stop) {
			stop = 0;
			ftk_app_carui_run(0);
		}
		break;
		
	case 1:
		if (stop) {
			stop = 0;
			ftk_app_carui_run(1);
		}
		break;
		
	case 2:
		if (!stop) {
			ftk_app_carui_destroy();
			stop = 1;
		}
		break;
		
	default:
		break;
	}

	return RET_OK;
}

static void ftk_carui_create_widgets(void)
{
	const char *ButtonTEXT[3] = {
		"(A)Start", "(B)Start", "Stop"
	};
	char path[FTK_MAX_PATH];
	int color;
	int width = 0;
	int height = 0;
	int ii;
	
	assert(FtkCar.win != NULL);
	
	width = ftk_widget_width(FtkCar.win);
	height = ftk_widget_width(FtkCar.win);
	printf("win.onCreate(): w=%d, h=%d.\n", width, height);

	FtkCar.imgmap = ftk_image_create(FtkCar.win, 118, 175, 617, 380);
	assert(FtkCar.imgmap != NULL);
	memset(path, 0, sizeof(path));
	strcpy(path, PATH_RES"map.png");
	FtkCar.map = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), path);
	assert(FtkCar.map != NULL);
	ftk_image_set_image(FtkCar.imgmap, FtkCar.map);
	ftk_widget_show(FtkCar.imgmap, 1);

	memset(path, 0, sizeof(path));
	strcpy(path, PATH_RES"red1.png");
	FtkCar.car_red_v = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), path);
	assert(FtkCar.car_red_v != NULL);
	ftk_bitmap_ref(FtkCar.car_red_v);
	
	memset(path, 0, sizeof(path));
	strcpy(path, PATH_RES"red1_h.png");
	FtkCar.car_red_h = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), path);
	assert(FtkCar.car_red_h != NULL);
	ftk_bitmap_ref(FtkCar.car_red_h);
	
	memset(path, 0, sizeof(path));
	strcpy(path, PATH_RES"blue1.png");
	FtkCar.car_blue_v = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), path);
	assert(FtkCar.car_blue_v != NULL);
	ftk_bitmap_ref(FtkCar.car_blue_v);
	
	memset(path, 0, sizeof(path));
	strcpy(path, PATH_RES"blue1_h.png");
	FtkCar.car_blue_h = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), path);
	assert(FtkCar.car_blue_h != NULL);
	ftk_bitmap_ref(FtkCar.car_blue_h);

	for (ii = 0; ii < CAR_CNT; ii++) {
		FtkCar.text[ii] = ftk_text_view_create(FtkCar.win, 20, 20+(35*ii), 300, 30);
		assert(FtkCar.text[ii] != NULL);
		ftk_widget_ref(FtkCar.text[ii]);

		FtkCar.imgcar[ii] = ftk_image_create(FtkCar.win, 400, 400, 30, 30);
		assert(FtkCar.imgcar[ii] != NULL);
		ftk_widget_ref(FtkCar.imgcar[ii]);

		FtkCar.number[ii] = ftk_text_view_create(FtkCar.win, 400, 400, 30, 30);
		assert(FtkCar.number[ii] != NULL);
		ftk_widget_ref(FtkCar.number[ii]);
	}

	for (ii = 0; ii < 3; ii++) {
		FtkCar.button[ii] = ftk_button_create(FtkCar.win, 400 + 100*ii, 50, 80, 40);
		assert(FtkCar.button[ii] != NULL);

		ftk_widget_set_text(FtkCar.button[ii], ButtonTEXT[ii]);
		ftk_widget_ref(FtkCar.button[ii]);

		ftk_button_set_clicked_listener(FtkCar.button[ii], on_button_clicked, (void*)ii);

		ftk_widget_show(FtkCar.button[ii], 1);
	}

	ftk_widget_ref(FtkCar.win);
}

int FTK_MAIN(int argc, char* argv[])
{
	char ARGV0[32], ARGV1[256], path[256];
	memset(ARGV0, 0, sizeof(ARGV0));
	memset(ARGV1, 0, sizeof(ARGV1));
	memset(path, 0, sizeof(path));
	
	char *args[] = { ARGV0, ARGV1 };
	argc = sizeof(args)/sizeof(args[0]);
	strcpy(args[0], argv[0]);

	getcwd(path, sizeof(path));
	printf("curr_dir=%s\n", path);
	strcpy(&path[strlen(path)-strlen("bin")], "gui/ftk");
	strcpy(args[1], "--data-dir=");
	strcat(args[1], path);
	
	memset(&FtkCar, 0, sizeof(FtkCar));

	FTK_INIT(argc, args);
	
	FtkCar.win = ftk_app_window_create();
	ftk_window_set_animation_hint(FtkCar.win, "app_main_window");
#ifndef LOCAL_SIMULATOR
	ftk_widget_set_text(FtkCar.win, "Welcome to Lamar Car2Car!");
#else
	ftk_widget_set_text(FtkCar.win, "Lamar Car2Car Simulator (4 Threads, 1 Mutex)");
#endif
	
	ftk_widget_show(FtkCar.win, 1);
	FTK_QUIT_WHEN_WIDGET_CLOSE(FtkCar.win);

	ftk_carui_create_widgets();
	
	FTK_RUN();
}
