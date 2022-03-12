CC=gcc
CCOPTS=-Wall -Wextra -Werror
LIBS=
DEFINE=

SRCS=$(wildcard *.c)
TARGETS=$(SRCS:.c=)

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

test:
	@$(MAKE) DEFINE=-DTEST


%: %.c
	$(CC) $(CCOPTS) $(DEFINE) -o $@ $< $(LIBS)
