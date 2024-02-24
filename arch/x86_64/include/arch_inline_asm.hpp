#ifndef _ARCH_INLINE_ASM_HPP_
#define _ARCH_INLINE_ASM_HPP_

#define IA __asm__
#define IA_DISABLE_INTERRUPT __asm__ ("cli");
#define IA_ENABLE_INTERRUPT  __asm__ ("sti");

#define IA_RETURN            __asm__ ("ret");
#define IA_INTERRUPT_RETURN  __asm__ ("iretq");

#endif