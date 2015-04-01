CC := clang++
CFLAGS := -g -fPIC -Wall -std=c++11
TARGET := loader loader-dbg
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))

.PYTHON: all

all: loader loader-dbg


%.o:%.cpp
	$(CC) $(CFLAGS) -c -m32 $<

loader:$(OBJS)
	$(CC) $(CFLAGS) -pie -lcapstone -ldl -m32 -Wno-c++11-extensions -o $@ $^
loader-dbg:$(OBJS)
	$(CC) $(CFLAGS) -pie -lcapstone -ldl -m32 -Wno-c++11-extensions -o $@ $^
clean:
	rm -rf *.o

