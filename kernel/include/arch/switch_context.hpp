#ifndef _SWITCH_CONTEXT_HPP_
#define _SWITCH_CONTEXT_HPP_

#include <kernel/types.hpp>

extern "C" void ARCHDEP switch_context(Registers *current_context , Registers *next_context);

#endif