#!/bin/sh

set xe

mkdir -p bin

if [ "$(uname)" = "Darwin" ]; then

  if [ -d "/opt/homebrew" ]; then 
    LIBPNG_PATH="/opt/homebrew"
  else
    LIBPNG_PATH="/usr/local"
  fi
  INCLUDE_PATH="-I${LIBPNG_PATH}/include"
  LIBRARY_PATH="-L${LIBPNG_PATH}/lib"
else
  INCLUDE_PATH=""
  LIBRARY_PATH=""
fi

gcc -o bin/main main.c ${INCLUDE_PATH} ${LIBRARY_PATH} -lpng -O3 -Wall -Wextra
