DIR_LIB = lib
DIR_SRC = src
DIR_HEADS = include
DIR_USR_LIB = bin

LIBRARY = libmrlib.dll

OBJECTS_SRC = $(wildcard $(DIR_SRC)/*.c)
OBJECTS = $(patsubst %.c,%.o,$(notdir $(OBJECTS_SRC)))

CC = gcc -std=c11 -DHAVE_STRUCT_TIMESPEC

.PHONY:all install clean

all:$(LIBRARY)

$(LIBRARY):$(OBJECTS)
	$(CC) $(OBJECTS) -shared -o$(DIR_LIB)/$(LIBRARY) -lpthreadGC2
	-rm -f $(OBJECTS)

$(OBJECTS):$(OBJECTS_SRC)
	$(CC) $(OBJECTS_SRC) -c -I$(DIR_HEADS)

install:
	cp -f $(DIR_LIB)/$(LIBRARY) $(DIR_USR_LIB)/

clean:
	-rm -f $(DIR_LIB)/*.dll *.o
