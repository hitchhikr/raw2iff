CC = gcc
RM = rm
CFLAGS = -Os -D__WINDOWS__
LDFLAGS = -s -shared -Wl,--subsystem,windows

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

all: pal_argb8.dll pal_rgb4.dll pal_rgb8.dll pic_planar_interleaved.dll pic_planar.dll pic_chunky.dll

pal_argb8.dll: pal_argb8.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_argb8.o $(LIBS)

pal_rgb4.dll: pal_rgb4.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_rgb4.o $(LIBS)

pal_rgb8.dll: pal_rgb8.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_rgb8.o $(LIBS)

pic_planar_interleaved.dll: pic_planar_interleaved.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_planar_interleaved.o $(LIBS)

pic_planar.dll: pic_planar.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_planar.o $(LIBS)

pic_chunky.dll: pic_chunky.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_chunky.o $(LIBS)

clean:
	-$(RM) pal_argb8.o pal_rgb4.o pal_rgb8.o pic_planar_interleaved.o pic_planar.o pic_chunky.o
