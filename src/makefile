CC = gcc
RM = rm
CFLAGS = -Os -mconsole

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

files = raw2iff.o

all: raw2iff.exe

raw2iff.exe: $(files)
	$(CC) $(CFLAGS) -s -o ../$@ $(files) $(LIBS)

clean:
	-$(RM) $(files)
