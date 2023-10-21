# Created by: Jonas Mehtali

.PHONY: all
all: clean compile

project_prefix = ../../../..

#define shaderc path
shaderc = $(project_prefix)/build/stage/bin/shaderc
INCLUDE1 = $(project_prefix)/thirdparty/bgfx.cmake/bgfx/examples/thirdparty/thirdparty/common
INCLUDE2 = $(project_prefix)/thirdparty/bgfx.cmake/bgfx/src
.PHONY: compile
compile:
	$(shaderc) -i $(INCLUDE1)  -i $(INCLUDE2) -f vs_cube.sc -o compiled/vs_cube_compiled --type "vertex" -p 150
	$(shaderc) -i $(INCLUDE1)  -i $(INCLUDE2) -f fs_cube.sc -o compiled/fs_cube_compiled --type "fragment" -p 150

.PHONY: clean
clean:
	rm -rf compiled/*