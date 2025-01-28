#ifndef _SWITCH_CONTEXT_HPP_
#define _SWITCH_CONTEXT_HPP_

#include <registers.hpp>

extern "C" void switch_context(struct Registers *current_context , struct Registers *next_context);

#endif