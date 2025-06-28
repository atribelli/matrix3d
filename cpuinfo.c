// cpuinfo.c
//
// Getting CPU info using the Windows API
//  Vendor
//      X64     __cpuid(cpu.regs, 0);
//      ARM64   ?
//  Brand
//      X64     __cpuidex(cpu.regs, 0x80000002, 0);
//              __cpuidex(cpu.regs, 0x80000003, 0);
//              __cpuidex(cpu.regs, 0x80000004, 0);
//      ARM64   ?
//  Part
//      X64     __cpuid(cpu.regs, 1);
//      ARM64   GetNativeSystemInfo(&info);
//              IsProcessorFeaturePresent(PF_ARM_...);
//  Cores
//      X64     GetNativeSystemInfo(&info);
//      ARM64   GetNativeSystemInfo(&info);
//  Features
//      X64     __cpuidex(cpu.regs, eax_r, ecx_r);
//      ARM64   IsProcessorFeaturePresent(PF_ARM_...);
//
// Getting CPU info using macOS's sysctlbyname()
//  Vendor
//      X64     __get_cpuid(0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//              sysctlbyname("machdep.cpu.vendor", buffer, &size, NULL, 0);
//      ARM64   ?
//  Brand
//      X64     __get_cpuid_count(0x80000002, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//              __get_cpuid_count(0x80000003, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//              __get_cpuid_count(0x80000004, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//      ARM64   sysctlbyname("machdep.cpu.brand_string", buffer, &size, NULL, 0);
//  Part
//      X64     __get_cpuid(1, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//      ARM64   ?
//  Cores
//      X64     sysctlbyname("machdep.cpu.core_count", &cores,  &size, NULL, 0)
//      ARM64   sysctlbyname("machdep.cpu.core_count", &cores,  &size, NULL, 0)
//  Features
//      X64     __get_cpuid_count(eax_r, ecx_r, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx)
//              sysctlbyname("machdep.cpu.features", buffer, &size, NULL, 0);
//      ARM64   sysctlbyname("hw.optional.neon", &ret, &size, NULL, 0);
//              sysctlbyname("hw.nperflevels", &ret, &size, NULL, 0);
//              sysctlbyname(hw.perflevel%d.name, buf, &size, NULL, 0);
//              sysctlbyname(hw.perflevel%d.physicalcpu, buf, &size, NULL, 0);
//
// Getting CPU info using Linux's /proc/cpuinfo
//  Vendor
//      X64     __get_cpuid(0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx)
//              get_proc_cpuinfo_entry(fp, "vendor_id");
//      ARM64   mrs x0, MIDR_EL1
//              get_proc_cpuinfo_entry(fp, "CPU implementer");
//      ARM32   get_proc_cpuinfo_entry(fp, "CPU implementer");
//  Brand
//      X64     __get_cpuid_count(0x80000002, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//              __get_cpuid_count(0x80000003, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//              __get_cpuid_count(0x80000004, 0, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//      ARM64   get_proc_cpuinfo_entry(fp, "Model");
//      ARM32   get_proc_cpuinfo_entry(fp, "Model");
//  Part
//      X64     __get_cpuid(1, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//      ARM64   mrs x0, MIDR_EL1
//      ARM32   get_proc_cpuinfo_entry(fp, "CPU implementer");
//              get_proc_cpuinfo_entry(fp, "CPU part");
//  Cores
//      X64     get_proc_cpuinfo_entry(fp, "cpu cores");
//      ARM64   get_proc_cpuinfo_entry(fp, "processor");
//      ARM32   get_proc_cpuinfo_entry(fp, "processor");
//  Features
//      X64     __get_cpuid_count(eax_r, ecx_r, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx);
//              get_proc_cpuinfo_entry(fp, "flags");
//      ARM64   get_proc_cpuinfo_entry(fp, "Features");
//      ARM32   get_proc_cpuinfo_entry(fp, "Features");

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpuinfo.h"
#include "midr.h"

#if defined(_M_X64)                         // 64-bit Intel Windows
#include <intrin.h>
#elif defined(__x86_64__)                   // 64-bit Intel macOS or Linux
#include <cpuid.h>
#endif

#if defined(_WIN64)                         // Windows
#include <windows.h>
#endif

#if defined(__APPLE__)                      // macOS
#include <sys/sysctl.h>
#endif



// ----------------------------------------------------------------------------
// Constants for conditional compilation

#if defined(_M_X64) || defined(__x86_64__)  // 64-bit Intel
#define ANY_X64 1
#else
#define ANY_X64 0
#endif

#if defined(_M_ARM64) || defined(__aarch64__) // 64-bit ARM
#define ANY_ARM64 1
#else
#define ANY_ARM64 0
#endif

#if defined(_M_ARM64) || defined(__aarch64__) || defined(__arm__) // 64- or 32-bit ARM
#define ANY_ARM 1
#else
#define ANY_ARM 0
#endif

