CC=		gcc
CXX=		g++
CFLAGS=		-g -Wall -O2
CXXFLAGS=	$(CFLAGS)
DFLAGS=		-DHAVE_PTHREAD
OBJS=		angsd.o
PROG=		ngstk
INCLUDES=
LIBS=		-lm -lz 

.SUFFIXES:.c .o .cc

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@
.cc.o:
		$(CXX) -c $(CXXFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

ngstk: $(OBJS) main.o
		$(CC) $(CFLAGS) $(DFLAGS) $(OBJS) main.o -o $@ $(LIBS)

angsd.o:angsd.h

clean:
	rm -f *~ *.a $(PROG) *.o a.out
