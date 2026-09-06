#!/bin/sh

set -e

if [ "$OSTYPE" != "linux-gnu" ]; then
  LIBS="-lsokol"
else
  LIBS="-lXcursor -lasound -lXi -lX11 -lGL -lm -ldl -lpthread"
fi

gcc -o click_econemy -Werror click_economy.c game/*.c engine/extern/gc/*.c -I. $LIBS

./click_econemy