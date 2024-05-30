/**
 * @file hash_table.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The general hash table for kernel
 * @date 2024-03-02
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho. 
 * 
 */

#ifndef _HASH_TABLE_HPP_
#define _HASH_TABLE_HPP_

#include <kernel/interface_type.hpp>
#include <kernel/mem/kmem_manager.hpp>
#include <linked_list.hpp>

typedef int hash_index_t;

/**
 * @brief The general hash table
 * 
 * @tparam T_OBJ The type of the object to be stored
 * @tparam T_KEY The type of the key
 */
template <typename T_OBJ , typename T_KEY> struct HashTable {
    struct list_object {
        T_KEY key;
        T_OBJ *object;
    };
    struct hash_container_s {
        ObjectLinkedList<list_object>*objects_container;
    }*hash_container;
    bool init(hash_index_t max , void(*op_copy)(T_KEY &dest , T_KEY src) , bool(*op_compare)(T_KEY dest , T_KEY src) , hash_index_t(*op_hash)(T_KEY key)) {
        if(max > 0xFFFFFFF) {
            return false;
        }
        max_index = max;
        key_op_copy = op_copy;
        key_op_compare = op_compare;
        hash_function = op_hash;
        // allocate the space
        hash_container = (hash_container_s *)memory::pmem_alloc(max_index*sizeof(hash_container_s));
        for(hash_index_t i = 0; i < max_index; i++) {
            hash_container[i].objects_container = 0x00;
        }
        return true;
    }
    bool add(T_KEY key , T_OBJ *object) {
        hash_index_t index = hash_function(key)%max_index;
        if(hash_container[index].objects_container == 0x00) {
            hash_container[index].objects_container = (ObjectLinkedList<list_object>*)memory::pmem_alloc(sizeof(ObjectLinkedList<list_object>));
            hash_container[index].objects_container->init();
        }
        ObjectLinkedList<list_object>*lst = hash_container[index].objects_container;
        // Check whether there is an item with same key
        struct ObjectLinkedList<list_object>::node_s *ptr = lst->get_start_node();
        while(ptr != 0x00) {
            if(ptr->object == 0x00) {
                ptr = ptr->next;
                continue;
            }
            // If there exists a same object with same key
            if(key_op_compare(ptr->object->key , key)) {
                return false;
            }
            ptr = ptr->next;
        }
        struct list_object *temp_obj = (list_object *)memory::pmem_alloc(sizeof(list_object));
        temp_obj->object = object;
        key_op_copy(temp_obj->key , key);
        if(hash_container[index].objects_container->add_object_rear(temp_obj) == INVALID) return false;
        
        return true;
    }
    bool remove(T_KEY key) {
        hash_index_t index = hash_function(key)%max_index;
        T_OBJ *object = search(key);
        return hash_container[index].object_container->remove_object(object);
    }
    T_OBJ *search(T_KEY key) {
        hash_index_t index = hash_function(key)%max_index;
        if(hash_container[index].objects_container == 0x00) return 0x00;
        struct ObjectLinkedList<list_object>::node_s *target = 0x00;
        struct ObjectLinkedList<list_object>::node_s *ptr = hash_container[index].objects_container->get_start_node();
        while(ptr != 0x00) {
            if(ptr->object == 0x00) {
                ptr = ptr->next;
                continue;
            }
            if(key_op_compare(ptr->object->key , key)) {
                target = ptr;
                break;
            }
            ptr = ptr->next;
        }
        if(target == 0x00) return 0x00;
        return target->object->object;
    }
    
    hash_index_t(*hash_function)(T_KEY key);
    void(*key_op_copy)(T_KEY &dest , T_KEY src);
    bool(*key_op_compare)(T_KEY dest , T_KEY src);
    hash_index_t max_index;
};

hash_index_t hash_function_string(char *key);

#endif