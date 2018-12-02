.PHONY=all tests clean obj update
CXX?=g++
CC?=gcc

GMATCH=$(findstring g++,$(CXX))

CLHASH_CHECKOUT = "&& git checkout master"
WARNINGS=-Wall -Wextra -Wno-char-subscripts \
		 -Wpointer-arith -Wwrite-strings -Wdisabled-optimization \
		 -Wformat -Wcast-align -Wno-unused-function -Wno-unused-parameter \
		 -pedantic -DUSE_PDQSORT -Wunused-variable -Wno-ignored-qualifiers
EXTRA?=
DBG?=
INCPLUS?=
EXTRA_LD?=
DBG:=
OS:=$(shell uname)
FLAGS=

OPT_MINUS_OPENMP= -O3 -funroll-loops\
	  -pipe -fno-strict-aliasing -march=native $(FLAGS) $(EXTRA)
OPT=$(OPT_MINUS_OPENMP) -fopenmp
XXFLAGS=
CXXFLAGS=$(OPT) $(XXFLAGS) -std=c++17 $(WARNINGS)
CXXFLAGS_MINUS_OPENMP=$(OPT_MINUS_OPENMP) $(XXFLAGS) -std=c++1z $(WARNINGS) -Wno-cast-align -Wno-gnu-zero-variadic-macro-arguments
CCFLAGS=$(OPT) $(CFLAGS) -std=c11 $(WARNINGS)
LIB=-lz -lsdsl
LD=-L. $(EXTRA_LD)
LD+= -Lsdsl-lite/lib

ifneq (,$(findstring g++,$(CXX)))
	ifeq ($(shell uname),Darwin)
		ifeq (,$(findstring clang,$(CXX)))
			POPCNT_CXX:=clang
		else
			POPCNT_CXX:=$(CXX)
		endif
	else
		POPCNT_CXX:=$(CXX)
	endif
endif

OBJS=$(patsubst %.c,%.o,$(wildcard src/*.c) bonsai/klib/kthread.o) $(patsubst %.cpp,%.o,$(wildcard src/*.cpp)) bonsai/klib/kstring.o clhash.o
DOBJS=$(patsubst %.c,%.do,$(wildcard src/*.c) bonsai/klib/kthread.o) $(patsubst %.cpp,%.do,$(wildcard src/*.cpp)) bonsai/klib/kstring.o clhash.o

INCLUDE+=-I. -Isdsl-lite/include

EX=$(patsubst src/%.cpp,%,$(wildcard src/*.cpp))


all: $(EX)

sdsl-lite/lib/libsdsl.a:
	cd sdsl-lite && cmake . && make -j8 && cd ..

%: src/%.cpp sdsl-lite/lib/libsdsl.a
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) -DNDEBUG $< -o $@ $(LIB)
clean:
	rm -f $(EX) $(D_EX) libzstd.a
mostlyclean: clean
