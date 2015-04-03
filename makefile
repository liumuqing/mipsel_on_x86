CC := clang++
CFLAGS := -g -fPIC -Wall  -std=c++11 -Ofast #-DDEBUG
TARGET := loader
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))

.PYTHON: all

all: loader


%.o:%.cpp
	$(CC) $(CFLAGS) -c -m32 $< -o $@

loader:$(OBJS)
	$(CC) $(CFLAGS) -pie -lcapstone -ldl -m32 -O3 -Wno-c++11-extensions -o $@ $^
clean:
	rm -rf *.o

