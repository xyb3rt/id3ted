all: id3ted

CC=g++
PREFIX?=/usr/local
# see INSTALL for available preprocessor variables
CFLAGS+= -I$(PREFIX)/include -Wall -pedantic
LDFLAGS+= -L$(PREFIX)/lib -ltag -lmagic

CPPFILES=$(wildcard *.cpp)
OBJFILES=$(CPPFILES:.cpp=.o)

id3ted:	$(OBJFILES)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.cpp Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

install: all
	install -D -m 0755 id3ted $(PREFIX)/bin/id3ted

clean:
	rm -f id3ted *.o

tags: *.h *.cpp
	ctags $^

cscope: *.h *.cpp
	cscope -b
