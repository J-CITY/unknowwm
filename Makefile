WMNAME  = wm

PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/bin
MANPREFIX = ${PREFIX}/share/man

X11INC = -I/usr/X11R6/include
X11LIB = -L/usr/X11R6/lib -lX11 -lXft -lfreetype -lpthread -lz -lfontconfig -I/usr/include/freetype2
XINERAMALIB = -lXrandr

INCS = -I. -I/usr/include ${X11INC}
LIBS = -L/usr/lib -lc ${X11LIB} ${XINERAMALIB}

CFLAGS   = -std=c++14 -pedantic -Wall -Wextra ${INCS}"
LDFLAGS  = ${LIBS}

GCC 	 = g++
EXEC = ${WMNAME}

SRC = ${WMNAME}.cpp main.cpp
OBJ = ${SRC:.c=.o}

all: CFLAGS += -Os
all: LDFLAGS += -s
all: options ${WMNAME}

debug: CFLAGS += -O0 -g
debug: options ${WMNAME}

.c.o:
	@echo GCC $<
	@${GCC} -c ${CFLAGS} $<

${OBJ}: config.h logger.h wm.h desktop.h monitor.h  client.h toml.h


${WMNAME}: ${OBJ}
	@echo CC -o $@
	@${GCC} -o $@ ${OBJ} ${LDFLAGS}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@install -Dm755 ${WMNAME} ${DESTDIR}${PREFIX}/bin/${WMNAME}
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man.1
	@install -Dm644 ${WMNAME}.1 ${DESTDIR}${MANPREFIX}/man1/${WMNAME}.1

.PHONY: all options clean install uninstall
