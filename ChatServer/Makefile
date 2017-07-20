CC = g++
CFLAGS = -std=c++11 -Wall -g
INCLUDES = -Iinclude -I../TCPLib
SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp,build/%.o,$(SRCS)) 
LIBS = $(wildcard ../TCPLib/*.o)
PTHREAD = -pthread
TARGET = bin/server

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $^ $(LIBS)

build/%.o:	src/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf build/*.o $(TARGET)
	


