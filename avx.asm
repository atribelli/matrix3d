; avx.asm
;
; Implements AVX2 assembly code.
;     mat_x_mat_f       Matrix 4x4 multiplication
;     mat_x_mat_d
;     vecarr_x_mat_f    Matrix and vector 4x4 multiplication
;     vecarr_x_mat_d

specialized     equ         5                       ; Must match C enumeration

                .code
                align       4
                public      mat_x_mat_f, mat_x_mat_d
                public      vecarr_x_mat_f, vecarr_x_mat_f2, vecarr_x_mat_d



;-------------------------------------------------------------------------------
; Matrix 4x4 multiplication

;-------------------------------------------------------------------------------
; specialized mat_x_mat_f(float *dest, float *a, float *b);
; Arguments:
;     RCX  Destination 4x4 matrix
;     RDX  Left source 4x4 matrix
;     R8   Right source 4x4 matrix
; Return:
;     RAX  Specialization identifying AVX2 code

                align       16
mat_x_mat_f     proc
                movaps      xmm0,   [r8]            ; Load all the matrix rows
                movaps      xmm1,   [r8 + 16]
                movaps      xmm2,   [r8 + 32]
                movaps      xmm3,   [r8 + 48]

                xorps       xmm8,   xmm8            ; Zero destination vector

                vbroadcastss xmm4,  dword ptr [rdx]      ; Duplicate the 1st element
                vbroadcastss xmm5,  dword ptr [rdx +  4] ;   of each column
                vbroadcastss xmm6,  dword ptr [rdx +  8]
                vbroadcastss xmm7,  dword ptr [rdx + 12]

                vfmadd231ps xmm8,   xmm0,   xmm4    ; Multiply and add the elements
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7

                movaps      [rcx],  xmm8            ; Store destination vector

                xorps       xmm8,   xmm8
                vbroadcastss xmm4,  dword ptr [rdx + 16] ; 2nd element
                vbroadcastss xmm5,  dword ptr [rdx + 20]
                vbroadcastss xmm6,  dword ptr [rdx + 24]
                vbroadcastss xmm7,  dword ptr [rdx + 28]
                vfmadd231ps xmm8,   xmm0,   xmm4
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7
                movaps      [rcx + 16], xmm8

                xorps       xmm8,   xmm8
                vbroadcastss xmm4,  dword ptr [rdx + 32] ; 3rd element
                vbroadcastss xmm5,  dword ptr [rdx + 36]
                vbroadcastss xmm6,  dword ptr [rdx + 40]
                vbroadcastss xmm7,  dword ptr [rdx + 44]
                vfmadd231ps xmm8,   xmm0,   xmm4
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7
                movaps      [rcx + 32], xmm8

                xorps       xmm8,   xmm8
                vbroadcastss xmm4,  dword ptr [rdx + 48] ; 4th element
                vbroadcastss xmm5,  dword ptr [rdx + 52]
                vbroadcastss xmm6,  dword ptr [rdx + 56]
                vbroadcastss xmm7,  dword ptr [rdx + 60]
                vfmadd231ps xmm8,   xmm0,   xmm4
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7
                movaps      [rcx + 48], xmm8

                mov         rax,    specialized
                ret
mat_x_mat_f     endp



;-------------------------------------------------------------------------------
; specialized mat_x_mat_d(double *dest, double *a, double *b);
; Arguments:
;     RCX  Destination 4x4 matrix
;     RDX  Left source 4x4 matrix
;     R8   Right source 4x4 matrix
; Return:
;     RAX  Specialization identifying AVX2 code

                align       16
mat_x_mat_d     proc
                vmovapd     ymm0,   [r8]            ; Load all the matrix rows
                vmovapd     ymm1,   [r8 + 32]
                vmovapd     ymm2,   [r8 + 64]
                vmovapd     ymm3,   [r8 + 96]

                vxorpd      ymm8,   ymm8,   ymm8    ; Zero destination vector

                vbroadcastsd ymm4,  qword ptr [rdx]      ; Duplicate the 1st element
                vbroadcastsd ymm5,  qword ptr [rdx +  8] ;   of each column
                vbroadcastsd ymm6,  qword ptr [rdx + 16]
                vbroadcastsd ymm7,  qword ptr [rdx + 24]

                vfmadd231pd ymm8,   ymm0,   ymm4    ; Multiply and add the elements
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7

                vmovapd     [rcx],  ymm8            ; Store destination vector

                vxorpd      ymm8,   ymm8,   ymm8
                vbroadcastsd ymm4,  qword ptr [rdx + 32] ; 2nd element
                vbroadcastsd ymm5,  qword ptr [rdx + 40]
                vbroadcastsd ymm6,  qword ptr [rdx + 48]
                vbroadcastsd ymm7,  qword ptr [rdx + 56]
                vfmadd231pd ymm8,   ymm0,   ymm4
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7
                vmovapd     [rcx + 32], ymm8

                vxorpd      ymm8,   ymm8,   ymm8
                vbroadcastsd ymm4,  qword ptr [rdx + 64] ; 3rd element
                vbroadcastsd ymm5,  qword ptr [rdx + 72]
                vbroadcastsd ymm6,  qword ptr [rdx + 80]
                vbroadcastsd ymm7,  qword ptr [rdx + 88]
                vfmadd231pd ymm8,   ymm0,   ymm4
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7
                vmovapd     [rcx + 64], ymm8

                vxorpd      ymm8,   ymm8,   ymm8
                vbroadcastsd ymm4,  qword ptr [rdx + 96] ; 4th element
                vbroadcastsd ymm5,  qword ptr [rdx + 104]
                vbroadcastsd ymm6,  qword ptr [rdx + 112]
                vbroadcastsd ymm7,  qword ptr [rdx + 120]
                vfmadd231pd ymm8,   ymm0,   ymm4
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7
                vmovapd     [rcx + 96], ymm8

                mov         rax,    specialized
                ret
