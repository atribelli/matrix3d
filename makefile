# makefile

uname   := $(shell uname)
longbit := $(shell getconf LONG_BIT)

optcpp = -std=c++17 -O3
optc   = -std=c17 -O3
#optdb  = -g

ifeq ($(uname), Linux)  # Linux assembly needs GNU specific section
optas  = --defsym IsLinux=1
else
optas  =
endif

ifeq ($(longbit), 32)   # ARM-v7a is the only supported 32-bit platform
optneon = -march=armv7-a -mfpu=neon-vfpv3
else
optneon = -march=armv8-a
endif

optavx  = -march=haswell



#-------------------------------------------------------------------------------
# Intel code

intel: matrix3d-loops matrix3d-unroll matrix3d-intrin matrix3d-avx

matrix3d-loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-loops $(optcpp) $(optavx) cpuinfo.cpp main.cpp

matrix3d-unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-unroll $(optcpp) $(optavx) -DUNROLL cpuinfo.cpp main.cpp

matrix3d-intrin: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-intrin $(optcpp) $(optavx) -DUNROLL -DINTRIN256 cpuinfo.cpp main.cpp

matrix3d-avx: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp avx.o main.cpp
	g++ $(optdb) -o matrix3d-avx $(optcpp) $(optavx) -DUNROLL -DASM256 cpuinfo.cpp avx.o main.cpp

avx.o: avx.s
	as $(optdb) -o avx.o $(optas) avx.s



#-------------------------------------------------------------------------------
# ARM64 code

arm64: matrix3d-a64loops matrix3d-a64unroll matrix3d-a64intrin matrix3d-neon64

matrix3d-a64loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-a64loops $(optcpp) $(optneon) cpuinfo.cpp main.cpp

matrix3d-a64unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-a64unroll $(optcpp) $(optneon) -DUNROLL cpuinfo.cpp main.cpp

matrix3d-a64intrin: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-a64intrin $(optcpp) $(optneon) -DUNROLL -DINTRIN cpuinfo.cpp main.cpp

matrix3d-neon64: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp neon64.o main.cpp
	g++ $(optdb) -o matrix3d-neon64 $(optcpp) $(optneon) -DUNROLL -DASM cpuinfo.cpp neon64.o main.cpp

neon64.o: neon.s
	as $(optdb) -o neon64.o $(optas) neon.s



#-------------------------------------------------------------------------------
# ARM32 code

arm32: matrix3d-a32loops matrix3d-a32unroll matrix3d-a32intrin # matrix3d-neon32

matrix3d-a32loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-a32loops $(optcpp) $(optneon) cpuinfo.cpp main.cpp

matrix3d-a32unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-a32unroll $(optcpp) $(optneon) -DUNROLL cpuinfo.cpp main.cpp

matrix3d-a32intrin: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	g++ $(optdb) -o matrix3d-a32intrin $(optcpp) $(optneon) -DUNROLL -DINTRIN cpuinfo.cpp main.cpp

matrix3d-neon32: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp neon32.o main.cpp
	g++ $(optdb) -o matrix3d-neon32 $(optcpp) $(optneon) -DUNROLL -DASM cpuinfo.cpp neon32.o main.cpp

neon32.o: neon.s
	as $(optdb) -o neon32.o $(optas) neon.s



#-------------------------------------------------------------------------------
# Quietly clean up

clean:
	rm -f matrix3d-loops matrix3d-unroll matrix3d-intrin matrix3d-avx matrix3d-a64loops matrix3d-a64unroll matrix3d-a64intrin matrix3d-neon64 matrix3d-a32loops matrix3d-a32unroll matrix3d-a32intrin matrix3d-neon32 a.out *.o
