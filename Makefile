#----------------user/install options----------------
# alter only if you're not clueless and want XorCurses
# installed in some other location or you're packaging
# it:

PREFIX=/usr/local/
BINDIR=$(PREFIX)bin/
SHAREDIR=$(PREFIX)share/XorCurses/
MAPDIR=$(SHAREDIR)maps
#------------------standard options------------------
# probably fine for most people:

CFLAGS:= -O2 -std=gnu99 -Wall -pedantic -DDATADIR=\"$(SHAREDIR)\"

# -DDATA_DIR is an easy way of giving the src files
# the default install location. options code relies
# upon it so DO NOT REMOVE.

#------------------developer options-----------------
# -DDEBUG spews out information
# primarily associated with the processing of
# movements and bomb detonations. Redirect stderr
# if you use it.

#CFLAGS:= -std=gnu99 -Wall -pedantic -ggdb -DDATADIR=\"$(SHAREDIR)\"
#CFLAGS:= -DTESTMAP -std=gnu99 -Wall -pedantic -ggdb -DDATADIR=\"$(SHAREDIR)\"
#CFLAGS:= -DDEBUG -std=gnu99 -Wall -pedantic -ggdb -DDATADIR=\"$(SHAREDIR)\"

# Add -DTESTMAP to enable access within the level
# menu of the test map (maps/0.txt). NOTE: DO NOT
# make install with -DTESTMAP defined. the test
# map is not installed and so xorcurses quits
# because it expects to find it and can't. i'm not
# gonna change this action.

#----------------------------------------------------
# END OF OPTIONS

PROG:=xorcurses
CC:=gcc

LIBS   :=-lcurses
SRC    :=$(wildcard *.c)
OBJS   :=$(patsubst %.c, %.o, $(SRC))
HEADERS:=$(wildcard *.h)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG) $(LIBS)
	
%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

xorcurses.o: xorcurses.c $(HEADERS)

clean:
	rm -f $(PROG) $(OBJS)

install: $(PROG)
	install $(PROG) $(BINDIR)
	install -d $(MAPDIR)
	install -t $(MAPDIR) maps/[1-9]*
	install -t $(SHAREDIR) help*.txt

uninstall:
	rm -f $(BINDIR)$(PROG)
	rm -rf $(SHAREDIR)