#if (defined(__x86_64__) || defined(__aarch64__)) && defined(__linux__) // 64-bit Linux
#define LINUX_64 1
#else
#define LINUX_64 0
#endif

#if defined(__arm__) && defined(__linux__)  // 32-bit Linux
#define LINUX_32 1
#else
#define LINUX_32 0
#endif



// ----------------------------------------------------------------------------
// Number of elements in an array

#define elementsof(array) (sizeof(array) / sizeof(array[0]))



// ----------------------------------------------------------------------------
// Let's be paranoid about string termination.
// Note that len must not be zero.

#define strterm(buf, len) ((buf)[(len)       - 1] = 0)
#define bufterm(buf)      ((buf)[sizeof(buf) - 1] = 0)



#if ANY_ARM

// ----------------------------------------------------------------------------
// Lookup table for ARM implementers and parts.
// Use -1 to indicate no entry. For example a -1 for the part number field
// indicates an implementer name, otherwise it is a part name.
// https://github.com/bp0/armids

static struct {
    int         implementer;
    int         partnum;
    const char *name;
} part_info[] = {
    0x41,    -1, "ARM",                     // Implementer
    0x41, 0x810, "ARM810",                  // Parts ...
    0x41, 0x920, "ARM920",
    0x41, 0x922, "ARM922",
    0x41, 0x926, "ARM926",
    0x41, 0x940, "ARM940",
    0x41, 0x946, "ARM946",
    0x41, 0x966, "ARM966",
    0x41, 0xa20, "ARM1020",
    0x41, 0xa22, "ARM1022",
    0x41, 0xa26, "ARM1026",
    0x41, 0xb02, "ARM11 MPCore",
    0x41, 0xb36, "ARM1136",
    0x41, 0xb56, "ARM1156",
    0x41, 0xb76, "ARM1176",
    0x41, 0xc05, "Cortex-A5",
    0x41, 0xc07, "Cortex-A7",
    0x41, 0xc08, "Cortex-A8",
    0x41, 0xc09, "Cortex-A9",
    0x41, 0xc0d, "Cortex-A12/A17",          // Originally A12
    0x41, 0xc0f, "Cortex-A15",
    0x41, 0xc0e, "Cortex-A17",
    0x41, 0xc14, "Cortex-R4",
    0x41, 0xc15, "Cortex-R5",
    0x41, 0xc17, "Cortex-R7",
    0x41, 0xc18, "Cortex-R8",
    0x41, 0xc20, "Cortex-M0",
    0x41, 0xc21, "Cortex-M1",
    0x41, 0xc23, "Cortex-M3",
    0x41, 0xc24, "Cortex-M4",
    0x41, 0xc27, "Cortex-M7",
    0x41, 0xc60, "Cortex-M0+",
    0x41, 0xd01, "Cortex-A32",
    0x41, 0xd03, "Cortex-A53",
    0x41, 0xd04, "Cortex-A35",
    0x41, 0xd05, "Cortex-A55",
    0x41, 0xd07, "Cortex-A57",
    0x41, 0xd08, "Cortex-A72",
    0x41, 0xd09, "Cortex-A73",
    0x41, 0xd0a, "Cortex-A75",
    0x41, 0xd0b, "Cortex-A76",
    0x41, 0xd0c, "Neoverse-N1",
    0x41, 0xd0d, "Cortex-A77",
    0x41, 0xd13, "Cortex-R52",
    0x41, 0xd20, "Cortex-M23",
    0x41, 0xd21, "Cortex-M33",
    0x41, 0xd4a, "Neoverse-E1",
    0x42, -1,    "Broadcom",
    0x42, 0x00f, "Brahma B15",
    0x42, 0x100, "Brahma B53",
    0x42, 0x516, "ThunderX2",
    0x43, -1,    "Cavium",
    0x43, 0x0a0, "ThunderX",
    0x43, 0x0a1, "ThunderX 88XX",
    0x43, 0x0a2, "ThunderX 81XX",
    0x43, 0x0a3, "ThunderX 83XX",
    0x43, 0x0af, "ThunderX2 99xx",
    0x44, -1,    "DEC",
    0x44, 0xa10, "SA110",
    0x44, 0xa11, "SA1100",
    0x4e, -1,    "nVidia",
    0x4e, 0x000, "Denver",
    0x4e, 0x003, "Denver 2",
    0x50, -1,    "APM",
    0x50, 0x000, "X-Gene",
    0x51, -1,    "Qualcomm",
    0x4e, 0x00f, "Scorpion",
    0x4e, 0x02d, "Scorpion",
    0x4e, 0x04d, "Krait",
    0x4e, 0x06f, "Krait",
    0x4e, 0x201, "Kryo",
    0x4e, 0x205, "Kryo",
    0x4e, 0x211, "Kryo",
    0x4e, 0x800, "Falkor V1/Kryo",
    0x4e, 0x801, "Kryo V2",
    0x4e, 0x802, "Kryo 3xx gold",
    0x4e, 0x803, "Kryo 3xx silver",
    0x4e, 0x805, "Kryo 5xx silver",
    0x4e, 0xc00, "Falkor",
    0x4e, 0xc01, "Saphira",
    0x53, -1,    "Samsung",
    0x53, 0x001, "exynos-m1",
    0x54, -1,    "Texas Instruments",
    0x56, -1,    "Marvell",
    0x56, 0x131, "Feroceon 88FR131",
    0x56, 0x581, "PJ4/PJ4b",
    0x56, 0x584, "PJ4B-MP",
    0x66, -1,    "Faraday",
    0x66, 0x526, "FA526",
    0x66, 0x626, "FA626",
    0x69, -1,    "Intel",
    0x69, 0x200, "i80200",
    0x69, 0x210, "PXA250A",
    0x69, 0x212, "PXA210A",
    0x69, 0x242, "i80321-400",
    0x69, 0x243, "i80321-600",
    0x69, 0x290, "PXA250B/PXA26x",
    0x69, 0x292, "PXA210B",
    0x69, 0x2c2, "i80321-400-B0",
    0x69, 0x2c3, "i80321-600-B0",
    0x69, 0x2d0, "PXA250C/PXA255/PXA26x",
    0x69, 0x2d2, "PXA210C",
    0x69, 0x2e3, "i80219",
    0x69, 0x411, "PXA27x",
    0x69, 0x41c, "IPX425-533",
    0x69, 0x41d, "IPX425-400",
    0x69, 0x41f, "IPX425-266",
    0x69, 0x682, "PXA32x",
    0x69, 0x683, "PXA930/PXA935",
    0x69, 0x688, "PXA30x",
    0x69, 0x689, "PXA31x",
    0x69, 0xb11, "SA1110",
    0x69, 0xc12, "IPX1200"
};

