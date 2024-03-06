#include <hash_table.hpp>

hash_index_t hash_function_string(char *key) {
    const int p = 53;
    const int m = 1e9+9;
    hash_index_t hash_value = 0;
    int pow_p = 1;
    for(int i = 0; key[i] != 0x00; i++) {
        hash_value = (hash_value+(key[i]-'a'+1)*pow_p)%m;
        pow_p = (p*pow_p)%m;
    }
    return hash_value;
}