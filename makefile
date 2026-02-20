CC = gcc
RM = rm
STRIP = strip
CFLAGS = -Os -mconsole

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

files = raw2iff.o

all: raw2iff.exe

raw2iff.exe: $(files)
	$(CC) $(CFLAGS) -o $@ $(files) $(LIBS)
	$(STRIP) $@

clean:
	-$(RM) $(files)