static int get_part_info_index(int implementer, int partnum) {
    for (int i = 0; i < elementsof(part_info); ++i ) {
        // Return if we have a match
        if (   part_info[i].implementer == implementer
            && part_info[i].partnum     == partnum) {
            return i;
        }
    }

    // Indicate no match
    return -1;
}

static struct {
    int         architecture;
    const char *name;
} arch_info[] = {
    0b0001, "Armv4",
    0b0010, "Armv4T",
    0b0011, "Armv5",
    0b0100, "Armv5T",
    0b0101, "Armv5TE",
    0b0110, "Armv5TEJ",
    0b0111, "Armv6"
};

static int get_architecture_index(int architecture) {
    for (int i = 0; i < elementsof(arch_info); ++i ) {
        // Return if we have a match
        if (arch_info[i].architecture == architecture) {
            return i;
        }
    }

    // Indicate no match
    return -1;
}

#endif // ANY_ARM



#if ANY_X64

// ----------------------------------------------------------------------------
// Return values from the cpuid instruction.
// Create a union so that regardless of API we can always access
// individual members as cpu.eax, etc.

typedef union {
    struct {
        uint32_t eax, ebx, ecx, edx;
    };
    int32_t regs[4];
} cpu_regs;



// ----------------------------------------------------------------------------
// Make sure we have the proper level of CPU functionality

static bool has_cpuid_level(int eax, int level) {
    cpu_regs cpu;
    
#if defined(_M_X64)                         // 64-bit Intel Windows
    
    __cpuid(cpu.regs, eax);

#elif defined(__x86_64__)                   // 64-bit Intel macOS or Linux
    
    if (! __get_cpuid(eax, &cpu.eax, &cpu.ebx, &cpu.ecx, &cpu.edx))
        return false;

#endif
    
    // Check level of CPUID support
    return cpu.eax >= level;
}



// ----------------------------------------------------------------------------
// Get features from CPUID using the EAX ECX pair

static bool get_cpu_functionality(cpu_regs *cpu, int eax, int ecx) {
    if (cpu == NULL)
        return false;

#if defined(_M_X64)                         // 64-bit Intel Windows

    __cpuidex(cpu->regs, eax, ecx);

#elif defined(__x86_64__)                   // 64-bit Intel macOS or Linux

    if (! __get_cpuid_count(eax, ecx,
                            &cpu->eax, &cpu->ebx, &cpu->ecx, &cpu->edx))
        return false;

#endif

    return true;
}

#endif // ANY_X64



// ----------------------------------------------------------------------------
// Check CPU functionality
//
// https://en.wikipedia.org/wiki/AVX-512
// https://en.wikipedia.org/wiki/CPUID

