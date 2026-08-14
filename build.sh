#!/bin/sh

set -e

tcc.exe -o click_econemy click_economy.c game/*.c gc/*.c -I../../sokol_prebuild/include/ -I. -lsokol

./click_econemy