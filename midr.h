// midr.h

#ifndef midr_h
#define midr_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    uint64_t get_midr   (void);
    uint64_t get_mpidr  (void);
    uint64_t get_revidr (void);
    uint64_t get_isar0  (void);
    uint64_t get_isar1  (void);
    uint64_t get_mmfr0  (void);
    uint64_t get_mmfr1  (void);
    uint64_t get_pfr0   (void);
    uint64_t get_pfr1   (void);
    uint64_t get_dfr0   (void);
    uint64_t get_dfr1   (void);

#ifdef __cplusplus
}
#endif

#endif  // midr_h
