#!/bin/sh

set -e

if [ "$OSTYPE" != "linux-gnu" ]; then
  LIBS="-lsokol"
else
  LIBS="-lXcursor -lasound -lXi -lX11 -lGL -lm -ldl -lpthread"
fi

tcc -o click_econemy -Werror click_economy.c game/*.c SokEngWrap/extern/gc/*.c -I. -ISokEngWrap/ $LIBS

./click_econemy