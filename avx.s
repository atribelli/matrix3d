# avx.s
#
# Implements AVX2 aassembly code.
#     mat_x_mat_f       Matrix 4x4 multiplication
#     mat_x_mat_d
#     vecarr_x_mat_f    Matrix and vector 4x4 multiplication
#     vecarr_x_mat_d

                .intel_syntax noprefix

                .ifdef      IsLinux
                .section    .note.GNU-stack, "", %progbits
                .endif

specialized     =           4                       # Must match C enumeration

                .text
                .balign     4
                .global      mat_x_mat_f,  mat_x_mat_d
                .global     _mat_x_mat_f, _mat_x_mat_d
                .global      vecarr_x_mat_f,  vecarr_x_mat_f2,  vecarr_x_mat_d
                .global     _vecarr_x_mat_f, _vecarr_x_mat_f2, _vecarr_x_mat_d



#-------------------------------------------------------------------------------
# Matrix 4x4 multiplication

#-------------------------------------------------------------------------------
# specialized mat_x_mat_f(float *dest, float *a, float *b);
# Arguments:
#     RDI  Destination 4x4 matrix
#     RSI  Left source 4x4 matrix
#     RDX  Right source 4x4 matrix
# Return:
#     RAX  Specialization identifying AVX2 code

                .balign     16
mat_x_mat_f:
_mat_x_mat_f:
                movaps      xmm0,   [rdx]           # Load all the matrix rows
                movaps      xmm1,   [rdx + 16]
                movaps      xmm2,   [rdx + 32]
                movaps      xmm3,   [rdx + 48]

                xorps       xmm8,   xmm8            # Zero destination vector

                vbroadcastss xmm4,  [rsi]           # Duplicate the 1st element
                vbroadcastss xmm5,  [rsi +  4]      #   of each column
                vbroadcastss xmm6,  [rsi +  8]
                vbroadcastss xmm7,  [rsi + 12]

                vfmadd231ps xmm8,   xmm0,   xmm4    # Multiply and add the elements
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7

                movaps      [rdi],  xmm8            # Store destination vector

                xorps       xmm8,   xmm8
                vbroadcastss xmm4,  [rsi + 16]      # 2nd element
                vbroadcastss xmm5,  [rsi + 20]
                vbroadcastss xmm6,  [rsi + 24]
                vbroadcastss xmm7,  [rsi + 28]
                vfmadd231ps xmm8,   xmm0,   xmm4
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7
                movaps      [rdi + 16], xmm8

                xorps       xmm8,   xmm8
                vbroadcastss xmm4,  [rsi + 32]      # 3rd element
                vbroadcastss xmm5,  [rsi + 36]
                vbroadcastss xmm6,  [rsi + 40]
                vbroadcastss xmm7,  [rsi + 44]
                vfmadd231ps xmm8,   xmm0,   xmm4
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7
                movaps      [rdi + 32], xmm8

                xorps       xmm8,   xmm8
                vbroadcastss xmm4,  [rsi + 48]      # 4th element
                vbroadcastss xmm5,  [rsi + 52]
                vbroadcastss xmm6,  [rsi + 56]
                vbroadcastss xmm7,  [rsi + 60]
                vfmadd231ps xmm8,   xmm0,   xmm4
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7
                movaps      [rdi + 48], xmm8

                mov         rax,    specialized
                ret



#-------------------------------------------------------------------------------
# specialized mat_x_mat_d(double *dest, double *a, double *b);
# Arguments:
#     RDI  Destination 4x4 matrix
#     RSI  Left source 4x4 matrix
#     RDX  Right source 4x4 matrix
# Return:
#     RAX  Specialization identifying AVX2 code

                .balign     16
mat_x_mat_d:
_mat_x_mat_d:
                vmovapd     ymm0,   [rdx]           # Load all the matrix rows
                vmovapd     ymm1,   [rdx + 32]
                vmovapd     ymm2,   [rdx + 64]
                vmovapd     ymm3,   [rdx + 96]

                vxorpd      ymm8,   ymm8,   ymm8    # Zero destination vector

                vbroadcastsd ymm4,  [rsi]           # Duplicate the 1st element
                vbroadcastsd ymm5,  [rsi +  8]      #   of each column
                vbroadcastsd ymm6,  [rsi + 16]
                vbroadcastsd ymm7,  [rsi + 24]

                vfmadd231pd ymm8,   ymm0,   ymm4    # Multiply and add the elements
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7

                vmovapd     [rdi],  ymm8            # Store destination vector

                vxorpd      ymm8,   ymm8,   ymm8
                vbroadcastsd ymm4,  [rsi + 32]      # 2nd element
                vbroadcastsd ymm5,  [rsi + 40]
                vbroadcastsd ymm6,  [rsi + 48]
                vbroadcastsd ymm7,  [rsi + 56]
                vfmadd231pd ymm8,   ymm0,   ymm4
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7
                vmovapd     [rdi + 32], ymm8

                vxorpd      ymm8,   ymm8,   ymm8
                vbroadcastsd ymm4,  [rsi + 64]      # 3rd element
                vbroadcastsd ymm5,  [rsi + 72]
                vbroadcastsd ymm6,  [rsi + 80]
                vbroadcastsd ymm7,  [rsi + 88]
                vfmadd231pd ymm8,   ymm0,   ymm4
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7
                vmovapd     [rdi + 64], ymm8

                vxorpd      ymm8,   ymm8,   ymm8
                vbroadcastsd ymm4,  [rsi + 96]      # 4th element
                vbroadcastsd ymm5,  [rsi + 104]
                vbroadcastsd ymm6,  [rsi + 112]
                vbroadcastsd ymm7,  [rsi + 120]
                vfmadd231pd ymm8,   ymm0,   ymm4
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7
                vmovapd     [rdi + 96], ymm8

                mov         rax,    specialized
                ret



