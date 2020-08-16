WMNAME  = wm

PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/bin
MANPREFIX = ${PREFIX}/share/man

X11INC = -I/usr/X11R6/include
X11LIB = -L/usr/X11R6/lib -lX11 -lXft -lfreetype -lpthread -lz -lfontconfig -I/usr/include/freetype2
XINERAMALIB = -lXrandr

INCS = -I. -I/usr/include ${X11INC}
LIBS = -L/usr/lib -lc ${X11LIB} ${XINERAMALIB}

CFLAGS   = -std=c++17 -pedantic -Wall -Wextra ${INCS}"
LDFLAGS  = ${LIBS}

GCC 	 = g++
EXEC = ${WMNAME}

SRC = ${WMNAME}.cpp utils.cpp logger.cpp tiling.cpp client.cpp desktop.cpp main.cpp
OBJ = ${SRC:.c=.o}

all: CFLAGS += -Os
all: LDFLAGS += -g
all: options ${WMNAME}

debug: CFLAGS += -O0
debug: options ${WMNAME}

.c.o:
	@echo GCC $<
	@${GCC} -c ${CFLAGS} $<

${OBJ}: config.h logger.h wm.h monitor.h desktop.h client.h toml.h


${WMNAME}: ${OBJ}
	@echo CC -o $@
	@${GCC} -o $@ ${OBJ} ${LDFLAGS} -g

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@install -Dm755 ${WMNAME} ${DESTDIR}${PREFIX}/bin/${WMNAME}
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man.1
	@install -Dm644 ${WMNAME}.1 ${DESTDIR}${MANPREFIX}/man1/${WMNAME}.1

.PHONY: all options clean install uninstall
