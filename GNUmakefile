SOURCES=$(addprefix src/, $(addsuffix .cpp, Snipes skill $(addprefix sdl/, console keyboard sound timer uifont startscreen startscreen_logic hudformat hud layout)))
CFLAGS=-std=c++11 $(shell sdl2-config --cflags)
# Sources use <SDL2/...> includes, so also add the parent include dir (needed for Homebrew)
CFLAGS+=-I$(shell sdl2-config --prefix)/include
CFLAGS+=$(if $(MAINT),-Werror -Wall -Wextra,)
CFLAGS2=-O3 -fstack-protector
#CFLAGS2=-Og -g -fsanitize=address -fsanitize=undefined
LDFLAGS=$(shell sdl2-config --libs) -lm -lSDL2_ttf
CXX=g++
#CXX="clang++"

snipes : $(SOURCES) $(wildcard src/*.h src/sdl/*.h) src/config.h GNUmakefile
	$(CXX) -o $@ $(CFLAGS) $(CFLAGS2) $(SOURCES) $(LDFLAGS)

src/config.h :
	@echo "Automatically creating src/config.h with default settings - edit this file to customize your build"
	cp src/config-sample.h src/config.h

clean :
	rm -f snipes snipes-tests

TEST_SOURCES=tests/test_main.cpp src/skill.cpp src/sdl/hudformat.cpp src/sdl/startscreen_logic.cpp src/sdl/layout.cpp

snipes-tests : $(TEST_SOURCES) $(wildcard src/*.h src/sdl/*.h) GNUmakefile
	$(CXX) -std=c++11 -Wall -Wextra -o $@ $(TEST_SOURCES)

test : snipes-tests
	./snipes-tests

.PHONY : clean test
