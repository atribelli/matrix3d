; midr.asm

            area    .text, code
            align   4



;------------------------------------------------------------------------------
; Get the MIDR register
; Return:
;   X0  MIDR

            align   8
            global  get_midr
get_midr
            mrs     x0, MIDR_EL1
            ret

            align   8
            global  get_isar0
get_isar0
            mrs     x0, ID_AA64ISAR0_EL1
            ret

            align   8
            global  get_isar1
get_isar1
            mrs     x0, ID_AA64ISAR1_EL1
            ret

            align   8
            global  get_pfr0
get_pfr0
            mrs     x0, ID_AA64PFR0_EL1
            ret

            end
