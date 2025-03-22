@REM Build for MINGW64 or 32 from MSYS2.
@set OUT_DIR=Output
@set OUT_EXE=DiskBlocks
@set INCLUDES=-I imgui/ -I imgui/backends
@set SOURCES=main.cpp disk_crawler.cpp ui_helper.cpp imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_win32.cpp imgui/imgui*.cpp
@set LIBS=-lopengl32 -lgdi32 -ldwmapi
mkdir %OUT_DIR%
g++ -DUNICODE %INCLUDES% %SOURCES% -o %OUT_DIR%/%OUT_EXE%.exe --static -mwindows %LIBS% %LIBS%
pause
