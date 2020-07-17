CC = clang
CFLAGS = -O0 -g

ALL = a.out

all: $(ALL)

a.out: ddllp.c ddllp.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(ALL)
