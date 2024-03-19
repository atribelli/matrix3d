# matrix3d.mak

optcpp = /std:c++17 /O2 /EHsc
optc   = /std:c17 /O2 /EHsc
optavx = /arch:AVX2
optas  =

# General C / C++ code and intrinsics

all: matrix3d-loops.exe matrix3d-unroll.exe matrix3d-intrin.exe  matrix3d-avx.exe

matrix3d-loops.exe: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	cl /Fematrix3d-loops $(optcpp) $(optavx) cpuinfo.cpp main.cpp

matrix3d-unroll.exe: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	cl /Fematrix3d-unroll $(optcpp) $(optavx) -DUNROLL cpuinfo.cpp main.cpp

matrix3d-intrin.exe: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp main.cpp
	cl /Fematrix3d-intrin $(optcpp) $(optavx) -DUNROLL -DINTRIN256 cpuinfo.cpp main.cpp

matrix3d-avx.exe: timer.h cpuinfo.h matrix3d.h matrix3d44.h cpuinfo.cpp avx.obj main.cpp
	cl /Fematrix3d-avx $(optcpp) $(optavx) -DUNROLL -DASM256 cpuinfo.cpp avx.obj main.cpp

avx.obj: avx.asm
	ml64 /c /Feavx $(optas) avx.asm

# Quietly clean up

clean:
	del /q *.exe *.obj
