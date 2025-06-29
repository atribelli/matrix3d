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

else ifeq ($(platform), arm64)

$(info ARM detected)
optarch = -march=armv8-a
target  = arm64

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

else ifeq ($(platform), aarch64)

$(info ARM detected)
optarch = -march=armv8-a
target  = arm64

endif   # ARM, Intel, 32-bit
endif   # Linux, Darwin

all: $(target)
	@echo $(target) done



#-------------------------------------------------------------------------------
# Intel code

intel: cpuid matrix3d-loops matrix3d-unroll matrix3d-intrin matrix3d-avx

cpuid: cpuinfo.o cpuid.o
	gcc $(optdb) -o cpuid $(optc) $(optarch) cpuinfo.o cpuid.o

cpuid.o: cpuinfo.h cpuid.c
	gcc $(optdb) -o cpuid.o -c $(optc) $(optarch) cpuid.c

cpuinfo.o: midr.h cpuinfo.h cpuinfo.c
	gcc $(optdb) -o cpuinfo.o -c $(optc) $(optarch) cpuinfo.c

matrix3d-loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o main.cpp
	g++ $(optdb) -o matrix3d-loops $(optcpp) $(optarch) cpuinfo.o main.cpp

matrix3d-unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o main.cpp
	g++ $(optdb) -o matrix3d-unroll $(optcpp) $(optarch) -DUNROLL cpuinfo.o main.cpp

matrix3d-intrin: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o main.cpp
	g++ $(optdb) -o matrix3d-intrin $(optcpp) $(optarch) -DUNROLL -DINTRIN256 cpuinfo.o main.cpp

matrix3d-avx: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o avx.o main.cpp
	g++ $(optdb) -o matrix3d-avx $(optcpp) $(optarch) -DUNROLL -DASM256 cpuinfo.o avx.o main.cpp

avx.o: avx.s
	as $(optdb) -o avx.o $(optas) avx.s



#-------------------------------------------------------------------------------
# ARM64 code

arm64: a64cpuid matrix3d-a64loops matrix3d-a64unroll matrix3d-a64intrin matrix3d-a64neon matrix3d-a64sme

a64cpuid: cpuinfo.o a64midr.o cpuid.o
	gcc $(optdb) -o a64cpuid $(optc) $(optarch) cpuinfo.o a64midr.o cpuid.o

a64cpuid.o: cpuinfo.h cpuid.c
	gcc $(optdb) -o a64cpuid.o -c $(optc) $(optarch) cpuid.c

a64cpuinfo.o: midr.h cpuinfo.h cpuinfo.c
	gcc $(optdb) -o a64cpuinfo.o -c $(optc) $(optarch) cpuinfo.c

a64midr.o: midr.h midr-a64.s
	as $(optdb) -o a64midr.o $(optas) midr-a64.s

matrix3d-a64loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o a64midr.o  main.cpp
	g++ $(optdb) -o matrix3d-a64loops $(optcpp) $(optarch) cpuinfo.o a64midr.o main.cpp

matrix3d-a64unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o a64midr.o main.cpp
	g++ $(optdb) -o matrix3d-a64unroll $(optcpp) $(optarch) -DUNROLL cpuinfo.o a64midr.o main.cpp

matrix3d-a64intrin: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o a64midr.o main.cpp
	g++ $(optdb) -o matrix3d-a64intrin $(optcpp) $(optarch) -DUNROLL -DINTRIN cpuinfo.o a64midr.o main.cpp

matrix3d-a64neon: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o a64midr.o a64neon.o main.cpp
	g++ $(optdb) -o matrix3d-a64neon $(optcpp) $(optarch) -DUNROLL -DASM cpuinfo.o a64midr.o a64neon.o main.cpp

a64neon.o: neon.s
	as $(optdb) -o a64neon.o $(optas) neon.s

matrix3d-a64sme: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o a64midr.o a64sme.o main.cpp
	g++ $(optdb) -o matrix3d-a64sme $(optcpp) $(optarch) -DUNROLL -DASM cpuinfo.o a64midr.o a64sme.o main.cpp

a64sme.o: sme.s
	as $(optdb) -o a64sme.o $(optas) sme.s



#-------------------------------------------------------------------------------
# ARM32 code

arm32: a32cpuid matrix3d-a32loops matrix3d-a32unroll

a32cpuid: cpuinfo.o cpuid.o
	gcc $(optdb) -o a32cpuid $(optc) $(optarch) cpuinfo.o cpuid.o

a32cpuid.o: cpuinfo.h cpuid.c
	gcc $(optdb) -o a32cpuid.o -c $(optc) $(optarch) cpuid.c

a32cpuinfo.o: midr.h cpuinfo.h cpuinfo.c
	gcc $(optdb) -o a32cpuinfo.o -c $(optc) $(optarch) cpuinfo.c

matrix3d-a32loops: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o main.cpp
	g++ $(optdb) -o matrix3d-a32loops $(optcpp) $(optarch) cpuinfo.o main.cpp

matrix3d-a32unroll: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.o main.cpp
	g++ $(optdb) -o matrix3d-a32unroll $(optcpp) $(optarch) -DUNROLL cpuinfo.o main.cpp



#-------------------------------------------------------------------------------
# Quietly clean up

clean:
	rm -f cpuid matrix3d-loops matrix3d-unroll matrix3d-intrin matrix3d-avx a64cpuid matrix3d-a64loops matrix3d-a64unroll matrix3d-a64intrin matrix3d-a64neon matrix3d-a64sme a32cpuid matrix3d-a32loops matrix3d-a32unroll a.out *.o
