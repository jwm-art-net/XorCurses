CFLAGS:= -std=gnu99 -Wall -Wextra -pedantic -DDATADIR=\"$(SHAREDIR)\"
CFLAGS:=$(CFLAGS) -ggdb -DDEBUG


PROG:=mapconvert
CC:=gcc

SRC    :=$(wildcard *.c)
OBJS   :=$(patsubst %.c, %.o, $(SRC))
HEADERS:=$(wildcard *.h)
XCOBJS := ../map.o ../fletcher.o ../data_file.o

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(XCOBJS) -o $(PROG)
	
%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -I.. -L.. $<

map.o: map.c $(HEADERS)

clean:
	rm -f $(PROG) $(OBJS)

