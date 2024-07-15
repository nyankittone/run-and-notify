MKDIR   := mkdir
RMDIR   := rm -r

UNIT_TESTS := unit-tests
EXE     := run-and-notify
DEV_EXE := devbuild
SRC     := src
INCLUDE := -Iinclude -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libpng16 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/sysprof-6
LIBS := -lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0
OBJ     := obj
DEV_OBJ := dev-obj

CC := cc
CFLAGS := -std=c99 -lc -pthread $(LIBS) -pedantic-errors -Wall $(INCLUDE)
RELEASE_FLAGS := -O3
DEV_FLAGS := -Og -g

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
DEV_OBJS := $(patsubst $(SRC)/%.c,$(DEV_OBJ)/%.o,$(SRCS))

.PHONY: all devbuild clean buildtests test

all: $(EXE)

$(EXE): $(OBJS) $(EXE).o
	$(CC) $(RELEASE_FLAGS) $^ $(CFLAGS) -o $@

$(DEV_EXE): $(DEV_OBJS) $(DEV_EXE).o
	$(CC) $(DEV_FLAGS) $^ $(CFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(RELEASE_FLAGS) -c $< $(CFLAGS) -o $@

$(DEV_OBJ)/%.o: $(SRC)/%.c | $(DEV_OBJ)
	$(CC) $(DEV_FLAGS) -c $< $(CFLAGS) -o $@

$(EXE).o: main.c
	$(CC) $(RELEASE_FLAGS) -c $< $(CFLAGS) -o $@

$(DEV_EXE).o: main.c
	$(CC) $(DEV_FLAGS) -c $< $(CFLAGS) -o $@

$(OBJ):
	$(MKDIR) $@

$(DEV_OBJ):
	$(MKDIR) $@

# Makefiles have broken my brain. I am now filled with hate.
#buildtests: $(wildcard $(UNIT_TESTS)/*.c) | $(wildcard $(DEV_OBJ)/*.o)
	
clean:
	if [ -f $(EXE).o ]; then rm $(EXE).o; fi
	if [ -f $(EXE) ]; then rm $(EXE); fi
	if [ -f $(DEV_EXE).o ]; then rm $(DEV_EXE).o; fi
	if [ -f $(DEV_EXE) ]; then rm $(DEV_EXE); fi
	if [ -d $(OBJ) ]; then $(RMDIR) $(OBJ); fi
	if [ -d $(DEV_OBJ) ]; then $(RMDIR) $(DEV_OBJ); fi