bool cpu_has_avx512_ifma_vbmi(void) {
#if ANY_X64
    
    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 7))
        return false;
    
    // Check features
    
    // EAX 7 ECX 0
    if (! get_cpu_functionality(&cpu, 7, 0))
        return false;
    
    // AVX-512 Integer Fused Multiply Add
    if ((cpu.ebx & (1 << 21)) == 0)
        return false;

    // AVX-512 Vector Byte Manipulation Instructions
    if ((cpu.ecx & (1 << 1)) == 0)
        return false;
    
    // Check prerequisites
    return cpu_has_avx512_f_cd();

#endif

    return false;
}

bool cpu_has_avx512_vl_dq_bw(void) {
#if ANY_X64
    
    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 7))
        return false;
    
    // Check features
    
    // EAX 7 ECX 0
    if (! get_cpu_functionality(&cpu, 7, 0))
        return false;
    
    // AVX-512 Vector Length Extensions
    if ((cpu.ebx & (1 << 31)) == 0)
        return false;

    // AVX-512 Doubleword and Quadword Instructions
    if ((cpu.ebx & (1 << 17)) == 0)
        return false;
    
    // AVX-512 Byte and Word Instructions
    if ((cpu.ebx & (1 << 30)) == 0)
        return false;
    
    // Check prerequisites
    return cpu_has_avx512_f_cd();

#endif

    return false;
}

bool cpu_has_avx512_er_pf(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 7))
        return false;
    
    // Check features
    
    // EAX 7 ECX 0
    if (! get_cpu_functionality(&cpu, 7, 0))
        return false;
    
    // AVX-512 Exponential and Reciprocal Instructions
    if ((cpu.ebx & (1 << 27)) == 0)
        return false;
    
    // AVX-512 Prefetch Instructions
    if ((cpu.ebx & (1 << 26)) == 0)
        return false;
    
    // Check prerequisites
    return cpu_has_avx512_f_cd();

#endif

    return false;
}

bool cpu_has_avx512_f_cd(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 7))
        return false;

    // Check features

    // EAX 7 ECX 0
    if (! get_cpu_functionality(&cpu, 7, 0))
        return false;

    // AVX-512 Foundation
    if ((cpu.ebx & (1 << 16)) == 0)
        return false;

    // AVX-512 Conflict Detection Instructions
    if ((cpu.ebx & (1 << 28)) == 0)
        return false;

    // Check prerequisites
    return is_cpu_gen_4();

#endif

    return false;
}

bool is_cpu_gen_4(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Intel 4th gen (Haswell)
    // https://www.intel.com/content/dam/develop/external/us/en/documents/how-to-detect-new-instruction-support-in-the-4th-generation-intel-core-processor-family.pdf

    // Check level of CPUID support
    if (! has_cpuid_level(0, 7))
        return false;
    if (! has_cpuid_level(0x80000000, 0x80000001))
        return false;

    // Check features

    // EAX 1 ECX 0
    if (! get_cpu_functionality(&cpu, 1, 0))
        return false;

    // FMA3
    if ((cpu.ecx & (1 << 12)) == 0)
        return false;

    // MOVBE
    if ((cpu.ecx & (1 << 22)) == 0)
        return false;

    // OSXSAVE
    if ((cpu.ecx & (1 << 27)) == 0)
        return false;

    // EAX 7 ECX 0
    if (! get_cpu_functionality(&cpu, 7, 0))
        return false;

    // BMI
    if ((cpu.ebx & (1 << 3)) == 0)
        return false;
    if ((cpu.ebx & (1 << 8)) == 0)
        return false;

    // EAX 0x80000001 ECX 0
    if (! get_cpu_functionality(&cpu, 0x80000001, 0))
        return false;

    // LZCNT
    if ((cpu.ecx & (1 << 5)) == 0)
        return false;
    
    // Check prerequisites
    return cpu_has_avx2();

#endif

    return false;
}

bool cpu_has_avx2(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 7))
        return false;

    // Check features

    // EAX 7 ECX 0
    if (! get_cpu_functionality(&cpu, 7, 0))
        return false;

    // AVX2
    if ((cpu.ebx & (1 << 5)) == 0)
        return false;

    // Check prerequisites
    return cpu_has_avx();

#endif

    return false;
}

bool cpu_has_avx(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 1))
        return false;

    // Check features

    // EAX 1 ECX 0
    if (! get_cpu_functionality(&cpu, 1, 0))
        return false;

    // AVX
    if ((cpu.ecx & (1 << 28)) == 0)
        return false;

    // Check prerequisites
    return cpu_has_sse4_2();

#endif

    return false;
}

bool cpu_has_sse4_2(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 1))
        return false;

    // Check features

    // EAX 1 ECX 0
    if (! get_cpu_functionality(&cpu, 1, 0))
        return false;

    // SSE4.2
    if ((cpu.ecx & (1 << 20)) == 0)
        return false;

    // SSE4.1
    if ((cpu.ecx & (1 << 19)) == 0)
        return false;

    // POPCNT
    if ((cpu.ecx & (1 << 23)) == 0)
        return false;

    // AESNI
    if ((cpu.ecx & (1 << 25)) == 0)
        return false;

    // PCLMULQDQ
    if ((cpu.ecx & (1 << 1)) == 0)
        return false;

    // Check prerequisites
    return cpu_has_sse3();

