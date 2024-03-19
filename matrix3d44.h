// matrix3d44.h

#ifndef matrix3d44_h
#define matrix3d44_h

#include <cstddef>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64)      // 64-bit Intel
#include <immintrin.h>
#elif defined(__aarch64__) || defined(__arm__)  // 64- or 32-bit ARM
#include <arm_neon.h>
#else
#error "No SIMD target"
#endif

namespace matrix3d {



// -----------------------------------------------------------------------------
// Assembly language specializations

#ifdef __cplusplus
extern "C" {
#endif

specialized mat_x_mat_f     (float  *dest, float  *a, float  *b);
specialized mat_x_mat_d     (double *dest, double *a, double *b);
specialized vecarr_x_mat_f  (float  *dest, float  *v, float  *m, size_t n);
specialized vecarr_x_mat_f2 (float  *dest, float  *v, float  *m, size_t n);
specialized vecarr_x_mat_d  (double *dest, double *v, double *m, size_t n);

#ifdef __cplusplus
}
#endif



// User defined compiler macro that allows unrolled 4x4 specializations
#ifdef UNROLL

// -----------------------------------------------------------------------------
// Matrix multiplication

template <typename T>
inline specialized mat_x_mat(mat<T, 4, 4> &dest,
                             mat<T, 4, 4> &a,
                             mat<T, 4, 4> &b) {
    
    T *pd = dest.m[0];
    T *pa = a.m[0];
    T *pb = b.m[0];

    T a0  = (*(pa +  0)),           // Some compilers needed a "hint"
      a1  = (*(pa +  1)),           // that it was not necessary to
      a2  = (*(pa +  2)),           // keep indexng into a and b data
      a3  = (*(pa +  3)),           // on each line
      b00 = (*(pb +  0)),
      b01 = (*(pb +  1)),
      b02 = (*(pb +  2)),
      b03 = (*(pb +  3)),
      b10 = (*(pb +  4)),
      b11 = (*(pb +  5)),
      b12 = (*(pb +  6)),
      b13 = (*(pb +  7)),
      b20 = (*(pb +  8)),
      b21 = (*(pb +  9)),
      b22 = (*(pb + 10)),
      b23 = (*(pb + 11)),
      b30 = (*(pb + 12)),
      b31 = (*(pb + 13)),
      b32 = (*(pb + 14)),
      b33 = (*(pb + 15));
    
    // Fully unrolled 4x4 matrix multiply
    
    (*(pd + 0)) = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
    (*(pd + 1)) = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
    (*(pd + 2)) = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
    (*(pd + 3)) = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;
    
    a0 = (*(pa + 4));
    a1 = (*(pa + 5));
    a2 = (*(pa + 6));
    a3 = (*(pa + 7));
    (*(pd + 4)) = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
    (*(pd + 5)) = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
    (*(pd + 6)) = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
    (*(pd + 7)) = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;
    
    a0 = (*(pa +  8));
    a1 = (*(pa +  9));
    a2 = (*(pa + 10));
    a3 = (*(pa + 11));
    (*(pd +  8)) = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
    (*(pd +  9)) = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
    (*(pd + 10)) = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
    (*(pd + 11)) = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;
    
    a0 = (*(pa + 12));
    a1 = (*(pa + 13));
    a2 = (*(pa + 14));
    a3 = (*(pa + 15));
    (*(pd + 12)) = a0 * b00 + a1 * b10 + a2 * b20 + a3 * b30;
    (*(pd + 13)) = a0 * b01 + a1 * b11 + a2 * b21 + a3 * b31;
    (*(pd + 14)) = a0 * b02 + a1 * b12 + a2 * b22 + a3 * b32;
    (*(pd + 15)) = a0 * b03 + a1 * b13 + a2 * b23 + a3 * b33;
    
    return unroll;
}



// -----------------------------------------------------------------------------
// Matrix and vector array multiplication

template <typename T>
inline specialized vecarr_x_mat(vec <T, 4>    *dest,
                                vec <T, 4>    *v,
                                mat <T, 4, 4> &m,
                                size_t        n) {

    T *pd = dest->v;
    T *pv = v->v;
    T *pm = m.m[0];

    T m00 = (*(pm +  0)),
      m01 = (*(pm +  1)),
      m02 = (*(pm +  2)),
      m03 = (*(pm +  3)),
      m10 = (*(pm +  4)),
      m11 = (*(pm +  5)),
      m12 = (*(pm +  6)),
      m13 = (*(pm +  7)),
      m20 = (*(pm +  8)),
      m21 = (*(pm +  9)),
      m22 = (*(pm + 10)),
      m23 = (*(pm + 11)),
      m30 = (*(pm + 12)),
      m31 = (*(pm + 13)),
      m32 = (*(pm + 14)),
      m33 = (*(pm + 15));

    for (int i = 0; i < n; ++i, pd += 4, pv += 4) {
        T v0 = (*(pv + 0)),
          v1 = (*(pv + 1)),
          v2 = (*(pv + 2)),
          v3 = (*(pv + 3));
        
        // Fully unrolled vector matrix multiplication
        
        (*(pd + 0)) = v0 * m00 + v1 * m10 + v2 * m20 + v3 * m30;
        (*(pd + 1)) = v0 * m01 + v1 * m11 + v2 * m21 + v3 * m31;
        (*(pd + 2)) = v0 * m02 + v1 * m12 + v2 * m22 + v3 * m32;
        (*(pd + 3)) = v0 * m03 + v1 * m13 + v2 * m23 + v3 * m33;
    }
    
    return unroll;
}

#endif  // UNROLL



// User defined compiler macros that allows intrinsics 4x4 specializations
#if defined(INTRIN) || defined(INTRIN256)

#if defined(__x86_64__) || defined(_M_X64)      // 64-bit Intel



// -----------------------------------------------------------------------------
// Matrix multiplication

template <>
inline specialized mat_x_mat(mat<float, 4, 4> &dest,
                             mat<float, 4, 4> &a,
                             mat<float, 4, 4> &b) {
    __m128 row0, row1, row2, row3, vec0, vec1, vec2, vec3, vecd;
    float *pd = dest.m[0];
    float *pa = a.m[0];
    float *pb = b.m[0];

    row0 = _mm_load_ps    (pb +  0);                        // Load all the matrix rows
    row1 = _mm_load_ps    (pb +  4);
    row2 = _mm_load_ps    (pb +  8);
    row3 = _mm_load_ps    (pb + 12);
    
    vecd = _mm_setzero_ps ();                               // Zero out vector
    vec0 = _mm_set_ps1    (*(pa + 0));                      // Duplicate the 1st element
    vec1 = _mm_set_ps1    (*(pa + 1));                      //   of each column in a vector
    vec2 = _mm_set_ps1    (*(pa + 2));
    vec3 = _mm_set_ps1    (*(pa + 3));
    vecd = _mm_fmadd_ps   (row0, vec0, vecd);               // Multiply and add the elements
    vecd = _mm_fmadd_ps   (row1, vec1, vecd);
    vecd = _mm_fmadd_ps   (row2, vec2, vecd);
    vecd = _mm_fmadd_ps   (row3, vec3, vecd);
           _mm_store_ps   (pd, vecd);                       // Store a vector
    
    vecd = _mm_setzero_ps ();
    vec0 = _mm_set_ps1    (*(pa + 4));                      // 2nd element of each column
    vec1 = _mm_set_ps1    (*(pa + 5));
    vec2 = _mm_set_ps1    (*(pa + 6));
    vec3 = _mm_set_ps1    (*(pa + 7));
    vecd = _mm_fmadd_ps   (row0, vec0, vecd);
    vecd = _mm_fmadd_ps   (row1, vec1, vecd);
    vecd = _mm_fmadd_ps   (row2, vec2, vecd);
    vecd = _mm_fmadd_ps   (row3, vec3, vecd);
           _mm_store_ps   (pd + 4, vecd);

    vecd = _mm_setzero_ps ();
    vec0 = _mm_set_ps1    (*(pa +  8));                     // 3rd element of each column
    vec1 = _mm_set_ps1    (*(pa +  9));
    vec2 = _mm_set_ps1    (*(pa + 10));
    vec3 = _mm_set_ps1    (*(pa + 11));
    vecd = _mm_fmadd_ps   (row0, vec0, vecd);
    vecd = _mm_fmadd_ps   (row1, vec1, vecd);
    vecd = _mm_fmadd_ps   (row2, vec2, vecd);
    vecd = _mm_fmadd_ps   (row3, vec3, vecd);
           _mm_store_ps   (pd + 8, vecd);

    vecd = _mm_setzero_ps ();
    vec0 = _mm_set_ps1    (*(pa + 12));                     // 4th element of each column
    vec1 = _mm_set_ps1    (*(pa + 13));
    vec2 = _mm_set_ps1    (*(pa + 14));
    vec3 = _mm_set_ps1    (*(pa + 15));
    vecd = _mm_fmadd_ps   (row0, vec0, vecd);
    vecd = _mm_fmadd_ps   (row1, vec1, vecd);
    vecd = _mm_fmadd_ps   (row2, vec2, vecd);
    vecd = _mm_fmadd_ps   (row3, vec3, vecd);
           _mm_store_ps   (pd + 12, vecd);
    
    return intel;
}

template <>
inline specialized mat_x_mat(mat<double, 4, 4> &dest,
                             mat<double, 4, 4> &a,
                             mat<double, 4, 4> &b) {
    __m256d row0, row1, row2, row3, vec0, vec1, vec2, vec3, vecd;
    double *pd = dest.m[0];
    double *pa = a.m[0];
    double *pb = b.m[0];

    row0 = _mm256_load_pd    (pb +  0);                     // Load all the matrix rows
    row1 = _mm256_load_pd    (pb +  4);
    row2 = _mm256_load_pd    (pb +  8);
    row3 = _mm256_load_pd    (pb + 12);
    
    vecd = _mm256_setzero_pd ();                            // Zero out vector
    vec0 = _mm256_set1_pd    (*(pa + 0));                   // Duplicate the 1st element
    vec1 = _mm256_set1_pd    (*(pa + 1));                   //   of each column in a vector
    vec2 = _mm256_set1_pd    (*(pa + 2));
    vec3 = _mm256_set1_pd    (*(pa + 3));
    vecd = _mm256_fmadd_pd   (row0, vec0, vecd);            // Multiply and add the elements
    vecd = _mm256_fmadd_pd   (row1, vec1, vecd);
    vecd = _mm256_fmadd_pd   (row2, vec2, vecd);
    vecd = _mm256_fmadd_pd   (row3, vec3, vecd);
           _mm256_store_pd   (pd, vecd);                    // Store a vector
    
    vecd = _mm256_setzero_pd ();
    vec0 = _mm256_set1_pd    (*(pa + 4));                   // 2nd element of each column
    vec1 = _mm256_set1_pd    (*(pa + 5));
    vec2 = _mm256_set1_pd    (*(pa + 6));
    vec3 = _mm256_set1_pd    (*(pa + 7));
    vecd = _mm256_fmadd_pd   (row0, vec0, vecd);
    vecd = _mm256_fmadd_pd   (row1, vec1, vecd);
    vecd = _mm256_fmadd_pd   (row2, vec2, vecd);
    vecd = _mm256_fmadd_pd   (row3, vec3, vecd);
           _mm256_store_pd   (pd + 4, vecd);

    vecd = _mm256_setzero_pd ();
    vec0 = _mm256_set1_pd    (*(pa +  8));                  // 3rd element of each column
    vec1 = _mm256_set1_pd    (*(pa +  9));
    vec2 = _mm256_set1_pd    (*(pa + 10));
    vec3 = _mm256_set1_pd    (*(pa + 11));
    vecd = _mm256_fmadd_pd   (row0, vec0, vecd);
    vecd = _mm256_fmadd_pd   (row1, vec1, vecd);
    vecd = _mm256_fmadd_pd   (row2, vec2, vecd);
    vecd = _mm256_fmadd_pd   (row3, vec3, vecd);
           _mm256_store_pd   (pd + 8, vecd);

    vecd = _mm256_setzero_pd  ();
    vec0 = _mm256_set1_pd     (*(pa + 12));                 // 4th element of each column
    vec1 = _mm256_set1_pd     (*(pa + 13));
    vec2 = _mm256_set1_pd     (*(pa + 14));
    vec3 = _mm256_set1_pd     (*(pa + 15));
    vecd = _mm256_fmadd_pd    (row0, vec0, vecd);
    vecd = _mm256_fmadd_pd    (row1, vec1, vecd);
    vecd = _mm256_fmadd_pd    (row2, vec2, vecd);
    vecd = _mm256_fmadd_pd    (row3, vec3, vecd);
           _mm256_store_pd    (pd + 12, vecd);
    
    return intel;
}



// -----------------------------------------------------------------------------
// Matrix and vector array multiplication

template <>
inline specialized vecarr_x_mat(vec <float, 4>    *dest,
                                vec <float, 4>    *v,
                                mat <float, 4, 4> &m,
                                size_t            n) {
// User defined compiler macro that allows two vector, 8 lane, implementations
#ifdef INTRIN256
    
    __m256 row0, row1, row2, row3, vec0, vec1, vec2, vec3, vecd;
    float *pd = dest->v;
    float *pv = v->v;
    float *pm = m.m[0];

    row0 = _mm256_loadu2_m128(pm +  0, pm +  0);        // Load the matrix twice,
    row1 = _mm256_loadu2_m128(pm +  4, pm +  4);        //   into upper and lower
    row2 = _mm256_loadu2_m128(pm +  8, pm +  8);        //   halves of vector
    row3 = _mm256_loadu2_m128(pm + 12, pm + 12);
    
    n = (n + 1) / 2;                                    // Process vectors in pairs
    
    for (int i = 0; i < n; ++i, pd += 8, pv += 8) {
        vecd = _mm256_setzero_ps ();                    // Zero out vectors
        vec0 = _mm256_set_ps     (*(pv + 4), *(pv + 4), // Duplicate 1st elements from
                                  *(pv + 4), *(pv + 4), //   each column into 4 lanes
                                  *(pv + 0), *(pv + 0),
                                  *(pv + 0), *(pv + 0));
        vec1 = _mm256_set_ps     (*(pv + 5), *(pv + 5), // 2nd elements
                                  *(pv + 5), *(pv + 5),
                                  *(pv + 1), *(pv + 1),
                                  *(pv + 1), *(pv + 1));
        vec2 = _mm256_set_ps     (*(pv + 6), *(pv + 6), // 3rd elements
                                  *(pv + 6), *(pv + 6),
                                  *(pv + 2), *(pv + 2),
                                  *(pv + 2), *(pv + 2));
        vec3 = _mm256_set_ps     (*(pv + 7), *(pv + 7), // 4th elements
                                  *(pv + 7), *(pv + 7),
                                  *(pv + 3), *(pv + 3),
                                  *(pv + 3), *(pv + 3));
        vecd = _mm256_fmadd_ps   (row0, vec0, vecd);    // Multiply and add the elements
        vecd = _mm256_fmadd_ps   (row1, vec1, vecd);
        vecd = _mm256_fmadd_ps   (row2, vec2, vecd);
        vecd = _mm256_fmadd_ps   (row3, vec3, vecd);
               _mm256_store_ps   (pd, vecd);            // Store a vector
    }
    
    return intel256;

// Single vector, 4 lane, implementations
#else
    
    __m128 row0, row1, row2, row3, vec0, vec1, vec2, vec3, vecd;
    float *pd = dest->v;
    float *pv = v->v;
    float *pm = m.m[0];

    row0 = _mm_load_ps(pm +  0);                        // Load all the matrix rows
    row1 = _mm_load_ps(pm +  4);
    row2 = _mm_load_ps(pm +  8);
    row3 = _mm_load_ps(pm + 12);
    
    for (int i = 0; i < n; ++i, pd += 4, pv += 4) {
        vecd = _mm_setzero_ps ();                       // Zero out vector
        vec0 = _mm_set1_ps    (*(pv + 0));              // Duplicate the nth element
        vec1 = _mm_set1_ps    (*(pv + 1));              //   of each column in a vector
        vec2 = _mm_set1_ps    (*(pv + 2));
        vec3 = _mm_set1_ps    (*(pv + 3));
        vecd = _mm_fmadd_ps   (row0, vec0, vecd);       // Multiply and add the elements
        vecd = _mm_fmadd_ps   (row1, vec1, vecd);
        vecd = _mm_fmadd_ps   (row2, vec2, vecd);
        vecd = _mm_fmadd_ps   (row3, vec3, vecd);
               _mm_store_ps   (pd, vecd);               // Store a vector
    }
    
    return intel;

#endif  // INTRIN256
}

template <>
inline specialized vecarr_x_mat(vec <double, 4>    *dest,
                                vec <double, 4>    *v,
                                mat <double, 4, 4> &m,
                                size_t             n) {
    __m256d row0, row1, row2, row3, vec0, vec1, vec2, vec3, vecd;
    double *pd = dest->v;
    double *pv = v->v;
    double *pm = m.m[0];

    row0 = _mm256_load_pd(pm +  0);                     // Load all the matrix rows
    row1 = _mm256_load_pd(pm +  4);
    row2 = _mm256_load_pd(pm +  8);
    row3 = _mm256_load_pd(pm + 12);

    for (int i = 0; i < n; ++i, pd += 4, pv += 4) {
        vecd = _mm256_setzero_pd ();                    // Zero out vector
        vec0 = _mm256_set1_pd    (*(pv + 0));           // Duplicate the nth element
        vec1 = _mm256_set1_pd    (*(pv + 1));           //   of each column in a vector
        vec2 = _mm256_set1_pd    (*(pv + 2));
        vec3 = _mm256_set1_pd    (*(pv + 3));
        vecd = _mm256_fmadd_pd   (row0, vec0, vecd);    // Multiply and add the elements
        vecd = _mm256_fmadd_pd   (row1, vec1, vecd);
        vecd = _mm256_fmadd_pd   (row2, vec2, vecd);
        vecd = _mm256_fmadd_pd   (row3, vec3, vecd);
               _mm256_store_pd   (pd, vecd);            // Store a vector
    }
    
    return intel;
}



#elif defined(__aarch64__) || defined(__arm__)  // 64- or 32-bit ARM



template <>
inline specialized mat_x_mat(mat<float, 4, 4> &dest,
                             mat<float, 4, 4> &a,
                             mat<float, 4, 4> &b) {
    float32x4_t row0, row1, row2, row3, vec0, vec1, vec2, vec3;
    float *pd = dest.m[0];
    float *pa = a.m[0];
    float *pb = b.m[0];

    row0 = vld1q_f32     (pb);              // Load all the matrix rows
    row1 = vld1q_f32     (pb +  4);
    row2 = vld1q_f32     (pb +  8);
    row3 = vld1q_f32     (pb + 12);

    vec0 = vld1q_dup_f32 (pa);              // Duplicate the 1st element
    vec1 = vld1q_dup_f32 (pa + 1);          //   of each column
    vec2 = vld1q_dup_f32 (pa + 2);
    vec3 = vld1q_dup_f32 (pa + 3);
    vec0 = vmulq_f32     (row0, vec0);      // Multiply the elements
    vec1 = vmulq_f32     (row1, vec1);
    vec2 = vmulq_f32     (row2, vec2);
    vec3 = vmulq_f32     (row3, vec3);
    vec0 = vaddq_f32     (vec0, vec1);      // Add the products
    vec1 = vaddq_f32     (vec2, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
           vst1q_f32     (pd, vec0);        // Store a vector

    vec0 = vld1q_dup_f32 (pa + 4);          // 2nd element of each column
    vec1 = vld1q_dup_f32 (pa + 5);
    vec2 = vld1q_dup_f32 (pa + 6);
    vec3 = vld1q_dup_f32 (pa + 7);
    vec0 = vmulq_f32     (row0, vec0);
    vec1 = vmulq_f32     (row1, vec1);
    vec2 = vmulq_f32     (row2, vec2);
    vec3 = vmulq_f32     (row3, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
    vec1 = vaddq_f32     (vec2, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
           vst1q_f32     (pd + 4, vec0);

    vec0 = vld1q_dup_f32 (pa +  8);         // 3rd element of each column
    vec1 = vld1q_dup_f32 (pa +  9);
    vec2 = vld1q_dup_f32 (pa + 10);
    vec3 = vld1q_dup_f32 (pa + 11);
    vec0 = vmulq_f32     (row0, vec0);
    vec1 = vmulq_f32     (row1, vec1);
    vec2 = vmulq_f32     (row2, vec2);
    vec3 = vmulq_f32     (row3, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
    vec1 = vaddq_f32     (vec2, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
           vst1q_f32     (pd + 8, vec0);

    vec0 = vld1q_dup_f32 (pa + 12);         // 4th element of each column
    vec1 = vld1q_dup_f32 (pa + 13);
    vec2 = vld1q_dup_f32 (pa + 14);
    vec3 = vld1q_dup_f32 (pa + 15);
    vec0 = vmulq_f32     (row0, vec0);
    vec1 = vmulq_f32     (row1, vec1);
    vec2 = vmulq_f32     (row2, vec2);
    vec3 = vmulq_f32     (row3, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
    vec1 = vaddq_f32     (vec2, vec3);
    vec0 = vaddq_f32     (vec0, vec1);
           vst1q_f32     (pd + 12, vec0);

    return arm;
}



// -----------------------------------------------------------------------------
// Matrix and vector array multiplication

template <>
inline specialized vecarr_x_mat(vec <float, 4>    *dest,
                                vec <float, 4>    *v,
                                mat <float, 4, 4> &m,
                                size_t            n) {
    float32x4_t   row0, row1, row2, row3, vec0, vec1, vec2, vec3, vecs;
    float *pd = dest->v;
    float *pv = v->v;
    float *pm = m.m[0];

    row0 = vld1q_f32(pm +  0);              // Load all the matrix rows
    row1 = vld1q_f32(pm +  4);
    row2 = vld1q_f32(pm +  8);
    row3 = vld1q_f32(pm + 12);

    for (int i = 0; i < n; ++i, pv += 4, pd += 4) {
        vec0 = vld1q_dup_f32 (pv + 0);      // Duplicate the nth element
        vec1 = vld1q_dup_f32 (pv + 1);      //   of each column in a vector
        vec2 = vld1q_dup_f32 (pv + 2);
        vec3 = vld1q_dup_f32 (pv + 3);
        vec0 = vmulq_f32     (row0, vec0);  // Multiply the elements
        vec1 = vmulq_f32     (row1, vec1);
        vec2 = vmulq_f32     (row2, vec2);
        vec3 = vmulq_f32     (row3, vec3);
        vec0 = vaddq_f32     (vec0, vec1);  // Add the products
        vec1 = vaddq_f32     (vec2, vec3);
        vec0 = vaddq_f32     (vec0, vec1);
               vst1q_f32     (pd, vec0);    // Store a vector
    }

    return arm;
}



#endif  // __x86_64__ _M_X64 __aarch64__ __arm__



// User defined compiler macros that allows assembly 4x4 specializations
#elif defined(ASM) || defined(ASM256)



#if defined(__x86_64__) || defined(_M_X64)      // 64-bit Intel

template <>
inline specialized mat_x_mat(mat<float, 4, 4> &dest,
                             mat<float, 4, 4> &a,
                             mat<float, 4, 4> &b) {
    return mat_x_mat_f(dest.m[0], a.m[0], b.m[0]);
}

template <>
inline specialized mat_x_mat(mat<double, 4, 4> &dest,
                             mat<double, 4, 4> &a,
                             mat<double, 4, 4> &b) {
    return mat_x_mat_d(dest.m[0], a.m[0], b.m[0]);
}

template <>
inline specialized vecarr_x_mat(vec <float, 4>    *dest,
                                vec <float, 4>    *v,
                                mat <float, 4, 4> &m,
                                size_t            n) {
#ifdef ASM256
    return vecarr_x_mat_f2 (dest->v, v->v, m.m[0], n);
#else
    return vecarr_x_mat_f  (dest->v, v->v, m.m[0], n);
#endif
}

template <>
inline specialized vecarr_x_mat(vec <double, 4>    *dest,
                                vec <double, 4>    *v,
                                mat <double, 4, 4> &m,
                                size_t             n) {
    return vecarr_x_mat_d(dest->v, v->v, m.m[0], n);
}

#elif defined(__aarch64__) || defined(__arm__)  // 64- or 32-bit ARM

template <>
inline specialized mat_x_mat(mat<float, 4, 4> &dest,
                             mat<float, 4, 4> &a,
                             mat<float, 4, 4> &b) {
    return mat_x_mat_f(dest.m[0], a.m[0], b.m[0]);
}

template <>
inline specialized vecarr_x_mat(vec <float, 4>    *dest,
                                vec <float, 4>    *v,
                                mat <float, 4, 4> &m,
                                size_t            n) {
#ifdef ASM256
    return vecarr_x_mat_f2 (dest->v, v->v, m.m[0], n);
#else
    return vecarr_x_mat_f  (dest->v, v->v, m.m[0], n);
#endif
}

#endif  // __x86_64__ _M_X64 __aarch64__ __arm__



#endif  // INTRIN INTRIN256 ASM ASM256



}   // namespace matrix3d

#endif  // matrix3d44_h
