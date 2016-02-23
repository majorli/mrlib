## LIB = -lpthread
INCLUDE = -Iinclude
OUT_LIB = lib
SRC_LIB = src
OUT_SAMPLE = bin
SRC_SAMPLE = samples

OBJECTS = $(patsubst %c, %out, $(notdir $(wildcard $(SRC_SAMPLE)/*.c)))

.PHONY:all clean

all: $(OBJECTS)

charset_conv.out:mr_string.o
	clang $(SRC_SAMPLE)/charset_conv.c $(OUT_LIB)/mr_string.o -o$(OUT_SAMPLE)/charset_conv.out $(INCLUDE)
scanChar.out:mr_string.o
	clang $(SRC_SAMPLE)/scanChar.c $(OUT_LIB)/mr_string.o -o$(OUT_SAMPLE)/scanChar.out $(INCLUDE)
str_trim.out:mr_string.o
	clang $(SRC_SAMPLE)/str_trim.c $(OUT_LIB)/mr_string.o -o$(OUT_SAMPLE)/str_trim.out $(INCLUDE)

mr_string.o:$(SRC_LIB)/mr_string.c mr_arraylist.o
	clang $(SRC_LIB)/mr_string.c $(OUT_LIB)/mr_arraylist.o -c -o$(OUT_LIB)/mr_string.o $(INCLUDE)
mr_arraylist.o:$(SRC_LIB)/mr_arraylist.c mr_containers.o
	clang $(SRC_LIB)/mr_arraylist.c $(OUT_LIB)/mr_containers.o -c -o$(OUT_LIB)/mr_arraylist.o $(INCLUDE)
mr_containers.o:$(SRC_LIB)/mr_containers.c
	clang $(SRC_LIB)/mr_containers.c -c -o$(OUT_LIB)/mr_containers.o $(INCLUDE)
clean:
	-rm $(OUT_LIB)/*.o $(OUT_SAMPLE)/*.out
