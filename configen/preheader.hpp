#include <arch/configurations.hpp>

#ifndef yes
#define yes 1
#endif

#ifndef no
#define no 0
#endif

#ifndef WARNING
#define WARNING(x) debug::out::printf(DEBUG_WARNING , x);
#endif