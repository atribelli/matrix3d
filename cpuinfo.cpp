// cpuinfo.cpp
// Assuming C++17

#include <cstdint>

#if defined(__x86_64__)                     // 64-bit Intel macOS or Linux
#include <cpuid.h>
#elif defined(_M_X64)                       // 64-bit Intel Windows
#include <intrin.h>
#endif

#include "cpuinfo.h"



// -------------------------------------------------------------------------
// Return values from the cpuid instruction

#if defined(__x86_64__) || defined(_M_X64)  // 64-bit Intel

union {
    struct {
        uint32_t eax, ebx, ecx, edx;
    };
    int32_t regs[4];
} cpu;

#endif

// -------------------------------------------------------------------------
// Make sure we have the proper level of CPU functionality

bool is_cpu_gen_4(void) {
    // Intel 4th gen (Haswell)
    // https://www.intel.com/content/dam/develop/external/us/en/documents/how-to-detect-new-instruction-support-in-the-4th-generation-intel-core-processor-family.pdf

#if defined(__x86_64__)                     // 64-bit Intel macOS or Linux
    
    // Check level of CPUID support
    if (! __get_cpuid(0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx))
        return false;
    if (cpu.eax < 7)
        return false;
    if (! __get_cpuid(0x80000000, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx))
        return false;
    if (cpu.eax < 0x80000001)
        return false;

    // Check features

    // EAX 1 ECX 0
    if (! __get_cpuid_count(1, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx))
        return false;
    
    // FMA
    if ((cpu.ecx & (1 << 12)) == 0)
        return false;

    // MOVBE
    if ((cpu.ecx & (1 << 22)) == 0)
        return false;

    // OSXSAVE
    if ((cpu.ecx & (1 << 27)) == 0)
        return false;

    // EAX 7 ECX 0
    if (! __get_cpuid_count(7, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx))
        return false;

    // AVX2
    if ((cpu.ebx & (1 << 5)) == 0)
        return false;

    // BMI
    if ((cpu.ebx & (1 << 3)) == 0)
        return false;
    if ((cpu.ebx & (1 << 8)) == 0)
        return false;

    // EAX 0x80000001 ECX 0
    if (! __get_cpuid_count(0x80000001, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx))
        return false;

    // LZCNT
    if ((cpu.ecx & (1 << 5)) == 0)
        return false;
    
    return true;

#elif defined(_M_X64)                       // 64-bit Intel Windows

    // Check level of CPUID support
    __cpuid(cpu.regs, 0);
    if (cpu.eax < 7)
        return false;
    __cpuid(cpu.regs, 0x80000000);
    if (cpu.eax < 0x80000001)
        return false;

    // Check features

    // EAX 1 ECX 0
    __cpuidex(cpu.regs, 1, 0);

    // FMA
    if ((cpu.ecx & (1 << 12)) == 0)
        return false;

    // MOVBE
    if ((cpu.ecx & (1 << 22)) == 0)
        return false;

    // OSXSAVE
    if ((cpu.ecx & (1 << 27)) == 0)
        return false;

    // EAX 7 ECX 0
    __cpuidex(cpu.regs, 7, 0);

    // AVX2
    if ((cpu.ebx & (1 << 5)) == 0)
        return false;

    // BMI
    if ((cpu.ebx & (1 << 3)) == 0)
        return false;
    if ((cpu.ebx & (1 << 8)) == 0)
        return false;

    // EAX 0x80000001 ECX 0
    __cpuidex(cpu.regs, 0x80000001, 0);

    // LZCNT
    if ((cpu.ecx & (1 << 5)) == 0)
        return false;
    
    return true;

#endif

    return false;
}



// -------------------------------------------------------------------------
// Identify the CPU

bool get_cpu_vendor(char *buffer, size_t len) {
    bool valid = false;
    
    if (buffer != nullptr && len >= 13) {

#if defined(__x86_64__)     // 64-bit Intel macOS or Linux
        
        if (__get_cpuid (0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx)) {
            *((uint32_t *) &buffer[ 0]) = cpu.ebx;
            *((uint32_t *) &buffer[ 4]) = cpu.edx;
            *((uint32_t *) &buffer[ 8]) = cpu.ecx;
                            buffer[12]  = 0;
            
            valid = true;
        }
    
#elif defined(_M_X64)       // 64-bit Intel Windows

        __cpuid(cpu.regs, 0);
        *((uint32_t *) &buffer[ 0]) = cpu.ebx;
        *((uint32_t *) &buffer[ 4]) = cpu.edx;
        *((uint32_t *) &buffer[ 8]) = cpu.ecx;
                        buffer[12]  = 0;
        
        valid = true;

#endif
        
    }

    return valid;
}

bool get_cpu_brand(char *buffer, size_t len) {
    bool valid = false;
    
    if (buffer != nullptr && len >= 49) {

#if defined(__x86_64__)     // 64-bit Intel macOS or Linux

        if (   __get_cpuid (0x80000000, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx)
            && ((cpu.eax & 0x80000000) && (cpu.eax >= 0x80000004))) {
            __get_cpuid (0x80000002, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
            *((uint32_t *) &buffer[ 0]) = cpu.eax;
            *((uint32_t *) &buffer[ 4]) = cpu.ebx;
            *((uint32_t *) &buffer[ 8]) = cpu.ecx;
            *((uint32_t *) &buffer[12]) = cpu.edx;
            __get_cpuid (0x80000003, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
            *((uint32_t *) &buffer[16]) = cpu.eax;
            *((uint32_t *) &buffer[20]) = cpu.ebx;
            *((uint32_t *) &buffer[24]) = cpu.ecx;
            *((uint32_t *) &buffer[28]) = cpu.edx;
            __get_cpuid (0x80000004, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
            *((uint32_t *) &buffer[32]) = cpu.eax;
            *((uint32_t *) &buffer[36]) = cpu.ebx;
            *((uint32_t *) &buffer[40]) = cpu.ecx;
            *((uint32_t *) &buffer[44]) = cpu.edx;
                            buffer[48]  = 0;
            
            valid = true;
        }

#elif defined(_M_X64)       // 64-bit Intel Windows

        __cpuid(cpu.regs, 0x80000000);
        if ((cpu.eax & 0x80000000) && (cpu.eax >= 0x80000004)) {
            __cpuid(cpu.regs, 0x80000002);
            *((uint32_t *) &buffer[ 0]) = cpu.eax;
            *((uint32_t *) &buffer[ 4]) = cpu.ebx;
            *((uint32_t *) &buffer[ 8]) = cpu.ecx;
            *((uint32_t *) &buffer[12]) = cpu.edx;
            __cpuid(cpu.regs, 0x80000003);
            *((uint32_t *) &buffer[16]) = cpu.eax;
            *((uint32_t *) &buffer[20]) = cpu.ebx;
            *((uint32_t *) &buffer[24]) = cpu.ecx;
            *((uint32_t *) &buffer[28]) = cpu.edx;
            __cpuid(cpu.regs, 0x80000004);
            *((uint32_t *) &buffer[32]) = cpu.eax;
            *((uint32_t *) &buffer[36]) = cpu.ebx;
            *((uint32_t *) &buffer[40]) = cpu.ecx;
            *((uint32_t *) &buffer[44]) = cpu.edx;
                            buffer[48]  = 0;
            
            valid = true;
        }

#endif
        
    }

    return valid;
}
