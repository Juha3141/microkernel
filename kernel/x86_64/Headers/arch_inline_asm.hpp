#ifndef _ARCH_INLINE_ASM_HPP_
#define _ARCH_INLINE_ASM_HPP_

#define IA __asm__
#define IA_DISABLE_INTERRUPT __asm__ ("cli");
#define IA_ENABLE_INTERRUPT  __asm__ ("sti");

#endif