#endif

    return false;
}

bool cpu_has_sse3(void) {
#if ANY_X64

    cpu_regs cpu;
    
    // Check level of CPUID support
    if (! has_cpuid_level(0, 1))
        return false;

    // Check features

    // EAX 1 ECX 0
    if (! get_cpu_functionality(&cpu, 1, 0))
        return false;

    // SSSE3
    if ((cpu.ecx & (1 << 9)) == 0)
        return false;

    // SSE3
    if ((cpu.ecx & (1 << 0)) == 0)
        return false;

    // SSE2
    if ((cpu.edx & (1 << 26)) == 0)
        return false;

    // SSE
    if ((cpu.edx & (1 << 25)) == 0)
        return false;

    // MMX
    if ((cpu.edx & (1 << 23)) == 0)
        return false;

    // Check prerequisites
    return true;

#endif

    return false;
}



// ----------------------------------------------------------------------------
// Get CPU info using Linux's /proc/cpuinfo
//
// Note that the FILE * is handled outside of these functions.
// This allows multple calls to continue the search from the last
// match. Unless the caller uses fseek().
//
// Note the use of a static char array, this code is not thread safe

#if defined(__linux__)                      // Linux

static char *get_proc_cpuinfo_entry(FILE *fp, char *entry) {
    if (fp != NULL && entry != NULL) {
        static char buf[2048];              // Not thread safe
        
        // Read next line
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            size_t size = strlen(entry);

            // See if line matches the entry we are looking for
            if (strncmp(buf, entry, size) == 0) {
                char *separator = strchr(buf, ':');
                
                // Make sure we have a separator
                if (     separator      != NULL
                    && *(separator + 1) != 0
                    && *(separator + 2) != 0) {
                    // Move to the data
                    separator += 2;
                    
                    // Remove the newline
                    separator[strlen(separator) - 1] = 0;
                    
                    // Return the string
                    return separator;
                }
            }
        }
    }

    // Indicate no match
    return NULL;
}

static int get_proc_cpuinfo_value(FILE *fp, char *entry, int base, int error) {
    if (fp != NULL && entry != NULL) {
        char *result = get_proc_cpuinfo_entry(fp, entry);

        // Make sure we have a string
        if (result != NULL && *result != 0) {
            long int value;
            char     *after;
            
            // Attempt a conversion
            value = strtol(result, &after, base);
            
            // Fail if no numeric digits were found.
            // Note we are allowing non numeric characters to follow.
            return (result == after) ? error : value;
        }
    }
    
    // Let caller specify the error code.
    // We don't know what the valid values are for this entry.
    return error;
}

#endif



// ----------------------------------------------------------------------------
// Identify the CPU vendor

#if ANY_X64

static bool get_cpu_vendor_intel(char *buffer, size_t len) {
    cpu_regs cpu;
    
    // String length is known
    if (len < 13)
        return false;
    
    // cpuid eax 0
    // 12-character string in ebx, edx, ecx
    if (! get_cpu_functionality(&cpu, 0, 0))
        return false;
    
    *((uint32_t*)&buffer[0]) = cpu.ebx;
    *((uint32_t*)&buffer[4]) = cpu.edx;
    *((uint32_t*)&buffer[8]) = cpu.ecx;
    buffer[12]               = 0;
    
    return true;
}

#endif



#if ANY_ARM

static bool get_cpu_vendor_arm(char *buffer, size_t len, int implementer) {
    if (implementer > 0) {
        // Convert implementer value to a table index
        implementer = get_part_info_index(implementer, -1);
        if (implementer > -1) {
            return   snprintf(buffer, len, "%s", part_info[implementer].name)
                   > 0;
        }
    }
    
    return false;
}

#endif



static bool verify_buffer(char *buffer, size_t len) {
    // Make sure that we have a buffer
    if (buffer == NULL || len == 0)
        return false;

    // Failsafe to a null string
    buffer[0] = 0;
    
    return true;
}



bool get_cpu_vendor(char *buffer, size_t len) {
    if (! verify_buffer(buffer, len))
        return false;
    
#if ANY_X64

    return get_cpu_vendor_intel(buffer, len);

#elif ANY_ARM64 && LINUX_64

    // mrs x0, MIDR_EL1
    // Implementer, bits [31:24]
    return get_cpu_vendor_arm(buffer, len, (get_midr() >> 24) & 0xff);

#elif LINUX_32
                               
    bool  success = false;
    FILE *fp      = fopen("/proc/cpuinfo", "r");

    if (fp != NULL) {
        int implementer = get_proc_cpuinfo_value(fp, "CPU implementer",
                                                 16, -1);
        
        success = get_cpu_vendor_arm(buffer, len, implementer);
        
        fclose(fp);
    }

    return success;

#endif

    return false;
}



