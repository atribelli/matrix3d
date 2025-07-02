// matrix3d.h

#ifndef matrix3d_h
#define matrix3d_h

#include <cstddef>
#include <cstring>

namespace matrix3d {



// -----------------------------------------------------------------------------
// Align for 256-bit register
const int alignment = 256 / 8;



// -----------------------------------------------------------------------------
// Enumeration identifying specializations

enum specialized {
    loops,      // Primary C++ implementation using loops
    unroll,     // Specialized implmentation with unrolled loops
    intrin,     // Specialized implmentation with SIMD Intrinsics
    intrin256,  //   Pairs of floats in 256-bit registers
    avx,        // Specialized implmentation with Intel AVX2 assembly language
    avx256,     //   Pairs of floats in 256-bit registers
    avx512,     //   Pairs of doubles and quads of floats in 512-bit registers
    neon,       // Specialized implmentation with ARM NEON assembly language
    sve,        // Specialized implmentation with ARM SVE2 assembly language
    sme,        // Specialized implmentation with ARM SME assembly language
    zero,       // Desired code not implemented, zero'd data instead
    other       // Something is wrong if this is reported
};

inline const char *get_string(specialized spec) {
    switch (spec) {
        case  loops     : return "loops    ";
        case  unroll    : return "unroll   ";
        case  intrin    : return "intrin   ";
        case  intrin256 : return "intrin256";
        case  avx       : return "avx      ";
        case  avx256    : return "avx256   ";
        case  avx512    : return "avx512   ";
        case  neon      : return "neon     ";
        case  sve       : return "sve      ";
        case  sme       : return "sme      ";
        case  zero      : return "zero     ";
        default         : return "other    ";
    }
}



// -----------------------------------------------------------------------------
// Vector structs

template <typename T, size_t N> struct vec {
    // Align for 4 element vector
    alignas(sizeof(T) * 4) T v[N];

    // Note that template will only match arrays of size N,
    // the compiler is verifying the array size for us
    void get(      T(&dest) [N]) { std::memcpy(dest, v,   sizeof(v)); }
    void set(const T(&src)  [N]) { std::memcpy(v,    src, sizeof(v)); }

    // Verify that the index is in range
    bool validate(size_t i) { return i < N; }
};

// Aliases for row and column major versions of a vector,
// to get compiler to enforce pre and post multiplication
template <typename T, size_t N> struct rvec : vec<T, N>{};
template <typename T, size_t N> struct cvec : vec<T, N>{};



// -----------------------------------------------------------------------------
// Matrix structs

template <typename T, size_t MAJ, size_t MIN> struct mat {
    alignas(alignment) T m[MAJ][MIN];
    
    // Note that template will only match arrays of size MAJ * MIN,
    // the compiler is verifying the array size for us
    void get(      T(&dest) [MAJ * MIN]) { std::memcpy(dest, m,   sizeof(m)); }
    void set(const T(&src)  [MAJ * MIN]) { std::memcpy(m,    src, sizeof(m)); }

    // Verify that row and column are in range
    bool validate(size_t i, size_t j) { return i < MAJ && j < MIN; }
    bool validate(size_t i)           { return i < MAJ;            }
};

// Aliases for row and column major versions of a matrix,
// to get compiler to enforce pre and post multiplication
template <typename T, size_t MAJ, size_t MIN> struct rmat : mat<T, MAJ, MIN>{};
template <typename T, size_t MAJ, size_t MIN> struct cmat : mat<T, MAJ, MIN>{};



// -----------------------------------------------------------------------------
// Set elements of matrix and vector

// Use a lambda function to iterate over vector and matrix
template <typename T, size_t N, typename F>
inline void set_vector(vec<T, N> &dest, F func) {
    for (int i = 0; i < N; ++i)
        dest.v[i] = func(i);
}

template <typename T, size_t MAJ, size_t MIN, typename F>
inline void set_matrix(mat<T, MAJ, MIN> &dest, F func) {
    for (int i = 0; i < MAJ; ++i)
        for (int j = 0; j < MIN; ++j)
            dest.m[i][j] = func(i, j);
}

template <typename T, size_t MAJ, size_t MIN>
inline void diagonal(mat<T, MAJ, MIN> &dest, T val) {
    set_matrix(dest, [val] (int i, int j)
                     -> T { return (i == j) ? val : T(0); });
}

template <typename T, size_t N>
inline void copy(mat<T, N, N> &dest, mat<T, N, N> &src) {
    set_matrix(dest, [&src] (int i, int j)
                     -> T { return src.m[i][j]; });
}

template <typename T, size_t N>
inline void add_scalar(vec<T, N> &dest, T val) {
    set_vector(dest, [&dest, val] (int i)
                     -> T { return dest.v[i] + val; });
}

template <typename T, size_t MAJ, size_t MIN>
inline void add_scalar(mat<T, MAJ, MIN> &dest, T val) {
    set_matrix(dest, [&dest, val] (int i, int j)
                     -> T { return dest.m[i][j] + val; });
}



// -----------------------------------------------------------------------------
// Matrix multiplication

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
// T(dest(MAJ,MIN))   = T(b(K,MIN)) * T(a(MAJ,K))
// [ a' e' i' m'      = [ A E I M   * [ a e i m
//   b' f' j' n'          B F J N       b f j n
//   c' g' k' o'          C G K O       c g k o
//   d' h' l' p' ]        D H L P ]     d h l p ]
//                    = [ Aa + Eb + Ic + Md, Ae + Ef + Ig + Mh, Ai + Ej + Ik + Ml, Am + En + Io + Mp
//                        Ba + Fb + Jc + Nd, Be + Ff + Jg + Nh, Bi + Fj + Jk + Nl, Bm + Fn + Jo + Np
//                        Ca + Gb + Kc + Od, Ce + Gf + Kg + Oh, Ci + Gj + Kk + Ol, Cm + Gn + Ko + Op
//                        Da + Hb + Lc + Pd, De + Hf + Lg + Ph, Di + Hj + Lk + Pl, Dm + Hn + Lo + Pp ]
// Linear array = [ Aa+Eb+Ic+Md, Ba+Fb+Jc+Nd, Ca+Gb+Kc+Od, Da+Hb+Lc+Pd,
//                  Ae+Ef+Ig+Mh, Be+Ff+Jg+Nh, Ce+Gf+Kg+Oh, De+Hf+Lg+Ph,
//                  Ai+Ej+Ik+Ml, Bi+Fj+Jk+Nl, Ci+Gj+Kk+Ol, Di+Hj+Lk+Pl,
//                  Am+En+Io+Mp, Bm+Fn+Jo+Np, Cm+Gn+Ko+Op, Dm+Hn+Lo+Pp ]
//
// Note the linear arrays are the same

template <typename T, size_t MAJ, size_t MIN, size_t K>
inline specialized mat_x_mat(mat<T, MAJ, MIN> &dest,
                             mat<T, MAJ, K>   &a,
                             mat<T, K,   MIN> &b) {
    for (int i = 0; i < MAJ; ++i) {
        for (int j = 0; j < MIN; ++j) {
            auto sum = T(0);

            for (int k = 0; k < K; ++k) {
                sum += a.m[i][k] * b.m[k][j];
            }
            
            dest.m[i][j] = sum;
        }
    }
    
    return loops;
}

template <typename T, size_t MAJ, size_t MIN, size_t K>
inline specialized rmata_x_rmatb(rmat<T, MAJ, MIN> &dest,
                                 rmat<T, MAJ, K>   &a,
                                 rmat<T, K,   MIN> &b) {
    return mat_x_mat(dest, a, b);
}

template <typename T, size_t MAJ, size_t MIN, size_t K>
inline specialized cmatb_x_cmata(cmat<T, MIN, MAJ> &tdest,
                                 cmat<T, K,   MIN> &tb,
                                 cmat<T, MAJ, K>   &ta) {
    // Transpositions not needed since memory layout the same
    return mat_x_mat(tdest, ta, tb);
}



// -----------------------------------------------------------------------------
// Matrix and vector multiplication

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
// T(dest(1,MIN)) = T(m(MAJ,MIN))        * T(v(1,MAJ))
// [ x'           = [ a e i m            * [ x
//   y'               b f j n                y
//   z'               c g k o                z
//   w' ]             d h l p ]              w ]
//                = [ ax + ey + iz + mw
//                    bx + fy + jz + nw
//                    cx + gy + kz + ow
//                    dx + hy + lz + pw ]
// Linear array: [ ax+ey+iz+mw, bx+fy+jz+nw, cx+gy+kz+ow, dx+hy+lz+pw ]
//
// Note the linear arrays are the same

template <typename T, size_t MAJ, size_t MIN>
inline specialized vec_x_mat(vec <T, MIN>      &dest,
                             vec <T, MAJ>      &v,
                             mat <T, MAJ, MIN> &m) {
    for (int j = 0; j < MIN; ++j) {
        auto sum = T(0);
        
        for (int i = 0; i < MAJ; ++i) {
            sum += v.v[i] * m.m[i][j];
        }
        dest.v[j] = sum;
    }
    
    return loops;
}

template <typename T, size_t MAJ, size_t MIN>
inline specialized rvec_x_rmat(rvec <T, MIN>      &dest,
                               rvec <T, MAJ>      &v,
                               rmat <T, MAJ, MIN> &m) {
    return vec_x_mat(dest, v, m);
}

template <typename T, size_t MAJ, size_t MIN>
inline specialized cmat_x_cvec(cvec <T, MAJ>      &tdest,
                               cmat <T, MAJ, MIN> &tm,
                               cvec <T, MIN>      &tv) {
    // Transpositions not needed since memory layout the same
    return vec_x_mat(tdest, tv, tm);
}



// -----------------------------------------------------------------------------
// Matrix and vector array multiplication

template <typename T, size_t MAJ, size_t MIN>
inline specialized vecarr_x_mat(vec <T, MAJ>      *dest,
                                vec <T, MAJ>      *v,
                                mat <T, MAJ, MIN> &m,
                                size_t            n) {
    for (int e = 0; e < n; ++e) {
        for (int j = 0; j < MIN; ++j) {
            auto sum = T(0);
            
            for (int i = 0; i < MAJ; ++i) {
                sum += v[e].v[i] * m.m[i][j];
            }
            dest[e].v[j] = sum;
        }
    }
    
    return loops;
}

template <typename T, size_t MAJ, size_t MIN>
inline specialized rvecarr_x_rmat(rvec <T, MAJ>      *dest,
                                  rvec <T, MAJ>      *v,
                                  rmat <T, MAJ, MIN> &m,
                                  size_t             n) {
    return vecarr_x_mat(dest, v, m, n);
}

template <typename T, size_t MAJ, size_t MIN>
inline specialized cmat_x_cvecarr(cvec <T, MIN>      *dest,
                                  cmat <T, MAJ, MIN> &m,
                                  cvec <T, MIN>      *v,
                                  size_t             n) {
    return vecarr_x_mat(dest, v, m, n);
}



}   // namespace matrix3d

#endif  // matrix3d_h
