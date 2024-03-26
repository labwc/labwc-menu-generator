# Define ASAN=1 to enable AddressSanitizer

.POSIX:

RM ?= rm -f
MAKEFLAGS += --no-print-directory
CFLAGS += -g -Wall -Os -std=c99 -Wextra
CFLAGS += `pkg-config --cflags glib-2.0`
LIBS += `pkg-config --libs glib-2.0`
LDFLAGS += $(LIBS)

#ifdef ASAN
#ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic
#CFLAGS += $(ASAN_FLAGS)
#LDFLAGS += $(ASAN_FLAGS) -fuse-ld=gold
#endif

PROG = labwc-menu-generator

all: $(PROG)

$(PROG): main.o desktop.o ignore.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	@$(RM) $(PROG) *.o
	@$(MAKE) -C t/ clean

check: $(PROG)
	@$(MAKE) -C t/ prove

.PHONY: all clean check
