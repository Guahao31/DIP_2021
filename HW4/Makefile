.PHONY: build clean all

target = a
CC = gcc
objs = main.o bmp.o

all:
	@make build;\
	rm *.o

build: $(objs)
	$(CC) -o $(target) *.o

%.o: %.c
	$(CC) $^ -c -o $@

clean: $(objs) $(target)
	rm *.o
	rm $(target)
