DIR_LIB = lib
DIR_SRC = src
DIR_SAMPLES_BIN = bin
DIR_SAMPLES_SRC = samples
DIR_USR_LIB = /usr/lib

LIBRARY = libmrlib.so

OBJECTS_SRC = $(wildcard $(DIR_SRC)/*.c)
OBJECTS = $(patsubst %.c, %.o, $(notdir $(OBJECTS_SRC)))

SAMPLES_SRC = $(wildcard $(DIR_SAMPLES_SRC)/*.c)
SAMPLES = $(patsubst %.c, %.out, $(notdir $(SAMPLES_SRC)))
SAMPLES_NAME = $(patsubst %.c, %, $(notdir $(SAMPLES_SRC)))

CC = clang
LIB = -L$(DIR_LIB) -lmrlib -lpthread 
INCLUDE = -Iinclude

define MAKE_SAMPLE
$(1):
	$(CC) $(DIR_SAMPLES_SRC)/$(2) -o$(DIR_SAMPLES_BIN)/$(1) $(INCLUDE) $(LIB)
endef

.PHONY:all clean install uninst

all: $(LIBRARY) $(SAMPLES)

$(LIBRARY):$(OBJECTS)
	$(CC) $(OBJECTS) -shared -fPIC -o$(DIR_LIB)/$(LIBRARY)
	-rm -f $(OBJECTS)

$(OBJECTS):$(OBJECTS_SRC)
	$(CC) $(OBJECTS_SRC) -fPIC -c $(INCLUDE)

$(foreach n,$(SAMPLES_NAME),$(eval $(call MAKE_SAMPLE,$(n).out,$(n).c)))

clean:
	-rm -f $(DIR_LIB)/*.so *.o $(DIR_SAMPLES_BIN)/*.out

install:
	cp -f $(DIR_LIB)/$(LIBRARY) $(DIR_USR_LIB)/
	ldconfig

uninst:
	rm -f $(DIR_USR_LIB)/$(LIBRARY)
	ldconfig

