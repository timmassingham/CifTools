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
objects = cif.o xio.o tile.o cluster.o matrix.o

all: cifinfo cifsplit cifjoin int2cif cifboot

cifinfo: $(objects)
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) cifinfo.c

cifsplit: $(objects)
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) cifsplit.c

cifjoin: $(objects)
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) cifjoin.c

cifboot: cifboot.o $(objects) sfmt.o random.o
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $^ 

int2cif: $(objects)
	 $(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) int2cif.c

sfmt.o: 
	gcc -o sfmt.o $(CFLAGSSFMT) -DMEXP=19937 -ISFMT-src-1.3/ -c SFMT-src-1.3/SFMT.c

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o $@ -c $<

.f.o:
	g77 -O3 -o $@ -c $<

clean:
	rm -f ../bin/* *~ *.o  *.obj  *.exe *.lib && cd SFMT-src-1.3/ && make clean

archive: clean
	cd ../.. && tar zcf ciftool.tgz --exclude=.git --exclude=archive --exclude=test ciftool
