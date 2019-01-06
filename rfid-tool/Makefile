CC=g++

DIR_OUT := out
DIR_SRC := src
DIR_INC := inc

CFLAGS += -Wall
CFLAGS += -std=c++11
CFLAGS += -O2 -ggdb
CFLAGS += -I$(DIR_INC) -I../common/inc

LDFLAGS += -lusb

SRCS := \
	src/common/Log.cpp \
	src/common/DataUtils.cpp \
	\
	src/rfid/Interface.cpp \
	src/rfid/InterfaceFactory.cpp \
	src/rfid/CarrierDecoder.cpp \
	src/rfid/Em4100Decoder.cpp \
	src/rfid/Em4100Eprom.cpp \
	src/rfid/T5557Encoder.cpp \
	\
	src/rfid/impl/ManchesterDecoder.cpp \
	src/rfid/impl/InterfaceUsbImpl.cpp \
	\
	src/main.cpp

APP_NAME := rfid-tool

init:
	mkdir -p $(DIR_OUT)

all: init
	$(CC) $(CFLAGS) -o $(DIR_OUT)/$(APP_NAME) $(SRCS) $(LDFLAGS)
	
clean:
	rm -rf $(DIR_OUT)
