#ifndef _OBJECT_MANAGER_HPP_
#define _OBJECT_MANAGER_HPP_

#include <kernel/essentials.hpp>
#include <string.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <kernel/debug.hpp>

#include <kernel/debug.hpp>

template <typename T> class FixedArray {
public:
    FixedArray() = default;
    void init(T *arr , max_t sz , max_t m_cnt , memory::memory_allocator_func_t f_alloc=memory::pmem_alloc , memory::memory_deallocator_func_t f_free=memory::pmem_free) {
        count = sz;
        max_count = m_cnt;
        m_alloc = f_alloc;
        m_free = f_free;
        container = (FixedArray::data_container_s *)m_alloc(max_count*sizeof(FixedArray::data_container_s) , 0);
        for(max_t i = 0; i < sz; i++) {
            container[i].occupied = true;
            (*container[i]) = arr[i];
        }
    }
    void init(max_t m_cnt , memory::memory_allocator_func_t f_alloc=memory::pmem_alloc , memory::memory_deallocator_func_t f_free=memory::pmem_free) {
        count = 0;
        max_count = m_cnt;
        m_alloc = f_alloc;
        m_free = f_free;

        container = (FixedArray::data_container_s *)m_alloc(max_count*sizeof(FixedArray::data_container_s) , 0);
        for(max_t i = 0; i < max_count; i++) {
            container[i].occupied = false;
        }
    }
    max_t add_empty_space() {
        if(container == 0x00) {
            return INVALID;
        }
        if(count >= max_count) return INVALID;
        max_t i = 0;
        for(; i < max_count; i++) {
            if(container[i].occupied == false) break;
        }
        if(i >= max_count) return INVALID; // all object is occupied
        container[i].occupied = true;
        count++;
        return i;
    }
    max_t add(const T& data) {
        max_t id = add_empty_space();
        if(id == INVALID) return INVALID;

        (*container[id]) = data;
        return id;
    }
    bool discard(T matching_data) {
        for(max_t i = 0; i < max_count; i++) {
            if((*container[i]) == matching_data) {
                return discard_space(i);
            }
        }
        return false;
    }
    bool discard_space(max_t id) {
        if(container[id].occupied != true) return false;

        container[id].occupied = false;
        count--;
        return true;
    }
    template <typename T2> max_t search(bool (*check)(T& data , T2 sample_data) , T2 sample_data) {
        for(max_t i = 0; i < max_count; i++) {
            if(check(*container[i] , sample_data) == true) {
                return i;
            }
        }
        return INVALID;
    }
    T &get(max_t id) const {
        return *container[id];
    }
    T &operator[](max_t id) const { return *container[id]; }

    max_t get_max_size() const { return max_count; }
    max_t size() const { return count; }

    /// @brief Deallocate the data container. Warning: To prevent memory leak, you must de-allocate all the objects in the container!
    void deallocate_this() {
        // if m_free is nullptr, this array can't be de-allocated
        if(m_free == nullptr) return;

        m_free(container);
        max_count = 0;
        count = 0;
    }
protected:
    class data_container_s {
    public:
        bool occupied;

        T& operator*(void) { return data; }
        data_container_s &operator=(const T &d) {
            data = d;
            return *this;
        }
    private:
        T data;
    }*container = 0x00;

private:
    memory::memory_allocator_func_t   m_alloc = memory::pmem_alloc;
    memory::memory_deallocator_func_t m_free  = memory::pmem_free;

    max_t max_count = 0;
    max_t count = 0;
};

#endif