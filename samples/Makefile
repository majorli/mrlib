DIR_SAMPLES_BIN = ../bin
DIR_SAMPLES_SRC = .

SAMPLES_SRC = $(wildcard $(DIR_SAMPLES_SRC)/*.c)
SAMPLES = $(patsubst %.c,%.out,$(notdir $(SAMPLES_SRC)))
SAMPLES_NAME = $(patsubst %.c,%,$(notdir $(SAMPLES_SRC)))

CC = gcc -std=c11
LIB = -lmrlib

define MAKE_SAMPLE
$(1):
	$(CC) $(DIR_SAMPLES_SRC)/$(2) -o$(DIR_SAMPLES_BIN)/$(1) $(LIB) -Iinclude -Llib
endef

.PHONY:all clean

all: $(SAMPLES)

$(foreach n,$(SAMPLES_NAME),$(eval $(call MAKE_SAMPLE,$(n).exe,$(n).c)))

clean:
	-rm -f $(DIR_SAMPLES_BIN)/*.exe
