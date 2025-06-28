// cpuid.c

#include <stdio.h>

#include "cpuinfo.h"

int main (void) {
    char buffer[2048];

    if (get_cpu_vendor(buffer, sizeof(buffer))) {
        printf("Vendor:   %s\n", buffer);
    }
    if (get_cpu_brand(buffer, sizeof(buffer))) {
        printf("Brand:    %s\n", buffer);
    }
    if (get_cpu_part(buffer, sizeof(buffer))) {
        printf("Part:     %s\n", buffer);
    }
    if (get_cpu_cores(buffer, sizeof(buffer))) {
        printf("Cores:    %s\n", buffer);
    }
    if (get_cpu_features(buffer, sizeof(buffer))) {
        printf("Features: %s\n", buffer);
    }

    return 0;
}
