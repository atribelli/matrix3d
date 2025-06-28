// midr-a64.s

            .text
            .balign 4



//-----------------------------------------------------------------------------
// Get the MIDR register
// Return:
//     X0  MIDR

            .balign 16
            .global get_midr, _get_midr
get_midr:
_get_midr:
            mrs     x0, MIDR_EL1
            ret

            .balign 16
            .global get_mpidr, _get_mpidr
get_mpidr:
_get_mpidr:
            mrs     x0, MPIDR_EL1
            ret

            .balign 16
            .global get_revidr, _get_revidr
get_revidr:
_get_revidr:
            mrs     x0, REVIDR_EL1
            ret

            .balign 16
            .global get_isar0, _get_isar0
get_isar0:
_get_isar0:
            mrs     x0, ID_AA64ISAR0_EL1
            ret

            .balign 16
            .global get_isar1, _get_isar1
get_isar1:
_get_isar1:
            mrs     x0, ID_AA64ISAR1_EL1
            ret

            .balign 16
            .global get_mmfr0, _get_mmfr0
get_mmfr0:
_get_mmfr0:
            mrs     x0, ID_AA64MMFR0_EL1
            ret

            .balign 16
            .global get_mmfr1, _get_mmfr1
get_mmfr1:
_get_mmfr1:
            mrs     x0, ID_AA64MMFR1_EL1
            ret

            .balign 16
            .global get_pfr0, _get_pfr0
get_pfr0:
_get_pfr0:
            mrs     x0, ID_AA64PFR0_EL1
            ret

            .balign 16
            .global get_pfr1, _get_pfr1
get_pfr1:
_get_pfr1:
            mrs     x0, ID_AA64PFR1_EL1
            ret

            .balign 16
            .global get_dfr0, _get_dfr0
get_dfr0:
_get_dfr0:
            mrs     x0, ID_AA64DFR0_EL1
            ret

            .balign 16
            .global get_dfr1, _get_dfr1
get_dfr1:
_get_dfr1:
            mrs     x0, ID_AA64DFR1_EL1
            ret
