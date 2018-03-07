# Target library
lib := libfs.a
ofiles := disk.o fs.o

CC := gcc
ARC := ar rcs
CFLAGS := -Wall -Werror -g

all: $(lib)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<


$(lib): $(ofiles)
	$(ARC) $(lib) $(ofiles)

clean:
	rm -f $(lib) $(ofiles)