#-------------------------------------------------------------------------------
# Matrix and vector 4x4 multiplication

#-------------------------------------------------------------------------------
# specialized vecarr_x_mat_f(float *dest, float *v, float *m, size_t n);
# Arguments:
#     RDI  Destination 1x4 vector array
#     RSI  Source 1x4 vector array
#     RDX  Transformation 4x4 matrix
#     RCX  Length of vector arrays
# Return:
#     RAX  Specialization identifying AVX2 code

                .balign     16
vecarr_x_mat_f:
_vecarr_x_mat_f:
                # Single vector, 4 lane, implementation

                movaps      xmm0,   [rdx]           # Load all the matrix rows
                movaps      xmm1,   [rdx + 16]
                movaps      xmm2,   [rdx + 32]
                movaps      xmm3,   [rdx + 48]

1:              xorps       xmm8,   xmm8            # Zero destination vector

                vbroadcastss xmm4,  [rsi]           # Duplicate the nth element
                vbroadcastss xmm5,  [rsi +  4]      #   of each column
                vbroadcastss xmm6,  [rsi +  8]
                vbroadcastss xmm7,  [rsi + 12]

                vfmadd231ps xmm8,   xmm0,   xmm4    # Multiply and add the elements
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7

                movaps      [rdi],  xmm8            # Store destination vector

                add         rdi,    16              # Update vector pointers
                add         rsi,    16

                dec         rcx                     # Branch if more vectors
                jnz         1b                      #   to process

                mov         rax,    specialized
                ret



                .balign     16
vecarr_x_mat_f2:
_vecarr_x_mat_f2:
                # Two vector, 8 lane, implementation

                inc         rcx                     # Process vectors in pairs
                shr         rcx, 1

                vbroadcastf128 ymm0, [rdx]          # Load the matrix twice,
                vbroadcastf128 ymm1, [rdx + 16]     #   into upper and lower
                vbroadcastf128 ymm2, [rdx + 32]     #   halves of vector
                vbroadcastf128 ymm3, [rdx + 48]

1:              vxorps      ymm12,  ymm12,  ymm12   # Zero destination vector

                vbroadcastss ymm4,  [rsi]           # Duplicate the nth element
                vbroadcastss ymm5,  [rsi +  4]      #   of each column
                vbroadcastss ymm6,  [rsi +  8]
                vbroadcastss ymm7,  [rsi + 12]
                vbroadcastss ymm8,  [rsi + 16]
                vbroadcastss ymm9,  [rsi + 20]
                vbroadcastss ymm10, [rsi + 24]
                vbroadcastss ymm11, [rsi + 28]
                vinsertf128 ymm4,   ymm4,   xmm8,   1
                vinsertf128 ymm5,   ymm5,   xmm9,   1
                vinsertf128 ymm6,   ymm6,   xmm10,  1
                vinsertf128 ymm7,   ymm7,   xmm11,  1

                vfmadd231ps ymm12,  ymm0,   ymm4    # Multiply and add the elements
                vfmadd231ps ymm12,  ymm1,   ymm5
                vfmadd231ps ymm12,  ymm2,   ymm6
                vfmadd231ps ymm12,  ymm3,   ymm7

                vmovaps     [rdi],  ymm12           # Store destination vectors

                add         rdi,    32              # Update vector pointers
                add         rsi,    32

                dec         rcx                     # Branch if more vectors
                jnz         1b                      #   to process

                mov         rax,    specialized + 1
                ret



#-------------------------------------------------------------------------------
# specialized vecarr_x_mat_d(double *dest, double *v, double *m, size_t n);
# Arguments:
#     RDI  Destination 1x4 vector array
#     RSI  Source 1x4 vector array
#     RDX  Transformation 4x4 matrix
#     RCX  Length of vector arrays
# Return:
#     RAX  Specialization identifying AVX2 code

                .balign     16
vecarr_x_mat_d:
_vecarr_x_mat_d:
                vmovapd     ymm0,   [rdx]           # Load all the matrix rows
                vmovapd     ymm1,   [rdx + 32]
                vmovapd     ymm2,   [rdx + 64]
                vmovapd     ymm3,   [rdx + 96]

1:              vxorpd      ymm8,   ymm8,   ymm8    # Zero destination vector

                vbroadcastsd ymm4,  [rsi]           # Duplicate the nth element
                vbroadcastsd ymm5,  [rsi +  8]      #   of each column
                vbroadcastsd ymm6,  [rsi + 16]
                vbroadcastsd ymm7,  [rsi + 24]

                vfmadd231pd ymm8,   ymm0,   ymm4    # Multiply and add the elements
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7

                vmovapd     [rdi],  ymm8            # Store destination vector

                add         rdi,    32              # Update vector pointers
                add         rsi,    32

                dec         rcx                     # Branch if more vectors
                jnz         1b                      #   to process

                mov         rax,    specialized
                ret
