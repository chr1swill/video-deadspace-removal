#
CFLAGS:=-O3 -Wall -Wextra
DEBUG ?= 0

ifeq ($(DEBUG), 1)
    CFLAGS += -ggdb -pg
endif

ifeq ($(shell uname), Darwin) 
    ifeq ($(shell [ -d "/opt/homebrew" ] && echo yes), yes) 
        LIBPNG_PATH:=/opt/homebrew
    else 
        LIBPNG_PATH:=/usr/local
    endif

    INCLUDE_PATH:=-I$(LIBPNG_PATH)/include
    LIBRARY_PATH:=-L$(LIBPNG_PATH)/lib
else 
    INCLUDE_PATH:=
    LIBRARY_PATH:=
endif

main: clean main.c libz.c
	gcc -o bin/main main.c libz.c $(INCLUDE_PATH) $(LIBRARY_PATH) -lpng $(CFLAGS)

load_png_test: clean libz.c load_png_test.c
	gcc -o bin/load_png_test load_png_test.c libz.c $(INCLUDE_PATH) $(LIBRARY_PATH) -lpng $(CFLAGS)

clean: 
	rm -rf bin
	mkdir -p bin