// ----------------------------------------------------------------------------
// Identify the CPU brand

#if ANY_X64

static bool get_cpu_brand_intel(char *buffer, size_t len) {
    cpu_regs cpu;
    
    // String length is known
    if (len < 49)
        return false;

    // Check level of CPUID support
    if (! has_cpuid_level(0x80000000, 0x80000004))
        return false;

    // cpuid eax 0x80000002 ecx 0
    // 16-character string in eax ebx, ecx, edx
    if (! get_cpu_functionality(&cpu, 0x80000002, 0))
        return false;

    *((uint32_t*)&buffer[0])  = cpu.eax;
    *((uint32_t*)&buffer[4])  = cpu.ebx;
    *((uint32_t*)&buffer[8])  = cpu.ecx;
    *((uint32_t*)&buffer[12]) = cpu.edx;

    if (! get_cpu_functionality(&cpu, 0x80000003, 0))
        return false;

    *((uint32_t*)&buffer[16]) = cpu.eax;
    *((uint32_t*)&buffer[20]) = cpu.ebx;
    *((uint32_t*)&buffer[24]) = cpu.ecx;
    *((uint32_t*)&buffer[28]) = cpu.edx;

    if (! get_cpu_functionality(&cpu, 0x80000004, 0))
        return false;

    *((uint32_t*)&buffer[32]) = cpu.eax;
    *((uint32_t*)&buffer[36]) = cpu.ebx;
    *((uint32_t*)&buffer[40]) = cpu.ecx;
    *((uint32_t*)&buffer[44]) = cpu.edx;
    buffer[48]                = 0;

    return true;
}

#endif



bool get_cpu_brand(char *buffer, size_t len) {
    if (! verify_buffer(buffer, len))
        return false;

#if ANY_X64

    return get_cpu_brand_intel(buffer, len);

#elif defined(__APPLE__)                    // macOS

    size_t size = len;
    if (   sysctlbyname("machdep.cpu.brand_string", buffer, &size, NULL, 0)
        == 0) {
        strterm(buffer, len);
        
        return true;
    }
    
#elif defined(__linux__)                    // Linux
    
    bool  success = false;
    FILE *fp      = fopen("/proc/cpuinfo", "r");
    
    if (fp != NULL) {
        char *model = get_proc_cpuinfo_entry(fp, "Model");

        if (model != NULL) {
            success = snprintf(buffer, len, "%s", model) > 0;
        }

        fclose(fp);
    }
    
    return success;

#endif
    
    return false;
}



// ----------------------------------------------------------------------------
// Identify the CPU part

#if ANY_X64

// https://en.wikipedia.org/wiki/CPUID

static uint32_t get_cpu_family(uint32_t family, uint32_t exfamily) {
    if (family == 15)
        family += exfamily;

    return family;
}

static uint32_t get_cpu_model(uint32_t model, uint32_t exmodel,
                              uint32_t family) {
    if (family == 6 || family == 15)
        model += exmodel << 4;

    return model;
}

static bool get_cpu_part_intel(char *buffer, size_t len) {
    cpu_regs cpu;
    
    // Check level of CPUID support
    if (has_cpuid_level(0, 1)) {
        // cpuid eax 1
        // Model,             bits [7:4]
        // Family ID,         bits [11:8]
        // Extendd Model ID,  bits [19:16]
        // Extendd Family ID, bits [27:20]
        if (get_cpu_functionality(&cpu, 1, 0)) {
            int family   = (cpu.eax >>  8) & 0x0f;
            int exfamily = (cpu.eax >> 20) & 0xff;
            int model    = (cpu.eax >>  4) & 0x0f;
            int exmodel  = (cpu.eax >> 16) & 0x0f;
            
            return   snprintf(buffer, len, "Family %d Model %d",
                              get_cpu_family (family, exfamily),
                              get_cpu_model  (model,  exmodel, family))
                   > 0;
        }
    }
    
    return false;
}

#endif



#if ANY_ARM

static bool get_cpu_part_arm(char *buffer, size_t len,
                             int implementer, int partnum) {
    if (implementer > 0 && partnum > 0) {
        // Convert part number value to a table index
        partnum = get_part_info_index(implementer, partnum);
        if (partnum > -1) {
            return snprintf(buffer, len, "%s", part_info[partnum].name) > 0;
        }
    }
    
    return false;
}

#endif



bool get_cpu_part(char *buffer, size_t len) {
    if (! verify_buffer(buffer, len))
        return false;

#if ANY_X64
    
    return get_cpu_part_intel(buffer, len);
    
#elif defined(_WIN64)                       // Windows
    
    SYSTEM_INFO info;
    
    GetNativeSystemInfo(&info);
    if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) {
        bool v8 = IsProcessorFeaturePresent(PF_ARM_V8_INSTRUCTIONS_AVAILABLE);
        
        return snprintf(buffer, len, "%s", "Armv8") > 0;
    }
    
