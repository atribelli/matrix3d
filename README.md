# matrix3d  
Intel AVX2 and ARM NEON matrix and vector code.

This is the ```main``` branch of the repository.

This is a testbed for experimenting with SIMD implementations. For comparison purposes there are also classic looped and unrolled C++ implementations. *C++ template specialization* is used to build different executables for looped, unrolled, SIMD intrinsics, and SIMD assembly language implementations. Note that intrinsic implementations generally have an advantage over assembly since assembly code is in a function rather than inlined. If you do not need to have your code work across various platforms and compilers you might try *C++ inline assembly*.

This code is mostly an educational exercise. In a real-world application, I would probably use something like the cross platform *OpenGL Mathematics* library.

## Files  
makefile - macOS and Linux builds.  
matrix3d.mak - Windows builds.  
matrix3d.h - Generic C++ implementations.  
matrix3d44.h - Unrolled C++ and intrinsics implementations.  
timer.h - Determine elapsed time.  
cpuinfo.h  
cpuinfo.cpp - Gets CPU info and features.  
avx.asm - Intel assembly implementation for Windows.  
avx.s - Intel assembly implementations for macOS and Linux.  
neon.s - ARM assembly implementation for Linux.  
main.cpp - Testing and timing code.  
params.txt - Inputs for testing code.

Inside the *mac* and *win* subfolders you will find Xcode and Visual Studio projects for development and debugging.

## Building  
make intel - Builds executables for Intel: matrix3d-loops, matrix3d-unroll, matrix3d-intrin, and matrix3d-avx.  
make arm64 - Builds executables for ARM64: matrix3d-a64loops, matrix3d-a64unroll, matrix3d-a64intrin, and matrix3d-neon64.  
make arm32 - Builds executables for ARM32: matrix3d-a32loops, matrix3d-a32unroll, and matrix3d-a32intrin.  
make clean - Removes executable and build files.  
nmake /f matrix3d.mak - Builds executables for Windows: matrix3d-loops, matrix3d-unroll, matrix3d-intrin, and matrix3d-avx.  
nmake /f matrix3d.mak clean - Removes executable and build files under Windows.

## Testing  
Intel based Mac.  
Windows PC.  
Linux PC.  
Raspberry Pi 64-bit.  
Raspberry Pi 32-bit.

## Code  
C++ Templates are used so that I can conveniently test code on different data types in the same executable, and to handle arbitrary matrix and vector dimensions. In a "real world" application I might define a ```real``` type that is an alias for whatever actual type I wanted to use and just use templates for things like automatically matching matrix dimensions (Ex RxC = RxK * KxC).

I am using two layers of *C++ template specializations*. The first layer is used for the unrolled code where I implement algorithms for specific matrix dimensions (Ex 4x4). Note that the floating-point type is still templatized. The second layer is for the SIMD code and uses even more detailed *template specializations* to implement algorithms for a specific floating-point type and a specific dimension (Ex ```float``` 4x4). A wrapper for calling assembly language functions is similar to the intrinsics template. These specialized implementations will automatically replace the general implementations for the matching template parameters.

Experience has shown that different implementations may be faster depending on the underlying hardware architecture and the compiler used. The code uses user defined compiler macros to choose between general C++, unrolled C++, SIMD intrinsics, and SIMD assembly language.  
UNROLL - Unrolled template specializations.  
INTRIN - SIMD intrinsics template specializations. The code will use predefined compiler macros to recognize the architecture and automatically include the appropriate AVX or NEON intrinsics headers.  
INTRIN256 - Same as SIMD macro but also has ```float``` code use 8 lanes, to process two vectors at a time.  
ASM - SIMD assemblty language template specializations.  
ASM256 - Same as ASM macro but with 8 lane ```float``` code.

## Example  
The template specialization used for a calculation is shown next to the timing information.  
```
% make intel
g++  -o matrix3d-loops -std=c++17 -O3 -march=haswell cpuinfo.cpp main.cpp
g++  -o matrix3d-unroll -std=c++17 -O3 -march=haswell -DUNROLL cpuinfo.cpp main.cpp
g++  -o matrix3d-intrin -std=c++17 -O3 -march=haswell -DUNROLL -DINTRIN256 cpuinfo.cpp main.cpp
as  -o avx.o  avx.s
g++  -o matrix3d-avx -std=c++17 -O3 -march=haswell -DUNROLL -DASM256 cpuinfo.cpp avx.o main.cpp
% ./matrix3d-loops 
Intel(R) Core(TM) i5-8259U CPU @ 2.30GHz
mata  4x4 * matb  4x4 float  test passed
mata  4x4 * matb  4x4 double test passed
matb  4x4 * mata  4x4 float  test passed
matb  4x4 * mata  4x4 double test passed
vec[] 1x4 * mat   4x4 float  test passed
vec[] 1x4 * mat   4x4 double test passed
mat   4x4 * vec[] 4x1 float  test passed
mat   4x4 * vec[] 4x1 double test passed
mata  3x2 * matb  2x3 int    test passed
mata  2x3 * matb  3x2 int    test passed
mata  3x3 * matb  3x3 int    test passed
matb  2x3 * mata  3x2 int    test passed
matb  3x2 * mata  2x3 int    test passed
matb  3x3 * mata  3x3 int    test passed
iterations          1,000,000,000
vec array elements  300
                float               double
mata x matb    1,382 ms primary     1,393 ms primary 
matb x mata    1,357 ms primary     1,376 ms primary 
vec[] x mat    1,224 ms primary     1,214 ms primary 
mat x vec[]    1,228 ms primary     1,204 ms primary 
% ./matrix3d-unroll 
...
                float               double
mata x matb    1,416 ms unroll      1,464 ms unroll  
matb x mata    1,490 ms unroll      1,368 ms unroll  
vec[] x mat    1,231 ms unroll      1,205 ms unroll  
mat x vec[]    1,221 ms unroll      1,193 ms unroll  
% ./matrix3d-intrin 
...
                float               double
mata x matb    1,412 ms intel       1,390 ms intel   
matb x mata    1,413 ms intel       1,356 ms intel   
vec[] x mat      656 ms intel256      747 ms intel   
mat x vec[]      654 ms intel256      741 ms intel   
% ./matrix3d-avx
...
                float               double
mata x matb    3,887 ms avx         3,855 ms avx     
matb x mata    3,856 ms avx         3,865 ms avx     
vec[] x mat      719 ms avx256        896 ms avx     
mat x vec[]      725 ms avx256        892 ms avx     
```

## To-do
```double``` implementation for ARM.  
NEON implementation for ARM32.  
Background task - reviewing and improving the code.  
