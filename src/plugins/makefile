CC = gcc
RM = rm
CFLAGS = -Os -D__WINDOWS__
LDFLAGS = -s -shared -Wl,--subsystem,windows

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

all: pal_argb8888.dll pal_rgb444.dll pal_rgb333.dll pal_rgb888.dll \
     pic_planar_interleaved.dll pic_planar.dll pic_chunky.dll pic_atari_st.dll

pal_argb8888.dll: pal_argb8888.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_argb8888.o $(LIBS)

pal_rgb444.dll: pal_rgb444.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_rgb444.o $(LIBS)

pal_rgb333.dll: pal_rgb333.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_rgb333.o $(LIBS)

pal_rgb888.dll: pal_rgb888.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pal_rgb888.o $(LIBS)

pic_planar_interleaved.dll: pic_planar_interleaved.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_planar_interleaved.o $(LIBS)

pic_planar.dll: pic_planar.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_planar.o $(LIBS)

pic_chunky.dll: pic_chunky.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_chunky.o $(LIBS)

pic_atari_st.dll: pic_atari_st.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../../plugins/$@ pic_atari_st.o $(LIBS)

clean:
	-$(RM) pal_argb8888.o pal_rgb444.o pal_rgb333.o pal_rgb888.o \
           pic_planar_interleaved.o pic_planar.o pic_chunky.o pic_atari_st.o
