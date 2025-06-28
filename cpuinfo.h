// cpuinfo.h

#ifndef cpuinfo_h
#define cpuinfo_h

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    bool cpu_has_avx512_ifma_vbmi (void);
    bool cpu_has_avx512_vl_dq_bw  (void);
    bool cpu_has_avx512_er_pf     (void);
    bool cpu_has_avx512_f_cd      (void);
    bool is_cpu_gen_4             (void);
    bool cpu_has_avx2             (void);
    bool cpu_has_avx              (void);
    bool cpu_has_sse4_2           (void);
    bool cpu_has_sse3             (void);
    bool get_cpu_vendor           (char *buffer, size_t len);
    bool get_cpu_brand            (char *buffer, size_t len);
    bool get_cpu_part             (char *buffer, size_t len);
    bool get_cpu_cores            (char *buffer, size_t len);
    bool get_cpu_features         (char *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // cpuinfo_h
