DIR_LIB = lib
DIR_SRC = src
DIR_HEADS = include
DIR_USR_LIB = /usr/lib
DIR_USR_HEADS = /usr/include

LIBRARY = libmrlib.so

OBJECTS_SRC = $(wildcard $(DIR_SRC)/mr_*.c)
OBJECTS = $(patsubst %.c,%.o,$(notdir $(OBJECTS_SRC)))

CC = clang

.PHONY:all install clean

all:$(LIBRARY)

$(LIBRARY):$(OBJECTS)
	$(CC) $(OBJECTS) -shared -fPIC -o$(DIR_LIB)/$(LIBRARY)
	-rm -f $(OBJECTS)

$(OBJECTS):$(OBJECTS_SRC)
	$(CC) $(OBJECTS_SRC) -fPIC -c -I$(DIR_HEADS)

install:
	cp -f $(DIR_LIB)/$(LIBRARY) $(DIR_USR_LIB)/
	ldconfig
	cp -f $(DIR_HEADS)/mr_*.h $(DIR_USR_HEADS)/

clean:
	-rm -f $(DIR_LIB)/*.so *.o
