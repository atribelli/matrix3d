//  main.cpp
//  Assuming C++17
//
//  Mac:
//      clang++ -std=c++17 -march=haswell -O3 main.cpp cpuinfo.cpp
//      clang++ -std=c++17 -march=haswell -O3 -DASM main.cpp cpuinfo.cpp avx.s
//  Linux:
//      g++ -std=c++17 -march=haswell -O3 main.cpp cpuinfo.cpp
//      as -o avx.o --defsym IsLinux=1 avx.s
//      g++ -std=c++17 -march=haswell -O3 -DIsLinux -DASM main.cpp cpuinfo.cpp avx.o
//  Raspberry Pi
//      g++ -std=c++17 -march=armv8-a -O3 main.cpp cpuinfo.cpp
//      g++ -std=c++17 -march=armv8-a -O3 -DASM main.cpp cpuinfo.cpp neon.s
//      g++ -std=c++17 -march=armv7-a -mfpu=neon-vfpv3 -O3 main.cpp cpuinfo.cpp
//      g++ -std=c++17 -march=armv7-a -mfpu=neon-vfpv3 -O3 main.cpp cpuinfo.cpp neon.s
//  Windows:
//      cl /std:c++17 /arch:AVX2 /O2 /EHsc main.cpp cpuinfo.cpp
//      ml64 /c /Feavx avx.asm
//      cl /std:c++17 /arch:AVX2 /O2 /EHsc /DASM main.cpp cpuinfo.cpp avx.obj
//
//  To enable unrolled template specializations add:
//      -DUNROLL
//  To enable intrinsics template specializations add one of the following:
//      -DINTRIN
//      -DINTRIN256
//  Build float code to use 8 lanes, process 2 vectors at a time:
//      -DINTRIN256
//  To enable assembly template specializations add one of the following:
//      -DASM
//      -DASM256
//  Build float code to use 8 lanes, process 2 vectors at a time:
//      -DASM256
//  To dump mismatches during testing:
//      -DDUMP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <locale>

#include "timer.h"
#include "cpuinfo.h"
#include "matrix3d.h"
#include "matrix3d44.h"

#if defined(__x86_64__) || defined(_M_X64)      // 64-bit Intel
#include <immintrin.h>
#endif

using namespace std;
using namespace matrix3d;



// -------------------------------------------------------------------------
// Naming convention for matrices and vectors:
// 'd' and 's' prefixes indicate destination and source data.
// 't' prefix indicates testing data.
// 'e' prefix indicates expected results.
// 'f' and 'd' suffixes indicate float and double types.
// 'a' and 'b' in matrices indicate multiplication parameters.
// '0' and '1' in vectors indicate even and odd numbered array elements,
//             SIMD code may process pairs of vectors.
// "rmat" and "rvec" indicates row major order matrices and vectors.
// "cmat" and "cvec" indicates column major order matrices and vectors.
// "arr" indicstes arrays.



// -----------------------------------------------------------------------------
// Compare actual and expected results

// Use ansi escape codes for colorization
char passed[] = "\x1b[32mpassed\x1b[0m";
char failed[] = "\x1b[31mfailed\x1b[0m";

template <size_t N>
void compare_vec(vec<N>  *dvecarr,
                 real       evec0[N],
                 real       evec1[N],
                 int        elements,
                 const char *msg) {
    auto valid = true;
    
    for (int i = 0; i < elements; ++i) {
        for (int j = 0; j < N; ++j) {
            // Even and odd numbered vectors have different expected values
            // since we may process two vectors at a time in SIMD code
            auto expected = (i & 1) ? evec1[j] : evec0[j];
            
            valid = valid && (dvecarr[i].v[j] == expected);
            
// User defined compiler macro that allows mismatches to be displayed
#ifdef DUMP
            if (dvecarr[i].v[j] != expected) {
                cout << " vecarr[" << i << "][" << j << "] " << dvecarr[i].v[j]
                     << " != expected[" << j << "] " << expected << endl;
            }
#endif
        }
    }

    // Overall results
    cout << msg << (valid ? passed : failed) << endl;
}

