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
midr.h  
cpuinfo.h  
cpuinfo.c - Gets CPU info and features.  
cpuid.c - Displays cpu info.  
avx.asm - Intel assembly implementation for Windows.  
avx.s - Intel assembly implementations for macOS and Linux.  
neon.s - ARM assembly implementation for macOS and Linux.  
sme.s  
main.cpp - Testing and timing code.  
params.txt - Inputs for testing code.

Inside the *mac* and *win* subfolders you will find Xcode and Visual Studio projects for development and debugging.

## Building  
make - Detects OS and architecture and builds intel, arm64, or arm32 code.  
intel: cpuid, matrix3d-loops, matrix3d-unroll, matrix3d-intrin, and matrix3d-avx.  
arm64: a64cpuid, matrix3d-a64loops, matrix3d-a64unroll, matrix3d-a64intrin, and matrix3d-a64neon.  
arm32: a32cpuid, matrix3d-a32loops, matrix3d-a32unroll, and matrix3d-a32intrin.  
make clean - Remove executable and build files.  
nmake /f matrix3d.mak - Builds executables for Windows: matrix3d-loops, matrix3d-unroll, matrix3d-intrin, and matrix3d-avx.  
nmake /f matrix3d.mak clean - Removes executable and build files under Windows.

## Testing  
Intel based Mac.  
ARM based Mac.  
Intel based Windows PC.  
ARM based Windows PC (Virtualized on Mac).  
Intel based Linux PC.  
ARM based Linux PC (Virtualized on Mac).  
Raspberry Pi 64-bit (ARM64 Linux).  
Raspberry Pi 32-bit (ARM32 Linux).  

## Code  
C++ Templates are used so that I can conveniently test code on different data types in the same executable, and to handle arbitrary matrix and vector dimensions. In a "real world" application I might define a ```real``` type that is an alias for whatever actual type I wanted to use and just use templates for things like automatically matching matrix dimensions (Ex RxC = RxK * KxC).

I am using two layers of *C++ template specializations*. The first layer is used for the unrolled code where I implement algorithms for specific matrix dimensions (Ex 4x4). Note that the floating-point type is still templatized. The second layer is for the SIMD code and uses even more detailed *template specializations* to implement algorithms for a specific floating-point type and a specific dimension (Ex ```float``` 4x4). A wrapper for calling assembly language functions is similar to the intrinsics template. These specialized implementations will automatically replace the general implementations for the matching template parameters.

Experience has shown that different implementations may be faster depending on the underlying hardware architecture and the compiler used. The code uses user defined compiler macros to choose between general C++, unrolled C++, SIMD intrinsics, and SIMD assembly language.  
UNROLL - Unrolled template specializations.  
INTRIN - SIMD intrinsics template specializations. The code will use predefined compiler macros to recognize the architecture and automatically include the appropriate AVX or NEON intrinsics headers.  
INTRIN256 - Same as SIMD macro but also has ```float``` code use 8 lanes, to process two vectors at a time.  
ASM - SIMD assemblty language template specializations.  
ASM256 - Same as ASM macro but with 8 lane ```float``` code.

## Examples  
The template specialization used for a calculation is shown next to the timing information.  
Note assembly language implementations like avx and neon have a disadvantage since they require function calls.  
What C implementation works best. The relative rankings of C, simd intrinsics, or simd asm. Will all vary depending on system architecture and CPU generation.
```
$ make
Linux detected
Intel detected
...
$ ./matrix3d-loops
GenuineIntel 11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz Family 6 Model 140 4-Core 
SSE3 SSE4.2 AVX AVX2 GEN4 AVX512-F-CD AVX512-VL-DQ-BW AVX512-IFMA-VBMI 
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
mata x matb      288 ms loops         295 ms loops   
matb x mata      263 ms loops         324 ms loops   
vec[] x mat      988 ms loops       1,962 ms loops   
mat x vec[]      987 ms loops       1,967 ms loops   
$ ./matrix3d-unroll
...
                float               double
mata x matb      340 ms unroll        287 ms unroll  
matb x mata      274 ms unroll        283 ms unroll  
vec[] x mat      484 ms unroll        755 ms unroll  
mat x vec[]      487 ms unroll        763 ms unroll  
$ ./matrix3d-intrin 
...
                float               double
mata x matb      993 ms intel       1,015 ms intel   
matb x mata      987 ms intel         998 ms intel   
vec[] x mat      486 ms intel256      737 ms intel   
mat x vec[]      485 ms intel256      736 ms intel   
$ ./matrix3d-avx
...
                float               double
mata x matb    3,569 ms avx         3,677 ms avx     
matb x mata   16,323 ms avx         3,680 ms avx     
vec[] x mat      559 ms avx256        740 ms avx     
mat x vec[]      560 ms avx256        738 ms avx     

% make  
macOS detected
ARM detected
...
% ./matrix3d-a64loops
Apple M4 Armv9 10-Core 
Performance:4 Efficiency:6 NEON SME SME2 
...
                float               double
mata x matb      301 ms loops         346 ms loops   
matb x mata      291 ms loops         263 ms loops   
vec[] x mat      493 ms loops         978 ms loops   
mat x vec[]      491 ms loops         977 ms loops   
% ./matrix3d-a64unroll
...
                float               double
mata x matb      345 ms unroll        306 ms unroll  
matb x mata      311 ms unroll        320 ms unroll  
vec[] x mat      487 ms unroll        974 ms unroll  
mat x vec[]      489 ms unroll        974 ms unroll  
% ./matrix3d-a64intrin
...
                float               
mata x matb      341 ms intrin      
matb x mata      347 ms intrin      
vec[] x mat      431 ms intrin      
mat x vec[]      432 ms intrin      
% ./matrix3d-a64neon  
...
                float               
mata x matb    2,278 ms neon        
matb x mata    2,248 ms neon        
vec[] x mat      946 ms neon        
mat x vec[]      945 ms neon        

$ make
Linux detected
ARM32 detected
...
$ ./matrix3d-a32loops
ARM Raspberry Pi 3 Model B Plus Rev 1.3 Cortex-A53 4-Core 
half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt vfpd32 lpae evtstrm crc32 
...
                float               double
mata x matb    5,759 ms loops       5,842 ms loops   
matb x mata    5,841 ms loops       5,841 ms loops   
vec[] x mat   25,090 ms loops      28,430 ms loops   
mat x vec[]   25,090 ms loops      28,428 ms loops   
$ ./matrix3d-a32unroll
...
                float               double
mata x matb    5,882 ms unroll      5,841 ms unroll  
matb x mata    5,841 ms unroll      5,841 ms unroll  
vec[] x mat   25,088 ms unroll     25,089 ms unroll  
mat x vec[]   25,085 ms unroll     25,090 ms unroll  
$ ./matrix3d-a32intrin
...
                float              
mata x matb   81,009 ms intrin     
matb x mata   80,974 ms intrin     
vec[] x mat   27,610 ms intrin     
mat x vec[]   27,614 ms intrin     
```

## To-do
```double``` implementation for ARM.  
NEON implementation for ARM32.  
Background task - reviewing and improving the code.  
