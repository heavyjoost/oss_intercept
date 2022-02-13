.POSIX:

DEBUG ?= 0
VOLPATH ?= /tmp/oss_intercept

CPPFLAGS += -DDEBUG=$(DEBUG) -DVOLPATH=\"$(VOLPATH)\"
CFLAGS += -fPIC -pedantic -Wall -Wextra
LDFLAGS += -s -nostdlib -shared

PREFIX	= /usr/local
LIBDIR	= $(PREFIX)/lib

NAME := intercept
VERSION = 0.1
TARGET := intercept.so
SRCS = intercept.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

install: $(TARGET)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp -f $(TARGET) $(DESTDIR)$(LIBDIR)

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

distclean: clean
	rm -f $(NAME)-$(VERSION).tar.gz

dist: clean
	mkdir -p $(NAME)-$(VERSION)
	cp -R Makefile $(SRCS) $(NAME)-$(VERSION)
	tar -cf - $(NAME)-$(VERSION) | gzip > $(NAME)-$(VERSION).tar.gz
	rm -rf $(NAME)-$(VERSION)

.PHONY: all install uninstall clean distclean dist
