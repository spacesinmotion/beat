#!/bin/sh

set -e

if [ "$OSTYPE" != "linux-gnu" ]; then
  LIBS="-lsokol"
else
  LIBS="-lXcursor -lasound -lXi -lX11 -lGL -lm"
fi

tcc -o click_econemy click_economy.c game/*.c gc/*.c -Iextern/sokol -I. $LIBS

./click_econemy