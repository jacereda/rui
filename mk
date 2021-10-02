#!/usr/bin/env bash
set -e
killall demo || echo "no demo"
killall ruiview || echo "no ruiview"
wayland-scanner client-header $XDGSHELL glcv/src/xdg-shell.h
wayland-scanner public-code $XDGSHELL glcv/src/xdg-shell.c
install -d b
cc -Wall -Os -U_FORTIFY_SOURCE `pkg-config wayland-client xkbcommon wayland-egl egl gl --cflags --libs` -I glcv/src glcv/src/wl.c ruiview.c -o b/ruiview
# cc -Wall -Og -g -U_FORTIFY_SOURCE `pkg-config x11 xcursor xrender egl gl --cflags --libs` -I glcv/src glcv/src/xlib.c ruiview.c -o b/ruiview
b/ruiview &
cosmoc -Ifake -Wall -Og -g -I microui/src -I microui/demo demo.c rui.c microui/src/microui.c -o b/demo
objcopy -Obinary b/demo b/demo.com
b/demo
