# Makefile for LLM Processor

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS = -lcurl

SRCS = llm_service.c llm_query.c
OBJS = $(SRCS:.c=.o)
TARGET = llm_query

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Dependencies
llm_service.o: llm_service.c llm_service.h
llm_processor.o: llm_query.c llm_service.h

.PHONY: all clean
