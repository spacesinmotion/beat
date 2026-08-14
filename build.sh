#!/bin/sh

tcc.exe -o click_econemy.exe click_economy.c game/*.c gc/*.c -I../../sokol_prebuild/include/ -I. -lsokol