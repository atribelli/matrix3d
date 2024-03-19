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

template <typename T, size_t N>
void compare_vec(vecarr<T, N> &dvecarr,
                 T            evec0[N],
                 T            evec1[N],
                 int          elements,
                 const char   *msg) {
    T    *pv   = dvecarr.array()->v;
    auto valid = true;
    
    for (int e = 0; e < elements; ++e) {
        for (int i = 0; i < N; ++i) {
            // Even and odd numbered vectors have different expected values
            // since we may process two vectors at a time in SIMD code
            auto expected = (e & 1) ? evec1[i] : evec0[i];
            
            valid = valid && (pv[e * N + i] == expected);
            
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

template <typename T, size_t MAJ, size_t MIN>
void compare_mat(mat<T, MAJ, MIN> &dmat,
                 T                emat[MAJ * MIN],
                 const char       *msg) {
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

    rmat<float,  4, 4> drmatf;          // drmat = srmata * srmatb
    rmat<float,  4, 4> srmataf;
    rmat<float,  4, 4> srmatbf;
    rmat<double, 4, 4> drmatd;
    rmat<double, 4, 4> srmatad;
    rmat<double, 4, 4> srmatbd;

    cmat<float,  4, 4> dcmatf;          // dcmat = scmatb * scmata
    cmat<float,  4, 4> scmataf;
    cmat<float,  4, 4> scmatbf;
    cmat<double, 4, 4> dcmatd;
    cmat<double, 4, 4> scmatad;
    cmat<double, 4, 4> scmatbd;

    rvecarr<float,  4> drvecarrf;      // drvec[] = srvec[] * srmat
    rvecarr<float,  4> srvecarrf;      // Pre multiplication of a vector
    rvecarr<double, 4> drvecarrd;
    rvecarr<double, 4> srvecarrd;

    cvecarr<float,  4> dcvecarrf;      // dcvec[] = scmat * scvec[]
    cvecarr<float,  4> scvecarrf;      // Post multiplication of a vector
    cvecarr<double, 4> dcvecarrd;
    cvecarr<double, 4> scvecarrd;

    vec<float,  4>     svec0f;          // Used for initialization of svec[]
    vec<float,  4>     svec1f;
    vec<double, 4>     svec0d;
    vec<double, 4>     svec1d;

    // Allocate aligned vector arrays
    int elements = 300;
    
    drvecarrf.alloc(elements);
    srvecarrf.alloc(elements);
    drvecarrd.alloc(elements);
    srvecarrd.alloc(elements);
    
    dcvecarrf.alloc(elements);
    scvecarrf.alloc(elements);
    dcvecarrd.alloc(elements);
    scvecarrd.alloc(elements);

    // Make sure allocations were successful
    if (   drvecarrf.array() == nullptr
        || srvecarrf.array() == nullptr
        || drvecarrd.array() == nullptr
        || srvecarrd.array() == nullptr
        || dcvecarrf.array() == nullptr
        || scvecarrf.array() == nullptr
        || dcvecarrd.array() == nullptr
        || scvecarrd.array() == nullptr) {
        cout << "Failed to allocate memory for vector arrays" << endl;
        exit(1);
    }

    // Zero out the destination matrices and vectors.
    // Note this includes an extra vector allocarted to round up.
    memset(&drmatf,   0, sizeof(rmat<float,  4, 4>));
    memset(&drmatd,   0, sizeof(rmat<double, 4, 4>));
    memset(&dcmatf,   0, sizeof(cmat<float,  4, 4>));
    memset(&dcmatd,   0, sizeof(cmat<double, 4, 4>));
    
    memset(drvecarrf.array(), 0, drvecarrf.count() * sizeof(rvec<float,  4>));
    memset(drvecarrd.array(), 0, drvecarrd.count() * sizeof(rvec<double, 4>));
    memset(dcvecarrf.array(), 0, dcvecarrf.count() * sizeof(cvec<float,  4>));
    memset(dcvecarrd.array(), 0, dcvecarrd.count() * sizeof(cvec<double, 4>));

    
    
    // -------------------------------------------------------------------------
    // Expected results of tests

    // dmat = smata * smatb
    //
    // Row major order
    // dest(MAJ,MIN)   = a(MAJ,K)    * b(K,MIN)
    // [ a' b' c' d'   = [ a b c d   * [ A B C D
    //   e' f' g' h'       e f g h       E F G H
    //   i' j' k' l'       i j k l       I J K L
    //   m' n' o' p' ]     m n o p ]     M N O P ]
    //                 = [ aA + bE + cI + dM, aB + bF + cJ + dN, aC + bG + cK + dO, aD + bH + cL + dP,
    //                     eA + fE + gI + hM, eB + fF + gJ + hN, eC + fG + gK + hO, eD + fH + gL + hP,
    //                     iA + jE + kI + lM, iB + jF + kJ + lN, iC + jG + kK + lO, iD + jH + kL + lP,
    //                     mA + nE + oI + pM, mB + nF + oJ + pN, mC + nG + oK + pO, mD + nH + oL + pP ]
    // Linear array = [ aA+bE+cI+dM, aB+bF+cJ+dN, aC+bG+cK+dO, aD+bH+cL+dP,
    //                  eA+fE+gI+hM, eB+fF+gJ+hN, eC+fG+gK+hO, eD+fH+gL+hP,
    //                  iA+jE+kI+lM, iB+jF+kJ+lN, iC+jG+kK+lO, iD+jH+kL+lP,
    //                  mA+nE+oI+pM, mB+nF+oJ+pN, mC+nG+oK+pO, mD+nH+oL+pP ]
    //
    // Column major order
    // T(dest(MAJ,MIN)) = T(b(K,MIN)) * T(a(MAJ,K))
    // [ a' e' i' m'    = [ A E I M   * [ a e i m
    //   b' f' j' n'        B F J N       b f j n
    //   c' g' k' o'        C G K O       c g k o
    //   d' h' l' p' ]      D H L P ]     d h l p ]
    //                  = [ Aa + Eb + Ic + Md, Ae + Ef + Ig + Mh, Ai + Ej + Ik + Ml, Am + En + Io + Mp
    //                      Ba + Fb + Jc + Nd, Be + Ff + Jg + Nh, Bi + Fj + Jk + Nl, Bm + Fn + Jo + Np
    //                      Ca + Gb + Kc + Od, Ce + Gf + Kg + Oh, Ci + Gj + Kk + Ol, Cm + Gn + Ko + Op
    //                      Da + Hb + Lc + Pd, De + Hf + Lg + Ph, Di + Hj + Lk + Pl, Dm + Hn + Lo + Pp ]
    // Linear array = [ Aa+Eb+Ic+Md, Ba+Fb+Jc+Nd, Ca+Gb+Kc+Od, Da+Hb+Lc+Pd,
    //                  Ae+Ef+Ig+Mh, Be+Ff+Jg+Nh, Ce+Gf+Kg+Oh, De+Hf+Lg+Ph,
    //                  Ai+Ej+Ik+Ml, Bi+Fj+Jk+Nl, Ci+Gj+Kk+Ol, Di+Hj+Lk+Pl,
    //                  Am+En+Io+Mp, Bm+Fn+Jo+Np, Cm+Gn+Ko+Op, Dm+Hn+Lo+Pp ]
    //
    // Note the linear arrays are the same
    //
    // 3 digits in array value: mrc.0f
    //                          m = matrix number, a=1, b=2
    //                          r = row number, 1 based
    //                          c = column number, 1 based
    float ematf[16] = { 111.0f * 211.0f + 112.0f * 221.0f + 113.0f * 231.0f + 114.0f * 241.0f,
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
    double ematd[16];
    
    // Row major order
    // Pre multiplication of a vector
    // dest(1,MIN)     = v(1,MAJ)    * m(MAJ,MIN)
    // [ x' y' z' w' ] = [ x y z w ] * [ a b c d
    //                                   e f g h
    //                                   i j k l
    //                                   m n o p ]
    //                 = [ xa + ye + zi + wm
    //                     xb + yf + zj + wn
    //                     xc + yg + zk + wo
    //                     xd + yh + zl + wp ]
    // Linear array: [ xa+ye+zi+wm, xb+yf+zj+wn, xc+yg+zk+wo, xd+yh+zl+wp ]
    //
    // Column major order
    // Post multiplication of a vector
    // T(dest(1,MIN)) = T(m(MAJ,MIN)) * T(v(1,MAJ))
    // [ x'           = [ a e i m     * [ x
    //   y'               b f j n         y
    //   z'               c g k o         z
    //   w' ]             d h l p ]       w ]
    //                = [ ax + ey + iz + mw
    //                    bx + fy + jz + nw
    //                    cx + gy + kz + ow
    //                    dx + hy + lz + pw ]
    // Linear array: [ ax+ey+iz+mw, bx+fy+jz+nw, cx+gy+kz+ow, dx+hy+lz+pw ]
    //
    // Note the linear arrays are the same
    //
    // 1 digit in vector value: v.0f
    //                          v = vector pair element number, 1 based.
    //                              even index vector 1..4, odd index vector 5..8.

    // Even index into vector array
    float evec0f[4] = { 1.0f * 111.0f + 2.0f * 121.0f + 3.0f * 131.0f + 4.0f * 141.0f,
                        1.0f * 112.0f + 2.0f * 122.0f + 3.0f * 132.0f + 4.0f * 142.0f,
                        1.0f * 113.0f + 2.0f * 123.0f + 3.0f * 133.0f + 4.0f * 143.0f,
                        1.0f * 114.0f + 2.0f * 124.0f + 3.0f * 134.0f + 4.0f * 144.0f };
    double evec0d[4];

    // Odd index into vector array
    float evec1f[] = { 5.0f * 111.0f + 6.0f * 121.0f + 7.0f * 131.0f + 8.0f * 141.0f,
                       5.0f * 112.0f + 6.0f * 122.0f + 7.0f * 132.0f + 8.0f * 142.0f,
                       5.0f * 113.0f + 6.0f * 123.0f + 7.0f * 133.0f + 8.0f * 143.0f,
                       5.0f * 114.0f + 6.0f * 124.0f + 7.0f * 134.0f + 8.0f * 144.0f };
    double evec1d[4];
    
    // Create expected double data from expected float data
    for (int i = 0; i < 16; ++i) {
        ematd[i] = double(ematf[i]);
    }
    for (int i = 0; i < 4; ++i) {
        evec0d[i] = double(evec0f[i]);
        evec1d[i] = double(evec1f[i]);
    }

    
    
    // -------------------------------------------------------------------------
    // Read the test parameters.
    // We don't want to define these values in the code to avoid
    // having a good optimizer introduce some compile time calculations.

    ifstream file;
    int      iterations = 1'000'000'000;
    int      width      = 8;
    
    float  tmatf[16];
    double tmatd[16];
    float  tvecf[4];
    double tvecd[4];

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
        
        tmatf[i] = float  (val);
        tmatd[i] = double (val);
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

        tvecf[i] = float  (val);
        tvecd[i] = double (val);
    }
    
    file.close();

    
    
    // -------------------------------------------------------------------------
    // Initialize the source matrices and vectors

    int scalar1 = 100;
    int scalar2 = 200;
    int scalar4 = 4;

    // Row major order.
    // Matrix values encode row and column number digits for debugging.
    srmataf.set(tmatf);
    srmatbf.set(tmatf);
    srmatad.set(tmatd);
    srmatbd.set(tmatd);
    
    // Add matrix identification digit
    add_scalar(srmataf, float(scalar1));
    add_scalar(srmatbf, float(scalar2));
    add_scalar(srmatad, double(scalar1));
    add_scalar(srmatbd, double(scalar2));
    
    // Column major order, same memory layout
    copy(scmataf, srmataf);
    copy(scmatbf, srmatbf);
    copy(scmatad, srmatad);
    copy(scmatbd, srmatbd);

    // Vector values encode index digit for debugging
    svec0f.set(tvecf);
    svec0d.set(tvecd);
    svec1f.set(tvecf);
    svec1d.set(tvecd);
    
    // Offset the odd vector indices
    add_scalar(svec1f, float(scalar4));
    add_scalar(svec1d, double(scalar4));

    for (int i = 0; i < elements; ++i) {
        // SIMD may operate on two vectors at a time
        // so lets make sure expected results are
        // different for each vector during testing.
        
        // Odd numbered index into array
        if (i & 1) {
            srvecarrf.array()[i] = *((rvec<float,  4> *) (&svec1f));
            srvecarrd.array()[i] = *((rvec<double, 4> *) (&svec1d));
            scvecarrf.array()[i] = *((cvec<float,  4> *) (&svec1f));    // Same memory
            scvecarrd.array()[i] = *((cvec<double, 4> *) (&svec1d));    //   layout
        }
        // Even numbered index into array
        else {
            srvecarrf.array()[i] = *((rvec<float,  4> *) (&svec0f));
            srvecarrd.array()[i] = *((rvec<double, 4> *) (&svec0d));
            scvecarrf.array()[i] = *((cvec<float,  4> *) (&svec0f));
            scvecarrd.array()[i] = *((cvec<double, 4> *) (&svec0d));
        }
    }

    
    
    // -------------------------------------------------------------------------
    // Test the multiplcations
    
    rmata_x_rmatb(drmatf, srmataf, srmatbf);
    rmata_x_rmatb(drmatd, srmatad, srmatbd);
    cmatb_x_cmata(dcmatf, scmatbf, scmataf);
    cmatb_x_cmata(dcmatd, scmatbd, scmatad);
    
    rvecarr_x_rmat(drvecarrf, srvecarrf, srmataf,   elements);
    rvecarr_x_rmat(drvecarrd, srvecarrd, srmatad,   elements);
    cmat_x_cvecarr(dcvecarrf, scmataf,   scvecarrf, elements);
    cmat_x_cvecarr(dcvecarrd, scmatad,   scvecarrd, elements);

    // Compare actual and expected results
    compare_mat<float,  4, 4> (drmatf,    ematf,
                               "mata  4x4 * matb  4x4 float  test ");
    compare_mat<double, 4, 4> (drmatd,    ematd,
                               "mata  4x4 * matb  4x4 double test ");
    compare_mat<float,  4, 4> (dcmatf,    ematf,
                               "matb  4x4 * mata  4x4 float  test ");
    compare_mat<double, 4, 4> (dcmatd,    ematd,
                               "matb  4x4 * mata  4x4 double test ");
    compare_vec<float,  4>    (drvecarrf, evec0f, evec1f, elements,
                               "vec[] 1x4 * mat   4x4 float  test ");
    compare_vec<double, 4>    (drvecarrd, evec0d, evec1d, elements,
                               "vec[] 1x4 * mat   4x4 double test ");
    compare_vec<float,  4>    (dcvecarrf, evec0f, evec1f, elements,
                               "mat   4x4 * vec[] 4x1 float  test ");
    compare_vec<double, 4>    (dcvecarrd, evec0d, evec1d, elements,
                               "mat   4x4 * vec[] 4x1 double test ");

    
    
    // -------------------------------------------------------------------------
    // Additional tests

    // Source and expected deta
    int tmat32[]  = {  1,  2,  3,  4,  5,  6 };
    int tmat23[]  = {  7,  8,  9, 10, 11, 12 };
    int tmat33a[] = {  1,  2,  3,  4,  5,  6,  7,  8,  9 };
    int tmat33b[] = { 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    int emat33[]  = { 27, 30, 33, 61, 68, 75, 95, 106, 117 };       // TI-84+
    int emat22[]  = { 76, 100, 103, 136 };                          // TI-84+
    int emat33c[] = { 84, 90, 96, 201, 216, 231, 318, 342, 366 };   // TI-84+

    // Source and destination matrices
    rmat<int, 3, 2> srmat32;
    rmat<int, 2, 3> srmat23;
    rmat<int, 3, 3> srmat33a;
    rmat<int, 3, 3> srmat33b;
    cmat<int, 3, 2> scmat32;
    cmat<int, 2, 3> scmat23;
    cmat<int, 3, 3> scmat33a;
    cmat<int, 3, 3> scmat33b;
    rmat<int, 3, 3> drmat33;
    rmat<int, 2, 2> drmat22;
    rmat<int, 3, 3> drmat33c;
    cmat<int, 3, 3> dcmat33;
    cmat<int, 2, 2> dcmat22;
    cmat<int, 3, 3> dcmat33c;

    // Initialize source matrices with test data.
    // Yes, data is supposed to be the same for
    // row and column major order.
    // Only the interpretation of the data differs.
    // Whether consecutive values are interpreted
    // as rows or interpreted as columns.
    //
    // Test linear arrays:  [ a b c d e f ]
    //                      [ g h i j k l ]
    //
    // Row major order 3x2: [ [ a b ]
    //                        [ c d ]
    //                        [ e f ] ]
    //                 2x3: [ [ a b c ]
    //                        [ d e f ] ]
    //
    // Col major order 3x2:  [ [ a   [ c   [ e
    //                           b ]   d ]   f ] ]
    //                 2x3:  [ [ a   [ d
    //                           b     e
    //                           c ]   f ] ]
    //
    // Row major order mul:   C = A * B
    //                        [ a b   * [ g h i
    //                          c d       j k l ]
    //                          e f ]
    //                      [ ag+bj ah+bk ai+bl
    //                        cg+dj ch+dk ci+dl
    //                        eg+fj eh+fk ei+fl ]
    //
    // Expected inear array: [ ag+bj ah+bk ai+bl cg+dj ch+dk ci+dl eg+fj eh+fk ei+fl ]
    //
    // Col major order mul: T(C) = T(B) * (T(A)
    //                      [ g j   * [ a c e
    //                        h k       b d f ]
    //                        i l ]
    //                      [ ga+jb gc+jd ge+jf
    //                        ha+kb hc+kd he+kf
    //                        ia+lb ic+ld ie+lf ]
    //
    // Expected linear array: [ ga+jb ha+kb ia+lb gc+jd hc+kd ic+ld ge+jf he+kf ie+lf ]
    //
    // Note the linear arrays match
    
    // Initialize source matrices
    srmat32.set  (tmat32);
    srmat23.set  (tmat23);
    srmat33a.set (tmat33a);
    srmat33b.set (tmat33b);
    scmat32.set  (tmat32);
    scmat23.set  (tmat23);
    scmat33a.set (tmat33a);
    scmat33b.set (tmat33b);

    // Zero destination matrices
    memset(&drmat33,  0, sizeof(drmat33));
    memset(&drmat22,  0, sizeof(drmat22));
    memset(&drmat33c, 0, sizeof(drmat33c));
    memset(&dcmat33,  0, sizeof(dcmat33));
    memset(&dcmat22,  0, sizeof(dcmat22));
    memset(&dcmat33c, 0, sizeof(dcmat33c));

    // Perform the row and column major order multiplications
    rmata_x_rmatb(drmat33,  srmat32,  srmat23);
    rmata_x_rmatb(drmat22,  srmat23,  srmat32);
    rmata_x_rmatb(drmat33c, srmat33a, srmat33b);
    cmatb_x_cmata(dcmat33,  scmat23,  scmat32);
    cmatb_x_cmata(dcmat22,  scmat32,  scmat23);
    cmatb_x_cmata(dcmat33c, scmat33b, scmat33a);

    // Compare actual and expected results
    compare_mat<int, 3, 3>(drmat33,  emat33,  "mata  3x2 * matb  2x3 int    test ");
    compare_mat<int, 2, 2>(drmat22,  emat22,  "mata  2x3 * matb  3x2 int    test ");
    compare_mat<int, 3, 3>(drmat33c, emat33c, "mata  3x3 * matb  3x3 int    test ");
    compare_mat<int, 3, 3>(dcmat33,  emat33,  "matb  2x3 * mata  3x2 int    test ");
    compare_mat<int, 2, 2>(dcmat22,  emat22,  "matb  3x2 * mata  2x3 int    test ");
    compare_mat<int, 3, 3>(dcmat33c, emat33c, "matb  3x3 * mata  3x3 int    test ");

    
    
    // -------------------------------------------------------------------------
    // Time the multiplcations

    // Use volatile to prevent optimizer from removing code
    volatile int no_opt_i;

    auto specf = other;
    timer<int, std::milli> timer;
    for (no_opt_i = 0; no_opt_i < int(iterations); no_opt_i = no_opt_i + 1) {
        specf = rmata_x_rmatb(drmatf, srmataf, srmatbf);
    }
    auto millif = timer.elapsed();

    auto specd = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < int(iterations); no_opt_i = no_opt_i + 1) {
        specd = rmata_x_rmatb(drmatd, srmatad, srmatbd);
    }
    auto millid = timer.elapsed();

    cout.imbue(std::locale(""));
    cout << "iterations          " << iterations         << endl
         << "vec array elements  " << elements           << endl
         << "                float               double" << endl
         << "mata x matb " << setw(width) << millif      << " ms "
                           << get_string(specf)          << " "
                           << setw(width) << millid      << " ms "
                           << get_string(specd)          << endl;

    specf = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < int(iterations); no_opt_i = no_opt_i + 1) {
        specf = cmatb_x_cmata(dcmatf, scmatbf, scmataf);
    }
    millif = timer.elapsed();

    specd = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < int(iterations); no_opt_i = no_opt_i + 1) {
        specd = cmatb_x_cmata(dcmatd, scmatbd, scmatad);
    }
    millid = timer.elapsed();

    cout << "matb x mata " << setw(width) << millif << " ms "
                           << get_string(specf)     << " "
                           << setw(width) << millid << " ms "
                           << get_string(specd)     << endl;

    // Scale the number of loops for array operations
    auto n = int(iterations / elements);

    specf = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < n; no_opt_i = no_opt_i + 1) {
        specf = rvecarr_x_rmat(drvecarrf, srvecarrf, srmataf, elements);
    }
    millif = timer.elapsed();

    specd = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < n; no_opt_i = no_opt_i + 1) {
        specd = rvecarr_x_rmat(drvecarrd, srvecarrd, srmatad, elements);
    }
    millid = timer.elapsed();

    cout << "vec[] x mat " << setw(width) << millif << " ms "
                           << get_string(specf)     << " "
                           << setw(width) << millid << " ms "
                           << get_string(specd)     << endl;

    specf = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < n; no_opt_i = no_opt_i + 1) {
        specf = cmat_x_cvecarr(dcvecarrf, scmataf, scvecarrf, elements);
    }
    millif = timer.elapsed();

    specd = other;
    timer.start();
    for (no_opt_i = 0; no_opt_i < n; no_opt_i = no_opt_i + 1) {
        specd = cmat_x_cvecarr(dcvecarrd, scmatad, scvecarrd, elements);
    }
    millid = timer.elapsed();

    cout << "mat x vec[] " << setw(width) << millif << " ms "
                           << get_string(specf)     << " "
                           << setw(width) << millid << " ms "
                           << get_string(specd)     << endl;

    
    
    // -------------------------------------------------------------------------
    // Free the vector arrays
    
    drvecarrf.free();
    srvecarrf.free();
    drvecarrd.free();
    srvecarrd.free();
    dcvecarrf.free();
    scvecarrf.free();
    dcvecarrd.free();
    scvecarrd.free();

}
