// neon.s
//
// Implements NEON asssembly code.
//     mat_x_mat_f       Matrix 4x4 multiplication
//     mat_x_mat_d
//     vecarr_x_mat_f    Matrix and vector 4x4 multiplication
//     vecarr_x_mat_d

specialized     =           9                       // Must match C enumeration

                .text
                .balign     4
                .global      mat_x_mat_f,  mat_x_mat_d
                .global     _mat_x_mat_f, _mat_x_mat_d
                .global      vecarr_x_mat_f,  vecarr_x_mat_f2,  vecarr_x_mat_d
                .global     _vecarr_x_mat_f, _vecarr_x_mat_f2, _vecarr_x_mat_d



//------------------------------------------------------------------------------
// Matrix 4x4 multiplication

//------------------------------------------------------------------------------
// specialized mat_x_mat_f(float *dest, float *a, float *b);
// Arguments:
//     X0  Destination 4x4 matrix
//     X1  Left source 4x4 matrix
//     X2  Right source 4x4 matrix
// Return:
//     X0  Specialization identifying AVX2 code

                .balign     16
mat_x_mat_f:
_mat_x_mat_f:
                ld1         { v0.4s - v3.4s }, [x2] // Load all the matrix rows

                dup         v8.4s, wzr              // Zero destination vector

                ld1r        { v4.4s }, [x1], #4     // Duplicate the 1st element
                ld1r        { v5.4s }, [x1], #4     //   of each column
                ld1r        { v6.4s }, [x1], #4
                ld1r        { v7.4s }, [x1], #4

                fmla        v8.4s,  v0.4s,  v4.s[0] // Multiply and add the elements
                fmla        v8.4s,  v1.4s,  v5.s[1]
                fmla        v8.4s,  v2.4s,  v6.s[2]
                fmla        v8.4s,  v3.4s,  v7.s[3]

                st1         { v8.4s }, [x0], #16    // Store destination vector

                dup         v8.4s, wzr
                ld1r        { v4.4s }, [x1], #4     // 2nd element
                ld1r        { v5.4s }, [x1], #4
                ld1r        { v6.4s }, [x1], #4
                ld1r        { v7.4s }, [x1], #4
                fmla        v8.4s,  v0.4s,  v4.s[0]
                fmla        v8.4s,  v1.4s,  v5.s[1]
                fmla        v8.4s,  v2.4s,  v6.s[2]
                fmla        v8.4s,  v3.4s,  v7.s[3]
                st1         { v8.4s }, [x0], #16

                dup         v8.4s, wzr
                ld1r        { v4.4s }, [x1], #4     // 3rd element
                ld1r        { v5.4s }, [x1], #4
                ld1r        { v6.4s }, [x1], #4
                ld1r        { v7.4s }, [x1], #4
                fmla        v8.4s,  v0.4s,  v4.s[0]
                fmla        v8.4s,  v1.4s,  v5.s[1]
                fmla        v8.4s,  v2.4s,  v6.s[2]
                fmla        v8.4s,  v3.4s,  v7.s[3]
                st1         { v8.4s }, [x0], #16

                dup         v8.4s, wzr
                ld1r        { v4.4s }, [x1], #4     // 4th element
                ld1r        { v5.4s }, [x1], #4
                ld1r        { v6.4s }, [x1], #4
                ld1r        { v7.4s }, [x1], #4
                fmla        v8.4s,  v0.4s,  v4.s[0]
                fmla        v8.4s,  v1.4s,  v5.s[1]
                fmla        v8.4s,  v2.4s,  v6.s[2]
                fmla        v8.4s,  v3.4s,  v7.s[3]
                st1         { v8.4s }, [x0], #16

                mov         x0,     specialized
                ret



//------------------------------------------------------------------------------
// specialized mat_x_mat_d(double *dest, double *a, double *b);
// Arguments:
//     X0  Destination 4x4 matrix
//     X1  Left source 4x4 matrix
//     X2  Right source 4x4 matrix
// Return:
//     X0  Specialization identifying AVX2 code

                .balign     16
mat_x_mat_d:
_mat_x_mat_d:
                mov         x0,     specialized
                ret



//------------------------------------------------------------------------------
// Matrix and vector 4x4 multiplication

//------------------------------------------------------------------------------
// specialized vecarr_x_mat_f(float *dest, float *v, float *m, size_t n);
// Arguments:
//     X0  Destination 1x4 vector array
//     X1  Source 1x4 vector array
//     X2  Transformation 4x4 matrix
//     X3  Length of vector arrays
// Return:
//     X0  Specialization identifying AVX2 code

                .balign     16
vecarr_x_mat_f:
vecarr_x_mat_f2:
_vecarr_x_mat_f:
_vecarr_x_mat_f2:
                ld1         { v0.4s - v3.4s }, [x2] // Load all the matrix rows

1:              dup         v8.4s, wzr              // Zero destination vector

                ld1r        { v4.4s }, [x1], #4     // Duplicate the nth element
                ld1r        { v5.4s }, [x1], #4     //   of each column
                ld1r        { v6.4s }, [x1], #4
                ld1r        { v7.4s }, [x1], #4

                fmla        v8.4s,  v0.4s,  v4.s[0] // Multiply and add the elements
                fmla        v8.4s,  v1.4s,  v5.s[1]
                fmla        v8.4s,  v2.4s,  v6.s[2]
                fmla        v8.4s,  v3.4s,  v7.s[3]

                st1         { v8.4s }, [x0], #16    // Store destination vector

                subs        x3,     x3,     #1      // Branch if more vectors
                bne         1b                      //   to process

                mov         x0,     specialized
                ret



//------------------------------------------------------------------------------
// specialized vecarr_x_mat_d(double *dest, double *v, double *m, size_t n);
// Arguments:
//     X0  Destination 1x4 vector array
//     X1  Source 1x4 vector array
//     X2  Transformation 4x4 matrix
//     X3  Length of vector arrays
// Return:
//     X0  Specialization identifying AVX2 code

                .balign     16
vecarr_x_mat_d:
_vecarr_x_mat_d:
                mov         x0,     specialized
                ret
