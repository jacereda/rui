setlocal
mkdir b
taskkill /IM demo.exe
taskkill /IM ruiview.exe
set flags=/DCV_DYN /nologo /link /subsystem:console /MANIFEST:NO /stack:0x800000,0x800000
cl /I glcv/src glcv/src/win.c ruiview.c gl.c /Fe:b/ruiview.exe gdi32.lib user32.lib ws2_32.lib %flags%
start b\ruiview
cl /I microui/src -I microui/demo demo.c rui.c microui/src/microui.c /Fe:b/demo.exe ws2_32.lib %flags%
b\demo
endlocal
