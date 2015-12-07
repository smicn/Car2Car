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
#ifndef CAR_H
#define CAR_H

class Car {
public:
	static const int COLOR_RED  = 0;
	static const int COLOR_BLUE = 1;
	static const int NUMBER_ONE = 1;
	static const int NUMBER_TWO = 2;

	Car(int id, int color, int number) {
		this->id = id;
		this->color = color;
		this->number = number;
		if (COLOR_RED == color) {
			xx = -15;
		}
		else {
			xx = 15;
		}
		if (NUMBER_ONE == number) {
			yy = 5;
		}
		else {
			yy = -5;
		}
		speed = speedOnBridge = 1;
		directionControlEnable = false;
	}
	
	Car(int id, int color, int number, int initx, int inity) {
		this->id = id;
		this->color = color;
		this->number = number;		
		this->xx = initx;
		this->yy = inity;
		this->id = calcID();
		speed = speedOnBridge = 1;
		directionControlEnable = false;
	}
	
	~Car() {
		if (dcs != NULL) {
			delete dcs;
			dcs = NULL;
		}
	}
	
	int getColor() {
		return color;
	}
	int getNumber() {
		return number;
	}
	int getID() {
		return id;
	}

	void ready(int direction) {
		if (0 == direction) {
			dcs = BridgeProtocol::getInstance()->createDCS("single");
			assert(dcs != NULL);
		}
		else {
			directionControlEnable = true;
			dcs = BridgeProtocol::getInstance()->createDCS("multiple");
			assert(dcs != NULL);
		}
	}
	
	void setSpeed(int speed) {
		this->speed = speed;
		this->speedOnBridge = speed;
	}
	void setSpeed(int speed, int speedOnBridge) {
		this->speed = speed;
		this->speedOnBridge = speedOnBridge;
	}
	
	void moveForwardOneStep(void) 
	{
		/* sofar, waiting for CS is the only reason to pause */
		if (isWaitingForCriticalSection()) {
			//dbg("waiting");
			return;
		}
		//dbg("moving");
		
		/* the car is at the left se of the brge */
		if (-20 == xx) {
			if (-5 == yy) {
				xx += speed;
			}
			else if (-5 < yy && yy <= 5) {
				yy -= speed;
			}
			else {
				assert(-5 <= yy && yy <= 5);
			}
		}
		else if (-20 < xx && xx < -10) {
			if (-5 == yy) {
				xx += speed;
			}
			else if (5 == yy) {
				xx -= speed;
			}
			else {
				assert(-5 == yy || 5 == yy);
			}
		}
		else if (-10 == xx) {
			if (5 == yy) {
				xx -= speed;
			}
			else if ((-5 <= yy && yy < -1) || (1 < yy && yy < 5)) {
				yy += speed;
			}
			else if (-1 == yy) {
				dbg("approach bridge from left side.");
				/* the right position to request CS */
				requestCriticalSection(1);
				xx += speedOnBridge;
			}
			else if (1 == yy) {
				/* the right position to release CS */
				releaseCriticalSection();
				dbg("left bridge at left side.");
				yy += speed;
			}	
		}

		/* the car is now crossing the brge */
		else if (-10 < xx && xx < 10) {
			//assert(isInCriticalSection());
			
			if (-1 == yy) {
				if (-9 == xx) dbg("i'm on bridge now, from left side.");
				
				xx += speedOnBridge;
			}
			else if (1 == yy) {
				if (9 == xx) dbg("i'm on bridge now, from right side.");
				
				xx -= speedOnBridge;
			}
		}

		/* the car is at the right side of the brge */
		else if (10 == xx) {
			/* just oppsite with what is doing at the left side */
			if (-5 == yy) {
				xx += speed;
			}
			else if ((-5 < yy && yy < -1) || (1 < yy && yy <= 5)) {
				yy -= speed;
			}
			else if (-1 == yy) {
				/* the right position to release CS */
				releaseCriticalSection();
				dbg("left bridge at right side.");
				yy -= speed;
			}
			else if (1 == yy) {
				dbg("approach bridge from right side.");
				/* the right position to request CS */
				requestCriticalSection(2);
				xx -= speedOnBridge;
			}
		}
		else if (10 < xx && xx < 20) {
			/* same as what is doing at left se */
			if (-5 == yy) {
				xx += speed;
			}
			else if (5 == yy) {
				xx -= speed;
			}
			else {
				assert(-5 == yy || 5 == yy);
			}
		}
		else if (20 == xx) {
			/* somewhat oppsite with what is doing at the left se */
			if (5 == yy) {
				xx -= speed;
			}
			else if (-5 <= yy && yy < 5) {
				yy += speed;
			}
			else {
				assert(-5 <= yy && yy <= 5);
			}
		}
		else {
			assert(-20 <= xx && xx <= 20);
		}
	}

	int getX() {
		return xx;
	}
	int getY() {
		return yy;
	}

	bool isWaitingForCrossingBridge() {
		return isWaitingForCriticalSection();
	}
	bool isCrossingBridge() {
		return ((-10 < xx && xx < 10) && isInCriticalSection());
	}

private:
	void requestCriticalSection(int direction) {
		if (directionControlEnable) {
			cout << "DCSMultipleAccess.requestEx( " << direction << " )" << endl;
			(static_cast<DCSMultipleAccess*>(dcs))->requestEx(direction);
		}
		else {
			dcs->request();
		}
	}
	void releaseCriticalSection() {
		dcs->release();
	}
	bool isWaitingForCriticalSection() {
		return dcs->isWaiting();
	}
	bool isInCriticalSection() {
		return dcs->isInCSNow();
	}

	int calcID() {
		return 2 * (number - 1) + color;
	}

	void dbg(const char *error) {
		cout << "[car" << id << "]: " << error << endl; 
	}
	
private:
	int color;
	int number;
	int id;

	int xx;
	int yy;

	int speed;
	int speedOnBridge;

	bool directionControlEnable;

	DistributedCriticalSection *dcs;
};

#endif
