// matrix3d.h

#ifndef matrix3d_h
#define matrix3d_h

#include <cstddef>
#include <cstring>

namespace matrix3d {



// -----------------------------------------------------------------------------
// Alias for floating point type to use
typedef float real;



// -----------------------------------------------------------------------------
// Align for 256-bit register
const int alignment = 256 / 8;



// -----------------------------------------------------------------------------
// Enumeration identifying specializations

enum specialized {
    primary,    // Primary C++ implementation using loops
    unroll,     // Specialized implmentation with unrolled loops
    intel,      // Specialized implmentation with Intel SIMD Intrinsics
    intel256,   //   Pairs of floats in 256-bit registers
    arm,        // Specialized implmentation with ARM SIMD Intrinsics
    avx,        // Specialized implmentation with Intel AVX2 Assembly language
    avx256,     //   Pairs of floats in 256-bit registers
    avx512,     //   Pairs of doubles and quads of floats in 512-bit registers
    neon,       // Specialized implmentation with ARM NEON Assembly language
    other       // Something is wrong if this is reported
};

inline const char *get_string(specialized spec) {
    switch (spec) {
        case  primary  : return "primary ";
        case  unroll   : return "unroll  ";
        case  intel    : return "intel   ";
        case  intel256 : return "intel256";
        case  arm      : return "arm     ";
        case  avx      : return "avx     ";
        case  avx256   : return "avx256  ";
        case  avx512   : return "avx512  ";
        case  neon     : return "neon    ";
        default        : return "other   ";
    }
}



// -----------------------------------------------------------------------------
// Vector structs

template <size_t N> struct vec {
    // Align for 4 element vector
    alignas(sizeof(real) * 4) real v[N];

    // Note that template will only match arrays of size N,
    // the compiler is verifying the array size for us
    void get(      real(&dest) [N]) { std::memcpy(dest, v,   sizeof(v)); }
    void set(const real(&src)  [N]) { std::memcpy(v,    src, sizeof(v)); }

    // Verify that the index is in range
    bool validate(size_t i) { return i < N; }
};



// -----------------------------------------------------------------------------
// Matrix structs

template <size_t MAJ, size_t MIN> struct mat {
    alignas(alignment) real m[MAJ][MIN];
    
    // Note that template will only match arrays of size MAJ * MIN,
    // the compiler is verifying the array size for us
    void get(      real(&dest) [MAJ * MIN]) { std::memcpy(dest, m,   sizeof(m)); }
    void set(const real(&src)  [MAJ * MIN]) { std::memcpy(m,    src, sizeof(m)); }

    // Verify that row and column are in range
    bool validate(size_t i, size_t j) { return i < MAJ && j < MIN; }
    bool validate(size_t i)           { return i < MAJ;            }
};



// -----------------------------------------------------------------------------
// Set elements of matrix and vector

// Use a lambda function to iterate over vector and matrix
template <size_t N, typename F>
inline void set_vector(vec<N> &dest, F func) {
    for (int i = 0; i < N; ++i)
        dest.v[i] = func(i);
}

template <size_t MAJ, size_t MIN, typename F>
inline void set_matrix(mat<MAJ, MIN> &dest, F func) {
    for (int i = 0; i < MAJ; ++i)
        for (int j = 0; j < MIN; ++j)
            dest.m[i][j] = func(i, j);
}

template <size_t MAJ, size_t MIN>
inline void diagonal(mat<MAJ, MIN> &dest, real val) {
    set_matrix(dest, [val] (int i, int j)
                     -> real { return (i == j) ? val : real(0); });
}

template <size_t N>
inline void copy(mat<N, N> &dest, mat<N, N> &src) {
    set_matrix(dest, [&src] (int i, int j)
                     -> real { return src.m[i][j]; });
}

template <size_t N>
inline void add_scalar(vec<N> &dest, real val) {
    set_vector(dest, [&dest, val] (int i)
                     -> real { return dest.v[i] + val; });
}

template <size_t MAJ, size_t MIN>
inline void add_scalar(mat<MAJ, MIN> &dest, real val) {
    set_matrix(dest, [&dest, val] (int i, int j)
                     -> real { return dest.m[i][j] + val; });
}



// -----------------------------------------------------------------------------
// Matrix multiplication

template <size_t MAJ, size_t MIN, size_t K>
inline specialized mat_x_mat(mat<MAJ, MIN> &dest,
                             mat<MAJ, K>   &a,
                             mat<K,   MIN> &b) {
    for (int i = 0; i < MAJ; ++i) {
        for (int j = 0; j < MIN; ++j) {
            auto sum = real(0);

            for (int k = 0; k < K; ++k) {
                sum += a.m[i][k] * b.m[k][j];
            }
            
            dest.m[i][j] = sum;
        }
    }
    
    return primary;
}



// -----------------------------------------------------------------------------
// Matrix and vector multiplication

template <size_t MAJ, size_t MIN>
inline specialized vec_x_mat(vec <MIN>      &dest,
                             vec <MAJ>      &v,
                             mat <MAJ, MIN> &m) {
    for (int j = 0; j < MIN; ++j) {
        auto sum = real(0);
        
        for (int i = 0; i < MAJ; ++i) {
            sum += v.v[i] * m.m[i][j];
        }
        dest.v[j] = sum;
    }
    
    return primary;
}



// -----------------------------------------------------------------------------
// Matrix and vector array multiplication

template <size_t MAJ, size_t MIN>
inline specialized vecarr_x_mat(vec <MAJ>      *dest,
                                vec <MAJ>      *v,
                                mat <MAJ, MIN> &m,
                                size_t         n) {
    for (int e = 0; e < n; ++e) {
        for (int j = 0; j < MIN; ++j) {
            auto sum = real(0);
            
            for (int i = 0; i < MAJ; ++i) {
                sum += v[e].v[i] * m.m[i][j];
            }
            dest[e].v[j] = sum;
        }
    }
    
    return primary;
}



}   // namespace matrix3d

#endif  // matrix3d_h
