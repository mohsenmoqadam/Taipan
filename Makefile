CC ?= cc
CFLAGS ?= -O2
CPPFLAGS ?=
WARNINGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wformat=2

.PHONY: all clean check

all: taipan

taipan: taipan.c taipan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=gnu17 taipan.c -o $@

check: taipan
	./taipan --help >/dev/null

clean:
	$(RM) taipan
