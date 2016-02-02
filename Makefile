## LIB = -lpthread
INCLUDE = -Iinclude
OUT_LIB = lib
SRC_LIB = src
OUT_SAMPLE = bin
SRC_SAMPLE = samples


###################################################################################################################
#mserver:$(OB)/clients.o $(OB)/utils.o $(OB)/codec.o $(OB)/parser.o mserver.c
#	clang $(OB)/clients.o $(OB)/utils.o $(OB)/codec.o $(OB)/parser.o mserver.c -o mserver $(INCLUDE) $(LIB)
#$(OB)/clients.o:$(OS)/clients.c
#	clang -c $(OS)/clients.c -o $(OB)/clients.o $(INCLUDE)
#$(OB)/utils.o:$(OS)/utils.c
#	clang -c $(OS)/utils.c -o $(OB)/utils.o $(INCLUDE)
#$(OB)/codec.o:$(OS)/codec.c
#	clang -c $(OS)/codec.c -o $(OB)/codec.o $(INCLUDE)
#$(OB)/parser.o:$(OS)/parser.c
#	clang -c $(OS)/parser.c -o $(OB)/parser.o $(INCLUDE)
###################################################################################################################

$(OUT_SAMPLE)/charset_conv:$(OUT_LIB)/mr_string.o
	clang $(SRC_SAMPLE)/charset_conv.c $(OUT_LIB)/mr_string.o -o $(OUT_SAMPLE)/charset_conv $(INCLUDE)
$(OUT_LIB)/mr_string.o:$(SRC_LIB)/mr_string.c
	clang $(SRC_LIB)/mr_string.c -c -o$(OUT_LIB)/mr_string.o $(INCLUDE)

.PHONY:clean
clean:
	-rm $(OUT_LIB)/*.o $(OUT_SAMPLE)/*
