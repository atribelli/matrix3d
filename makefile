# makefile



#-----------------------------------------------------------------------------
# Determine the current environment

platform := $(shell uname -m)
kernel   := $(shell uname -s)
wordsize := $(shell getconf LONG_BIT)

# Set variables for the current environment
# and determine which set of build commands to execute

optcpp = -std=c++17 -O3
optc   = -std=c17 -O3
#optdb  = -g

ifeq ($(kernel), Darwin)

$(info macOS detected)
optas =

ifeq ($(platform), x86_64)

$(info Intel detected)
optarch = -march=haswell
target  = intel
simd    = avx

else ifeq ($(platform), arm64)

$(info ARM detected)
optarch = -march=armv8-a
optsve  = -march=armv8-a+sve
optsme  = -march=armv9-a+sme
target  = arm64
simd    = neon sme
headers = midr.h
objs    = midr.o

endif   # ARM, Intel

else ifeq ($(kernel), Linux)

$(info Linux detected)
optas = --defsym IsLinux=1  # Linux assembly needs GNU specific section

ifeq ($(wordsize), 32)      # ARM-v7a is the only supported 32-bit architecture

$(info ARM32 detected)
optarch = -march=armv7-a -mfpu=neon-vfpv3
target  = arm32

else ifeq ($(platform), x86_64)

$(info Intel detected)
optarch = -march=haswell
target  = intel
simd    = avx

else ifeq ($(platform), aarch64)

$(info ARM detected)
optarch = -march=armv8-a
target  = arm64
simd    = neon

endif   # ARM, Intel, 32-bit
endif   # Linux, Darwin



#-------------------------------------------------------------------------------
# Common code

all: cpuid loops unroll intrin $(simd)

cpuid: cpuid.o cpuinfo.o $(objs)
	g++ $(optdb) -o cpuid $(optarch) $(optcpp) cpuid.o cpuinfo.o $(objs)

cpuid.o: cpuinfo.h cpuid.c
	gcc $(optdb) -o cpuid.o -c $(optarch) $(optc) cpuid.c

cpuinfo.o: cpuinfo.h $(headers) cpuinfo.c
	gcc $(optdb) -o cpuinfo.o -c  $(optarch) $(optc) cpuinfo.c
	
loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o $(objs)
	g++ $(optdb) -o loops $(optarch) $(optcpp) main.cpp cpuinfo.o $(objs)

unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o $(objs)
	g++ $(optdb) -o unroll $(optarch) $(optcpp) -DUNROLL main.cpp cpuinfo.o $(objs)

intrin: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o $(objs)
	g++ $(optdb) -o intrin $(optarch) $(optcpp) -DUNROLL -DINTRIN main.cpp cpuinfo.o $(objs)

neon: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o neon.o $(objs)
	g++ $(optdb) -o neon $(optarch) $(optcpp) -DUNROLL -DASM main.cpp cpuinfo.o neon.o $(objs)

sve: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o sve.o $(objs)
	g++ $(optdb) -o sve $(optarch) $(optcpp) -DUNROLL -DASM main.cpp cpuinfo.o sve.o $(objs)

sme: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o sme.o $(objs)
	g++ $(optdb) -o sme $(optarch) $(optcpp) -DUNROLL -DASM main.cpp cpuinfo.o sme.o $(objs)

avx: timer.h cpuinfo.h matrix3d.h matrix3d44.h main.cpp cpuinfo.o avx.o $(objs)
	g++ $(optdb) -o avx $(optarch) $(optcpp) -DUNROLL -DASM main.cpp cpuinfo.o avx.o $(objs)



#-------------------------------------------------------------------------------
# Intel code

avx.o: avx.s
	as $(optdb) -o avx.o $(optarch) $(optas) avx.s



#-------------------------------------------------------------------------------
# ARM64 code

midr.o: midr-a64.s
	as $(optdb) -o midr.o $(optarch) $(optas) midr-a64.s

neon.o: neon.s
	as $(optdb) -o neon.o $(optarch) $(optas) neon.s

sve.o: sve.s
	as $(optdb) -o sve.o  $(optsve) $(optas) sve.s

sme.o: sme.s
	as $(optdb) -o sme.o $(optsme) $(optas) sme.s



#-------------------------------------------------------------------------------
# ARM32 code



#-------------------------------------------------------------------------------
# Quietly clean up

clean:
	rm -f cpuid loops unroll intrin avx neon sve sme a.out *.o
