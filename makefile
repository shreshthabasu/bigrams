CFLAGS = -ggdb3 -std=c11 -Wall -Wunused-parameter -Wstrict-prototypes -Werror -Wextra -Wshadow
#CFLAGS += -fsanitize=signed-integer-overflow
CFLAGS += -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wfloat-conversion
#CFLAGS += -fsanitize=address -fsanitize=undefined

bigrams: bigrams.c hashtable.c
	gcc -o $@ $^ $(CFLAGS) -g -lm -O3