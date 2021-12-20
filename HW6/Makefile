.PHONY: build clean all

target = a
CC = gcc
objs = main.o bmp.o

all:
	@make build;\
	rm *.o

build: $(objs)
	$(CC) -O3 -o $(target) *.o

%.o: %.c
	$(CC) -O3 $^ -c -o $@

clean: $(objs) $(target)
	rm *.o
	rm $(target)
