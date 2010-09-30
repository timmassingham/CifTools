CC = gcc
CFLAGS = -Wall -O3 -funroll-loops -DNDEBUG -fomit-frame-pointer -std=gnu99 -msse2 
#CFLAGS = -std=c99 -Wall -g -std=gnu99 -fstack-protector-all -D_FORTIFY_SOURCE=2
#CFLAGS = -pg -std=gnu99 
LD = ld
LDFLAGS =  -lm -lz -lbz2
INCFLAGS = 
objects = cif.o xio.o tile.o cluster.o matrix.o

all: cifinfo cifsplit cifjoin int2cif

cifinfo: $(objects)
	$(CC) $(CFLAGS) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) cifinfo.c

cifsplit: $(objects)
	$(CC) $(CFLAGS) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) cifsplit.c

cifjoin: $(objects)
	$(CC) $(CFLAGS) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) cifjoin.c

int2cif: $(objects)
	$(CC) $(CFLAGS) $(DEFINES) $(INCFLAGS) -o ../bin/$@ $(LDFLAGS) $(objects) int2cif.c

.c.o:
	$(CC) $(CFLAGS) $(INCFLAGS) -o $@ -c $<

.f.o:
	g77 -O3 -o $@ -c $<

clean:
	rm -f ../bin/* *~ *.o  *.obj  *.exe *.lib

archive: clean
	cd ../.. && tar zcf ciftool.tgz --exclude=.git --exclude=archive ciftool
