# Makefile for all the examples

all_examples:=$(shell find -iname "*.c")

CFLAGS:=$(shell pkg-config --libs --cflags gfal2) $(shell pkg-config --libs --cflags gfal_transfer)
OBJS=$(all_examples:%.c=%)

all: $(OBJS)

clean:
	$(RM) $(OBJS)
