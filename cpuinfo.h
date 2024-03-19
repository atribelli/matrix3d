// cpuinfo.h

#ifndef cpuinfo_h
#define cpuinfo_h

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

    bool is_cpu_gen_4   (void);
    bool get_cpu_vendor (char *buffer, size_t len);
    bool get_cpu_brand  (char *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // cpuinfo_h
