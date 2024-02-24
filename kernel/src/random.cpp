#include <random.hpp>

/* Random function, source from : 
 * https://xoax.net/sub_cpp/crs_core/Lesson22/
 */

static int global_seed = 0;

void srand(long seed) {
    global_seed = seed;
}

long rand(void) {
    global_seed = global_seed*1134232345+24923;
    return ((unsigned)(global_seed/65536)%32768);
}