mat_x_mat_d     endp



;-------------------------------------------------------------------------------
; Matrix and vector 4x4 multiplication

;-------------------------------------------------------------------------------
; specialized vecarr_x_mat_f(float *dest, float *v, float *m, size_t n);
; Arguments:
;     RCX  Destination 1x4 vector array
;     RDX  Source 1x4 vector array
;     R8   Transformation 4x4 matrix
;     R9   Length of vector arrays
; Return:
;     RAX  Specialization identifying AVX2 code

                align       16
vecarr_x_mat_f  proc
                ; Single vector, 4 lane, implementation

                movaps      xmm0,   [r8]            ; Load all the matrix rows
                movaps      xmm1,   [r8 + 16]
                movaps      xmm2,   [r8 + 32]
                movaps      xmm3,   [r8 + 48]

next:           xorps       xmm8,   xmm8            ; Zero destination vector

                vbroadcastss xmm4,  dword ptr [rdx]      ; Duplicate the nth element
                vbroadcastss xmm5,  dword ptr [rdx +  4] ;   of each column
                vbroadcastss xmm6,  dword ptr [rdx +  8]
                vbroadcastss xmm7,  dword ptr [rdx + 12]

                vfmadd231ps xmm8,   xmm0,   xmm4    ; Multiply and add the elements
                vfmadd231ps xmm8,   xmm1,   xmm5
                vfmadd231ps xmm8,   xmm2,   xmm6
                vfmadd231ps xmm8,   xmm3,   xmm7

                movaps      [rcx],  xmm8            ; Store destination vector

                add         rcx,    16              ; Update vector pointers
                add         rdx,    16

                dec         r9                      ; Branch if more vectors
                jnz         next                    ;   to process

                mov         rax,    specialized
                ret
vecarr_x_mat_f  endp



                align       16
vecarr_x_mat_f2 proc
                ; Two vector, 8 lane, implementation

                inc         r9                      ; Process vectors in pairs
                shr         r9, 1

                vbroadcastf128 ymm0, oword ptr [r8]      ; Load the matrix twice,
                vbroadcastf128 ymm1, oword ptr [r8 + 16] ;   into upper and lower
                vbroadcastf128 ymm2, oword ptr [r8 + 32] ;   halves of vector
                vbroadcastf128 ymm3, oword ptr [r8 + 48]

next:           vxorps      ymm12,  ymm12,  ymm12   ; Zero destination vector

                vbroadcastss ymm4,  dword ptr [rdx]      ; Duplicate the nth element
                vbroadcastss ymm5,  dword ptr [rdx +  4] ;   of each column
                vbroadcastss ymm6,  dword ptr [rdx +  8]
                vbroadcastss ymm7,  dword ptr [rdx + 12]
                vbroadcastss ymm8,  dword ptr [rdx + 16]
                vbroadcastss ymm9,  dword ptr [rdx + 20]
                vbroadcastss ymm10, dword ptr [rdx + 24]
                vbroadcastss ymm11, dword ptr [rdx + 28]
                vinsertf128 ymm4,   ymm4,   xmm8,   1
                vinsertf128 ymm5,   ymm5,   xmm9,   1
                vinsertf128 ymm6,   ymm6,   xmm10,  1
                vinsertf128 ymm7,   ymm7,   xmm11,  1

                vfmadd231ps ymm12,  ymm0,   ymm4    ; Multiply and add the elements
                vfmadd231ps ymm12,  ymm1,   ymm5
                vfmadd231ps ymm12,  ymm2,   ymm6
                vfmadd231ps ymm12,  ymm3,   ymm7

                vmovaps     [rcx],  ymm12           ; Store destination vectors

                add         rcx,    32              ; Update vector pointers
                add         rdx,    32

                dec         r9                      ; Branch if more vectors
                jnz         next                    ;   to process

                mov         rax,    specialized + 1
                ret
vecarr_x_mat_f2 endp



;-------------------------------------------------------------------------------
; specialized vecarr_x_mat_d(double *dest, double *v, double *m, size_t n);
; Arguments:
;     RCX  Destination 1x4 vector array
;     RDX  Source 1x4 vector array
;     R8   Transformation 4x4 matrix
;     R9   Length of vector arrays
; Return:
;     RAX  Specialization identifying AVX2 code

                align       16
vecarr_x_mat_d  proc
                vmovapd     ymm0,   [r8]            ; Load all the matrix rows
                vmovapd     ymm1,   [r8 + 32]
                vmovapd     ymm2,   [r8 + 64]
                vmovapd     ymm3,   [r8 + 96]

next:           vxorpd      ymm8,   ymm8,   ymm8    ; Zero destination vector

                vbroadcastsd ymm4,  qword ptr [rdx]      ; Duplicate the nth element
                vbroadcastsd ymm5,  qword ptr [rdx +  8] ;   of each column
                vbroadcastsd ymm6,  qword ptr [rdx + 16]
                vbroadcastsd ymm7,  qword ptr [rdx + 24]

                vfmadd231pd ymm8,   ymm0,   ymm4    ; Multiply and add the elements
                vfmadd231pd ymm8,   ymm1,   ymm5
                vfmadd231pd ymm8,   ymm2,   ymm6
                vfmadd231pd ymm8,   ymm3,   ymm7

                vmovapd     [rcx],  ymm8            ; Store destination vector

                add         rcx,    32              ; Update vector pointers
                add         rdx,    32

                dec         r9                      ; Branch if more vectors
                jnz         next                    ;   to process

                mov         rax,    specialized
                ret
vecarr_x_mat_d  endp

                end
