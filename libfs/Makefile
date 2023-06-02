
CC := gcc
CFLAGS := -Wall -Wextra -Werror 

LIB := libfs.a
OBJS = disk.o fs.o
all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)
