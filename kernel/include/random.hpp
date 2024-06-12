#ifndef _RANDOM_HPP_
#define _RANDOM_HPP_

#include <kernel/essentials.hpp>

#define RANDOM_MAXIMUM 65536

void srand(long seed);
long rand(void);

#endif