#elif defined(__APPLE__)                    // macOS
    
    static char features[2048] = "";
    int64_t     ret            = 0;
    size_t      size           = sizeof(ret);
    
    // ARM version is implied by available features
    if (   sysctlbyname("hw.optional.arm.FEAT_SME", &ret, &size, NULL, 0) == 0
        && ret == 1) {
        return snprintf(buffer, len, "%s", "Armv9") > 0;
    }
    else
        if (   sysctlbyname("hw.optional.arm64", &ret, &size, NULL, 0) == 0
             && ret == 1) {
        return snprintf(buffer, len, "%s", "Armv8") > 0;
    }

#elif ANY_ARM64 && LINUX_64

    // https://developer.arm.com/documentation/ddi0601/2025-03/AArch64-Registers/MIDR-EL1--Main-ID-Register
    //
    // mrs x0, MIDR_EL1
    // Implementer, bits [31:24]
    // PartNum,     bits [15:4]
    uint64_t midr = get_midr();
    
    return get_cpu_part_arm(buffer, len,
                            (midr >> 24) & 0x00ff,
                            (midr >>  4) & 0x0fff);
    
#elif LINUX_32

    bool  success = false;
    FILE *fp      = fopen("/proc/cpuinfo", "r");
    
    if (fp != NULL) {
        int implementer = get_proc_cpuinfo_value(fp, "CPU implementer",
                                                 16, -1),
            partnum     = get_proc_cpuinfo_value(fp, "CPU part",
                                                 16, -1);
        
        success = get_cpu_part_arm(buffer, len, implementer, partnum);
        
        fclose(fp);
    }
    
    return success;

#endif
    
    return false;
}



// ----------------------------------------------------------------------------
// Identify the CPU cores

bool get_cpu_cores(char *buffer, size_t len) {
    if (! verify_buffer(buffer, len))
        return false;

#if defined(_WIN64)                         // Windows
    
    SYSTEM_INFO info;
    
    GetNativeSystemInfo(&info);
    return snprintf(buffer, len, "%d-core", info.dwNumberOfProcessors) > 0;
    
#elif defined(__APPLE__)                    // macOS
    
    int64_t cores = 0;
    size_t  size  = sizeof(cores);
    
    if (sysctlbyname("machdep.cpu.core_count", &cores, &size, NULL, 0) == 0) {
        return snprintf(buffer, len, "%lld-Core", cores) > 0;
    }
    
#elif defined(__linux__)                    // Linux

    bool  success = false;
    FILE *fp      = fopen("/proc/cpuinfo", "r");
    
    if (fp != NULL) {
        long cores = -1;

#if defined(__x86_64__)                     // 64-bit Intel

        cores = get_proc_cpuinfo_value(fp, "cpu cores", 10, -1);

#elif defined(__arm__) || defined(__aarch64__) // 32- or 64-bit ARM

        // There is no explicit count field, so count off processors
        do {
            int value = get_proc_cpuinfo_value(fp, "processor", 10, -1);
            
            if (value >= 0) {
                cores = value + 1;
            }
            else {
                break;
            }
        } while (true);

#endif
        
        if (cores > 0) {
            success = snprintf(buffer, len, "%ld-Core", cores) > 0;
        }

        fclose(fp);
    }
    
    return success;

#endif

    return false;
}



// ----------------------------------------------------------------------------
// Identify the CPU features

#if ANY_X64

bool get_cpu_features_intel(char *buffer, size_t len) {
    static char features[2048] = "";

    if (cpu_has_sse3()) {
        strcat(features, "SSE3 ");
    }
    if (cpu_has_sse4_2()) {
        strcat(features, "SSE4.2 ");
    }
    if (cpu_has_avx()) {
        strcat(features, "AVX ");
    }
    if (cpu_has_avx2()) {
        strcat(features, "AVX2 ");
    }
    if (is_cpu_gen_4()) {
        strcat(features, "GEN4 ");
    }
    if (cpu_has_avx512_f_cd()) {
        strcat(features, "AVX512-F-CD ");
    }
    if (cpu_has_avx512_er_pf()) {
        strcat(features, "AVX512-ER-PF ");
    }
    if (cpu_has_avx512_vl_dq_bw()) {
        strcat(features, "AVX512-VL-DQ-BW ");
    }
    if (cpu_has_avx512_ifma_vbmi()) {
        strcat(features, "AVX512-IFMA-VBMI ");
    }
    
    bufterm(features);
    
    strncpy(buffer, features, len);
    strterm(buffer, len);

    return true;
}

#endif



#if ANY_ARM64 && LINUX_64

