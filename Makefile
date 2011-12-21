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

CFLAGS:= -std=gnu99 -Wall -Wextra -pedantic -DDATADIR=\"$(SHAREDIR)\"

# -DDATA_DIR is an easy way of giving the src files
# the default install location. options code relies
# upon it so DO NOT REMOVE.

#------------------developer options-----------------
# -DDEBUG spews out information
# primarily associated with the processing of
# movements and bomb detonations. Redirect stderr
# if you use it.

#CFLAGS:=$(CFLAGS) -ggdb -DDEBUG

CFLAGS:=$(CFLAGS) -O2

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
VERSION:=0.2.1

CFLAGS:=$(CFLAGS) -DVERSION=\"$(VERSION)\"

LIBS   :=-lcurses
SRC    :=$(wildcard *.c)
OBJS   :=$(patsubst %.c, %.o, $(SRC))
HEADERS:=$(wildcard *.h)
HELP   :=$(wildcard help*.txt)
MAPS   :=$(wildcard maps/[0-1][0-9].xcm)

DISTFILES :=ChangeLog CHANGES INSTALL Makefile NEWS README TODO
DIST      :=XorCurses-$(VERSION)
DISTDIR   :=$(DIST)/

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
	install -t $(MAPDIR) $(MAPS)
	install -t $(SHAREDIR) $(HELP)

uninstall:
	rm -f $(BINDIR)$(PROG)
	rm -rf $(SHAREDIR)

dist:
	rm -rf $(DISTDIR)
	mkdir $(DISTDIR)
	cp $(SRC) $(DISTDIR)
	cp $(HEADERS) $(DISTDIR)
	cp $(HELP) $(DISTDIR)
	mkdir $(DISTDIR)/maps
	cp $(MAPS) $(DISTDIR)/maps
	cp $(DISTFILES) $(DISTDIR)
	tar -cjf $(DIST).tar.bz2 $(DISTDIR)
	rm -rf $(DISTDIR)
