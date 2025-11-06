#ifndef _X86_64_CPUID_HPP_
#define _X86_64_CPUID_HPP_

#include <kernel/essentials.hpp>

static inline void cpuid(dword req , dword &eax , dword &ebx , dword &ecx , dword &edx) {
    __asm__ (
        "mov eax , %0\n\t"
        "cpuid\n\t"
        "mov %0 , eax\n\t"
        "mov %1 , ebx\n\t"
        "mov %2 , ecx\n\t"
        "mov %3 , edx\n\t"
    :"=r"(eax) , "=r"(ebx) , "=r"(ecx) , "=r"(edx)
    :"0"(req));
}

#endif