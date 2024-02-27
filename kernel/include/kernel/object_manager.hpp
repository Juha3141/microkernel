#ifndef _OBJECT_MANAGER_HPP_
#define _OBJECT_MANAGER_HPP_

#include <kernel/interface_type.hpp>
#include <string.hpp>
#include <kernel/kmem_manager.hpp>

#include <kernel/debug.hpp>

template <typename T> class DataManager {
    public:
        void init(int max_data_count) {
            int i;
            count = 0;
            max_count = max_data_count;
            data_container = (struct DataManager::data_container_s *)memory::pmem_alloc(max_count*sizeof(struct DataManager::data_container_s));
            for(i = 0; i < max_count; i++) {
                data_container[i].occupied = false;
                memset(&data_container[i].data , 0 , sizeof(T));
            }
        }
        
        max_t register_space(void) {
            max_t i;
            if(data_container == 0x00) {
                // debug::out::printf(DEBUG_WARNING , "Error : ObjectContainer yet not initialized\n");
                return INVALID;
            }
            if(count >= max_count) return INVALID;
            for(i = 0; i < max_count; i++) {
                if(data_container[i].occupied == false) {
                    break;
                }
            }
            if(i >= max_count) return INVALID; // all object is occupied
            data_container[i].occupied = true;
            count++;
            return i;
        }
        bool discard_space(max_t id) {
            if(data_container[id].occupied == false) return false;
            data_container[id].occupied = true;
            memset(&data_container[id].data , 0 , sizeof(T));
            count--;
            return true;
        }
        template <typename T2> max_t search(bool (*check)(T &data , T2 sample_data) , T2 sample_data) {
            for(max_t i = 0; i < max_count; i++) {
                if(check(data_container[i].data , sample_data) == true) {
                    return i;
                }
            }
            return INVALID;
        }
        
        // some inline function
        inline T *get_data(max_t id) { return &data_container[id].data; }

        max_t max_count = 0;
        max_t count = 0;
    protected:
        struct data_container_s {
            bool occupied;

            T data;
        }*data_container = 0x00;
};

// Manages [Pointer of] some object
template <typename T> class ObjectManager {
    public:
        void init(int max_obj_count) {
            int i;
            count = 0;
            max_count = max_obj_count;
            object_container = (struct ObjectManager::object_container_s *)memory::pmem_alloc(sizeof(struct ObjectManager::object_container_s)*max_count);
            for(i = 0; i < max_count; i++) {
                object_container[i].occupied = false;
                object_container[i].object = 0x00;
            }
        }
        max_t register_object(T *object) { // returns ID
            max_t i;
            if(object_container == 0x00) {
                // debug::out::printf(DEBUG_WARNING , "Error : ObjectContainer yet not initialized\n");
                return INVALID;
            }
            if(count >= max_count) return INVALID;
            for(i = 0; i < max_count; i++) {
                if(object_container[i].occupied == false) {
                    break;
                }
            }
            if(i >= max_count) return INVALID;
            object_container[i].occupied = true;
            object_container[i].object = object;
            count++;
            return i;
        }
        max_t discard_object(T *object) {
            max_t i;
            for(i = 0; i < max_count; i++) {
                if(object_container[i].object == object) {
                    count--;
                    object_container[i].occupied = false;
                    return i;
                }
            }
            return INVALID;
        }
        T *get_object(max_t id) {
            if((id >= max_count)||(object_container[id].occupied == false)) return 0x00;
            return object_container[id].object;
        }
        
        template <typename T2> max_t search(bool (*check)(T *data , T2 sample_data) , T2 sample_data) {
            for(max_t i = 0; i < max_count; i++) {
                if(check(object_container[i].object , sample_data) == true) {
                    return i;
                }
            }
            return INVALID;
        }
        max_t max_count = 0;
        max_t count = 0;
    protected:
        struct object_container_s {
            bool occupied;

            T *object;
        }*object_container = 0x00;
};

#endif