template <size_t MAJ, size_t MIN>
void compare_mat(mat<MAJ, MIN> &dmat,
                 real       emat[MAJ * MIN],
                 const char *msg) {
    auto valid = true;
    
    for (int i = 0; i < MAJ; ++i) {
        for (int j = 0; j < MIN; ++j) {
            auto k = i * MIN + j;
            
            valid = valid && (dmat.m[i][j] == emat[k]);
            
#ifdef DUMP
            if (dmat.m[i][j] != emat[k]) {
                cout << " mat[" << i << "][" << j << "] " << dmat.m[i][j]
                     << " != expected[" << k << "] " << emat[k] << endl;
            }
#endif
        }
    }

    // Overall results
    cout << msg << (valid ? passed : failed) << endl;
}



// -----------------------------------------------------------------------------

int main(void) {

    // -------------------------------------------------------------------------
    // Verify CPU features and identity CPU
    
#if defined(__x86_64__) || defined(_M_X64)  // 64-bit Intel
    char buffer[64];

    // Make sure we have the proper level of CPU functionality (Haswell)
    if (! is_cpu_gen_4()) {
        cout << "CPU is not x86-64 4th gen compatible" << endl;
        exit(1);
    }

    // Identify the CPU
    if (get_cpu_brand(buffer, sizeof(buffer))) {
        cout << (char *) buffer << endl;
    }
#endif

    
    
    // -------------------------------------------------------------------------
    // Matrices and vector arrays

    mat<4, 4> dmat;             // drmat = srmata * srmatb
    mat<4, 4> smata;
    mat<4, 4> smatb;

    vec<4>    *dvecarr;         // drvec[] = srvec[] * srmat
    vec<4>    *svecarr;         // Pre multiplication of a vector

    vec<4>     svec0;           // Used for initialization of svec[]
    vec<4>     svec1;

    // Round up to an even number of array elements
    // so we can process float vectors in pairs
    int elements = 300;
    int rounded  = (elements + 1) & ~1;
    
    // Allocate aligned vector arrays
#if defined(__x86_64__) || defined(_M_X64)      // 64-bit Intel
    dvecarr = (vec<4> *) _mm_malloc(rounded  * sizeof(vec<4>), alignment);
    svecarr = (vec<4> *) _mm_malloc(rounded  * sizeof(vec<4>), alignment);
#else
    dvecarr = (rvec<4> *)std::aligned_alloc(alignment, rounded  * sizeof(vec<4>));
    svecarr = (rvec<4> *)std::aligned_alloc(alignment, rounded  * sizeof(vec<4>));
#endif

    // Make sure allocations were successful
    if (   dvecarr == nullptr
        || svecarr == nullptr) {
        cout << "Failed to allocate memory for vector arrays" << endl;
        exit(1);
    }

    // Zero out the destination matrices and vectors.
    // Note this includes an extra vector allocated to round up.
    memset(&dmat,   0, sizeof(mat<4, 4>));
    memset(dvecarr, 0, rounded  * sizeof(vec<4>));

    
    
    // -------------------------------------------------------------------------
    // Expected results of tests
    
    // 3 digits in array value: mrc.0f
    //                          m = matrix number, a=1, b=2
    //                          r = row number, 1 based
    //                          c = column number, 1 based
    real emat[16] = { 111.0f * 211.0f + 112.0f * 221.0f + 113.0f * 231.0f + 114.0f * 241.0f,
                      111.0f * 212.0f + 112.0f * 222.0f + 113.0f * 232.0f + 114.0f * 242.0f,
                      111.0f * 213.0f + 112.0f * 223.0f + 113.0f * 233.0f + 114.0f * 243.0f,
                      111.0f * 214.0f + 112.0f * 224.0f + 113.0f * 234.0f + 114.0f * 244.0f,
                      121.0f * 211.0f + 122.0f * 221.0f + 123.0f * 231.0f + 124.0f * 241.0f,
                      121.0f * 212.0f + 122.0f * 222.0f + 123.0f * 232.0f + 124.0f * 242.0f,
                      121.0f * 213.0f + 122.0f * 223.0f + 123.0f * 233.0f + 124.0f * 243.0f,
                      121.0f * 214.0f + 122.0f * 224.0f + 123.0f * 234.0f + 124.0f * 244.0f,
                      131.0f * 211.0f + 132.0f * 221.0f + 133.0f * 231.0f + 134.0f * 241.0f,
                      131.0f * 212.0f + 132.0f * 222.0f + 133.0f * 232.0f + 134.0f * 242.0f,
                      131.0f * 213.0f + 132.0f * 223.0f + 133.0f * 233.0f + 134.0f * 243.0f,
                      131.0f * 214.0f + 132.0f * 224.0f + 133.0f * 234.0f + 134.0f * 244.0f,
                      141.0f * 211.0f + 142.0f * 221.0f + 143.0f * 231.0f + 144.0f * 241.0f,
                      141.0f * 212.0f + 142.0f * 222.0f + 143.0f * 232.0f + 144.0f * 242.0f,
                      141.0f * 213.0f + 142.0f * 223.0f + 143.0f * 233.0f + 144.0f * 243.0f,
                      141.0f * 214.0f + 142.0f * 224.0f + 143.0f * 234.0f + 144.0f * 244.0f };
    
    // 1 digit in vector value: v.0f
    //                          v = vector pair element number, 1 based.
    //                              even index vector 1..4, odd index vector 5..8.

    // Even index into vector array
    float evec0[4] = { 1.0f * 111.0f + 2.0f * 121.0f + 3.0f * 131.0f + 4.0f * 141.0f,
                       1.0f * 112.0f + 2.0f * 122.0f + 3.0f * 132.0f + 4.0f * 142.0f,
                       1.0f * 113.0f + 2.0f * 123.0f + 3.0f * 133.0f + 4.0f * 143.0f,
                       1.0f * 114.0f + 2.0f * 124.0f + 3.0f * 134.0f + 4.0f * 144.0f };
 
    // Odd index into vector array
    float evec1[] = { 5.0f * 111.0f + 6.0f * 121.0f + 7.0f * 131.0f + 8.0f * 141.0f,
                      5.0f * 112.0f + 6.0f * 122.0f + 7.0f * 132.0f + 8.0f * 142.0f,
                      5.0f * 113.0f + 6.0f * 123.0f + 7.0f * 133.0f + 8.0f * 143.0f,
                      5.0f * 114.0f + 6.0f * 124.0f + 7.0f * 134.0f + 8.0f * 144.0f };

    
    
    // -------------------------------------------------------------------------
    // Read the test parameters.
    // We don't want to define these values in the code to avoid
    // having a good optimizer introduce some compile time calculations.

    ifstream file;
    int      iterations = 1'000'000'000;
    int      width      = 8;
    
    float  tmat[16];
    float  tvec[4];

    file.open("params.txt");
    if (file.rdstate() & fstream::failbit) {
        cout << "Could not open params.txt" << endl;
        exit(1);
    }
    
    // Timing iterations
    
    file >> iterations;
    if (file.eof()) {
        cout << "Could not read iterations" << endl;
        exit(1);
    }
    if (iterations == 0) {
        cout << "Bad iterations" << endl;
        exit(1);
    }

    // Number of array vector elements

    file >> elements;
    if (file.eof()) {
        cout << "Could not read elements" << endl;
        exit(1);
    }
    if (elements == 0) {
        cout << "Bad elements" << endl;
        exit(1);
    }

    // 4x4 matrix with element values representing row column position.
    // 2 digits in array value: rc.0f
    //                          r = row number, 1 based
    //                          c = column number, 1 based

    for (int i = 0; i < 16; ++i) {
        int val;
        
        file >> val;
        if (file.eof()) {
            cout << "Could not read matrix parameter" << endl;
            exit(1);
        }
        if (val == 0) {
            cout << "Bad matrix parameter" << endl;
            exit(1);
        }
        
        tmat[i] = real(val);
    }
    
    // 1x4 vector with element values representing column position, 1 based
    
    for (int i = 0; i < 4; ++i) {
        int val;
        
        file >> val;
        if (file.eof()) {
            cout << "Could not read vector parameter" << endl;
            exit(1);
        }
        if (val == 0) {
            cout << "Bad vector parameter" << endl;
            exit(1);
        }

        tvec[i] = real(val);
    }
    
    file.close();

    
    
    // -------------------------------------------------------------------------
    // Initialize the source matrices and vectors

    int scalar1 = 100;
    int scalar2 = 200;
    int scalar4 = 4;

    // Matrix values encode row and column number digits for debugging.
    smata.set(tmat);
    smatb.set(tmat);
    
    // Add matrix identification digit
    add_scalar(smata, float(scalar1));
    add_scalar(smatb, float(scalar2));

    // Vector values encode index digit for debugging
    svec0.set(tvec);
    svec1.set(tvec);
    
    // Offset the odd vector indices
    add_scalar(svec1, float(scalar4));

    for (int i = 0; i < elements; ++i) {
        // SIMD may operate on two vectors at a time
        // so lets make sure expected results are
        // different for each vector during testing.
        
        // Odd numbered index into array
        if (i & 1) {
            svecarr[i] = *((vec<4> *) (&svec1));
        }
        // Even numbered index into array
        else {
            svecarr[i] = *((vec<4> *) (&svec0));
        }
    }

    
    
    // -------------------------------------------------------------------------
    // Test the multiplcations
    
    mat_x_mat(dmat, smata, smatb);
    mat_x_mat(dmat, smata, smatb);
    
    vecarr_x_mat(dvecarr, svecarr, smata, elements);
    vecarr_x_mat(dvecarr, svecarr, smata, elements);

    // Compare actual and expected results
    compare_mat<4, 4> (dmat,    emat,                   "mat   4x4 * mat 4x4 test ");
    compare_vec<4>    (dvecarr, evec0, evec1, elements, "vec[] 1x4 * mat 4x4 test ");

    
    
    // -------------------------------------------------------------------------
    // Additional tests

    // Source and expected deta
    real tmat32[]  = {  1,  2,  3,  4,  5,  6 };
    real tmat23[]  = {  7,  8,  9, 10, 11, 12 };
    real tmat33a[] = {  1,  2,  3,  4,  5,  6,  7,  8,  9 };
    real tmat33b[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    real emat33[]  = { 27, 30, 33, 61, 68, 75, 95, 106, 117 };       // TI-84+
    real emat22[]  = { 76, 100, 103, 136 };                          // TI-84+
    real emat33c[] = { 84, 90, 96, 201, 216, 231, 318, 342, 366 };   // TI-84+

    // Source and destination matrices
    mat<3, 2> smat32;
    mat<2, 3> smat23;
    mat<3, 3> smat33a;
    mat<3, 3> smat33b;
    mat<3, 3> dmat33;
    mat<2, 2> dmat22;
    mat<3, 3> dmat33c;

    // Initialize source matrices with test data.
    smat32.set  (tmat32);
    smat23.set  (tmat23);
    smat33a.set (tmat33a);
    smat33b.set (tmat33b);

    // Zero destination matrices
    memset(&dmat33,  0, sizeof(dmat33));
    memset(&dmat22,  0, sizeof(dmat22));
    memset(&dmat33c, 0, sizeof(dmat33c));

    // Perform the row and column major order multiplications
    mat_x_mat(dmat33,  smat32,  smat23);
    mat_x_mat(dmat22,  smat23,  smat32);
    mat_x_mat(dmat33c, smat33a, smat33b);

    // Compare actual and expected results
    compare_mat<3, 3>(dmat33,  emat33,  "mat   3x2 * mat 2x3 test ");
    compare_mat<2, 2>(dmat22,  emat22,  "mat   2x3 * mat 3x2 test ");
    compare_mat<3, 3>(dmat33c, emat33c, "mat   3x3 * mat 3x3 test ");

    
    
    // -------------------------------------------------------------------------
    // Time the multiplcations

    // Use volatile to prevent optimizer from removing code
    volatile int no_opt_i;

    auto specf = other;
    timer<int, std::milli> timer;
    for (no_opt_i = 0; no_opt_i < int(iterations); no_opt_i = no_opt_i + 1) {
        specf = mat_x_mat(dmat, smata, smatb);
    }
    auto milli = timer.elapsed();

    cout.imbue(std::locale(""));
    cout << "iterations          " << iterations   << endl
         << "vec array elements  " << elements     << endl
         << "                float"                << endl
         << "mata x matb " << setw(width) << milli << " ms "
                           << get_string(specf)    << endl;

    // Scale the number of loops for array operations
    auto n = int(iterations / elements);

    specf = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < n; no_opt_i = no_opt_i + 1) {
        specf = vecarr_x_mat(dvecarr, svecarr, smata, elements);
    }
    milli = timer.elapsed();

    cout << "vec[] x mat " << setw(width) << milli << " ms "
                           << get_string(specf)    << endl;

    
    
    // -------------------------------------------------------------------------
    // Free the vector arrays
    
#if defined(__x86_64__) || defined(_M_X64)      // 64-bit Intel
    _mm_free(dvecarr);
    _mm_free(svecarr);
#else
    std::free(dvecarr);
    std::free(svecarr);
#endif

}