bool get_cpu_features_arm(char *buffer, size_t len) {
    static char features[2048] = "";

    // mrs x0, ID_AA64ISAR0_EL1
    // Dot Product, bits [47:44]
    uint64_t isar0 = get_isar0();
    if ((isar0 >> 44) & 0x0f) {
        strcat(features, "DP ");
    }

    // mrs x0, ID_AA64ISAR1_EL1
    //  Complex number addition and multiplication,, bits [19:16]
    uint64_t isar1 = get_isar1();
    if ((isar1 >> 16) & 0x0f) {
        strcat(features, "FCMA ");
    }

    // mrs x0, ID_AA64PFR0_EL1
    //  Scalable Vector Extension, bits [35:32]
    //  Advanced SIMD,             bits [23:20]
    //  Floating Point,            bits [19:16]
    uint64_t pfr0 = get_pfr0();
    if ((pfr0 >> 32) & 0x0f) {
        strcat(features, "SVE ");
    }
    if ((pfr0 >> 20) & 0x0f) {
        strcat(features, "AdvSIMD ");
    }
    if ((pfr0 >> 16) & 0x0f) {
        strcat(features, "FP ");
    }

    bufterm(features);
    
    strncpy(buffer, features, len);
    strterm(buffer, len);

    return true;
}

#endif



#if defined(__APPLE__)                      // macOS

static bool append_cpu_core_entry(char *features,
                                  char *entry,
                                  int   i,
                                  bool  text) {
    // Make sure we have a destination and a name to lookup
    if (features == NULL || entry == NULL)
        return false;
    
    char   name[64];
    char   buffer[32];
    size_t size;

    // We need to construct a sysctl name that incorporates
    // a count. There should be a "%d" in the entry string.
    if (snprintf(name, sizeof(name), entry, i) > 0) {
        // Handle a text based entry
        if (text) {
            size = sizeof(buffer);
            if (sysctlbyname(name, buffer, &size, NULL, 0) == 0) {
                bufterm(buffer);
                strcat(features, buffer);
                
                return true;
            }
        }
        // Handle a numeric based entry
        else {
            int64_t ret = 0;
            size        = sizeof(ret);
            if (   sysctlbyname(name, &ret, &size, NULL, 0) == 0
                && snprintf(buffer, sizeof(buffer), "%lld", ret) > 0) {
                strcat(features, buffer);
                strcat(features, " ");
                
                return true;
            }
        }
    }

    return false;
}

static bool get_cpu_core_decription(char *features, int i) {
    if (features != NULL) {
        // Append the core desciption portion of the output
        if (append_cpu_core_entry(features,
                                  "hw.perflevel%d.name",
                                  i,
                                  true)) {
            strcat(features, ":");
            
            // Append the count portion of the output
            return append_cpu_core_entry(features,
                                         "hw.perflevel%d.physicalcpu",
                                         i,
                                         false);
        }
    }
    
    return false;
}

#endif



bool get_cpu_features(char *buffer, size_t len) {
    if (! verify_buffer(buffer, len))
        return false;

#if ANY_X64

    return get_cpu_features_intel(buffer, len);

#elif defined(_WIN64)                       // Windows
    
    char features[2048] = "";

    if (IsProcessorFeaturePresent(PF_ARM_VFP_32_REGISTERS_AVAILABLE)) {
        strcat(features, "NEON ");
    }

    if (IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE)) {
        strcat(features, "DP ");
    }

    strncpy(buffer, features, len);
    strterm(buffer, len);

    return true;

#elif defined(__APPLE__)                    // macOS

    static char features[2048] = "";
    int64_t     ret            = 0;
    size_t      size           = sizeof(ret);

    // Show the types of cores
    if (sysctlbyname("hw.nperflevels", &ret, &size, NULL, 0) == 0) {
        int levels = (int) ret;
        
        for (int i = 0; i < levels; ++i) {
            get_cpu_core_decription(features, i);
        }
    }
    
    if (   sysctlbyname("hw.optional.neon", &ret, &size, NULL, 0) == 0
        && ret == 1) {
        strcat(features, "NEON ");
    }
    if (   sysctlbyname("hw.optional.arm.FEAT_SME", &ret, &size, NULL, 0) == 0
        && ret == 1) {
        strcat(features, "SME ");
    }
    if (   sysctlbyname("hw.optional.arm.FEAT_SME2", &ret, &size, NULL, 0) == 0
        && ret == 1) {
        strcat(features, "SME2 ");
    }

    return snprintf(buffer, len, "%s", features) > 0;
    
#elif ANY_ARM64 && LINUX_64

    return get_cpu_features_arm(buffer, len);
    
#elif LINUX_32

    bool  success = false;
    FILE *fp      = fopen("/proc/cpuinfo", "r");

    if (fp != NULL) {
        char *features = get_proc_cpuinfo_entry(fp, "Features");

        if (features != NULL) {
            success = snprintf(buffer, len, "%s", features) > 0;
        }

        fclose(fp);
    }
    
    return success;

#endif
    
    return false;
}
