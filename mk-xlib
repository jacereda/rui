#!/usr/bin/env bash
set -e
killall demo || echo "no demo"
killall ruiview || echo "no ruiview"
wayland-scanner client-header $XDGSHELL glcv/src/xdg-shell.h
wayland-scanner public-code $XDGSHELL glcv/src/xdg-shell.c
install -d b
cc -Wall -Os -U_FORTIFY_SOURCE `pkg-config wayland-client xkbcommon wayland-egl egl --cflags --libs` -ldl -I glcv/src glcv/src/wl.c ruiview.c gl.c -o b/ruiview
#cc -Wall -Og -g -U_FORTIFY_SOURCE `pkg-config x11 xcursor xrender gl --cflags --libs` -I glcv/src glcv/src/xlib.c ruiview.c gl.c -ldl -o b/ruiview
b/ruiview &
cc -Wall -Og -g -U_FORTIFY_SOURCE -I microui/src -I microui/demo demo.c rui.c microui/src/microui.c -o b/demo
b/demo
