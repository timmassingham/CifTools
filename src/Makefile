CC = gcc
CFLAGS = -Wall -O3 -funroll-loops -DNDEBUG -fomit-frame-pointer -std=gnu99 -msse2 
#CFLAGS = -std=c99 -Wall -g -std=gnu99 -fstack-protector-all -D_FORTIFY_SOURCE=2
#CFLAGS = -pg -std=gnu99 
CFLAGSSFMT = -msse2 -DHAVE_SSE2 -O9 -finline-functions -fomit-frame-pointer \
-DNDEBUG -fno-strict-aliasing --param max-inline-insns-single=1800 -std=c99
LD = ld
LDFLAGS =  -lm -lz -lbz2
INCFLAGS = 
DEFINES =
MANDIR = ../man
objects = cif.o xio.o tile.o cluster.o matrix.o

all: cifinfo cifsplit cifjoin int2cif cifboot cifcat cifstat

cifinfo: $(objects) cifinfo.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

cifstat: $(objects) cifstat.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

cifsplit: $(objects) cifsplit.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

cifjoin: $(objects) cifjoin.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

cifboot: cifboot.o $(objects) sfmt.o random.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

cifcat: $(objects) cifcat.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

int2cif: $(objects) int2cif.o
	 $(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $^ $(LDFLAGS)

sfmt.o: 
	gcc -o sfmt.o $(CFLAGSSFMT) -DMEXP=19937 -ISFMT-src-1.3/ -c SFMT-src-1.3/SFMT.c

%.o : %.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o $@ -c $<

.f.o:
	g77 -O3 -o $@ -c $<

%.1.html : %.1.txt
	cd $(MANDIR) && asciidoc -d manpage $<

%.1 : %.1.txt
	cd $(MANDIR) && a2x -d manpage -f manpage $<

clean:
	rm -f ../bin/* *~ *.o  *.obj  *.exe *.lib $(MANDIR)/*.1 && cd SFMT-src-1.3/ && make clean

archive: clean man
	cd ../.. && tar zcf ciftools.tgz --exclude=.git --exclude=archive --exclude=test ciftools

man: man_troff man_html

man_troff: $(MANDIR)/ciftools.1 $(MANDIR)/cifboot.1 $(MANDIR)/cifcat.1 $(MANDIR)/cifinfo.1 $(MANDIR)/cifjoin.1 $(MANDIR)/cifsplit.1 $(MANDIR)/cifstat.1 $(MANDIR)/int2cif.1 

man_html: $(MANDIR)/ciftools.1.html $(MANDIR)/cifboot.1.html $(MANDIR)/cifcat.1.html $(MANDIR)/cifinfo.1.html $(MANDIR)/cifjoin.1.html $(MANDIR)/cifsplit.1.html $(MANDIR)/cifstat.1.html $(MANDIR)/int2cif.1.html

