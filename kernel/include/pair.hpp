#ifndef _PAIR_HPP_
#define _PAIR_HPP_

#include <kernel/essentials.hpp>

template <typename T1, typename T2>
struct Pair {
    Pair() = default;
    Pair(Pair& p) = default;
    Pair(T1 t , T2 u) : first(t) , second(u) {}
    void swap_with_new(Pair &new_p) {
        swap(first , new_p.first);
        swap(second , new_p.second);
    }
    Pair& operator=(const Pair& p) {
        first = p.first;
        second = p.second;
        return *this;
    }

    bool operator>=(const Pair<T1,T2>& rhs) {
        if(first == rhs.first) return second >= rhs.second;
        return first >= rhs.first;
    }
    bool operator<=(const Pair<T1,T2>& rhs) {
        if(first == rhs.first) return second <= rhs.second;
        return first <= rhs.first;
    }
    bool operator>(const Pair<T1,T2>& rhs) {
        if(first == rhs.first) return second > rhs.second;
        return first > rhs.first;
    }
    bool operator<(const Pair<T1,T2>& rhs) {
        if(first == rhs.first) return second < rhs.second;
        return first < rhs.first;
    }
    T1 first;
    T2 second;
};

template<typename T1 , typename T2>
bool operator==(const Pair<T1,T2>& lhs , const Pair<T1,T2>& rhs) { return (lhs.first == rhs.first) && (lhs.second == rhs.second); }

template <typename T1 , typename T2>
Pair<T1,T2> make_pair(T1 t , T2 u) { return {t , u}; }



#endif