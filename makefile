CC := clang++
CFLAGS := -g -fPIC -Wall -std=c++11
TARGET := main
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))

.PYTHON: all
all:$(TARGET)

%.o:%.cpp
	$(CC) $(CFLAGS) -c -m32 $<

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -pie -lcapstone -ldl -m32 -Wno-c++11-extensions -o $@ $^

clean:
	rm -rf *.o
