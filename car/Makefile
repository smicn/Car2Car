#
# Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
#

BUILD_FLAGS = -lrt -pthread -g -fpermissive
BUILD_CC    = g++
BUILD_RM    = rm -f
BUILD_CP    = cp -f

SOURCES_CAR := \
	MessageHandler.cpp \
	DistributedCriticalSection.cpp \
	DCSMultipleAccess.cpp \
	BridgeProtocol.cpp \
	HeartbeatReporter.cpp \
	CarFacade.cpp

car:
	$(BUILD_CC) $(SOURCES_CAR) -o car $(BUILD_FLAGS)

all: car

clean:
	$(BUILD_RM) car gui
