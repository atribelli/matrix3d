// midr-a32.s

            .text
            .balign 4
            .global get_midr, _get_midr



//-----------------------------------------------------------------------------
// Get the MIDR register
// Return:
//     R0  MIDR

            .balign 16
get_midr:
_get_midr:
            mrc     p15, 0, r0, c0, c0, 0
            bx      lr
