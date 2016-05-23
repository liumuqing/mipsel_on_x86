CC := clang++
DEBUG := -DDEBUG
CFLAGS := -g -march=native -fPIC -Wall -std=c++11 -Ofast -fno-builtin-memcpy -fno-builtin-strcpy -fno-builtin-strcmp $(DEBUG) #-DBIGENDIAN #-DDEBUG
TARGET := loader
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))

.PYTHON: all

all: loader


%.o:%.cpp
	$(CC) $(CFLAGS) -c -fPIC -m32 $< -o $@

loader:$(OBJS)
	$(CC) -v $(CFLAGS) -fPIC -pie -lcapstone -ldl -m32 -Ofast -Wno-c++11-extensions -L./  -o $@ $^
clean:
	rm -rf *.o

