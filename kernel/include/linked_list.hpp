#ifndef _OBJECT_LIST_HPP_
#define _OBJECT_LIST_HPP_

#include <kernel/essentials.hpp>
#include <kernel/mem/kmem_manager.hpp>

template <typename T> class LinkedList {
    public:
        struct node_s {
            max_t id;
            T object;

            node_s *previous;
            node_s *next;
        };

        void init(void) {
            count = 0;
            id_index = 0;
            start_node = 0x00;
            
            last_node = start_node;
        }
        node_s *add_front(T obj) { // id
            node_s *new_node = (node_s *)memory::pmem_alloc(sizeof(node_s));
            new_node->object = obj;

            if(start_node == 0x00) {
                start_node = new_node;
                new_node->previous = 0x00;
                new_node->next = 0x00;
                last_node = new_node;
                return new_node;
            }
            new_node->previous = 0x00;
            connect_node(new_node , start_node);
            new_node->id = allocate_id();
            
            start_node = new_node;
            count++;
            return new_node;
        }
        node_s *add_rear(T obj) {
            node_s *new_node = (node_s *)memory::pmem_alloc(sizeof(node_s));
            new_node->object = obj;

            if(start_node == 0x00) {
                start_node = new_node;
                new_node->previous = 0x00;
                new_node->next = 0x00;
                last_node = new_node;
                return new_node;
            }
            new_node->next = 0x00;
            connect_node(last_node , new_node);
            last_node = new_node;
            count++;
            return new_node;
        }
        bool remove(node_s *node) { return remove_node(node); }
        bool remove(T obj) { return remove_node(get_node(obj)); }
        bool remove(max_t id) { return remove_node(get_node(id)); }
        
        /// @brief Get the node_s structure from the given id
        /// @tparam T 
        /// @param id ID
        /// @return The node_s structure corresponding to the provided id
        node_s *get_node(max_t id) const {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(ptr->id == id) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }
        
        node_s *get_node(const T &obj) const {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(ptr->object == obj) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }
        inline node_s *get_start_node(void) const { return start_node; }
        max_t size() const { return count; }

        /// @brief Scan through the entire list, call the provided check() function for every element
        /// @tparam T 
        /// @param check The checker function, user-provided, it must compare data with the given sample_data
        /// @param sample_data the sample data that will be used when calling the check() function internally
        /// @return The first element that the check() function returned true, otherwise 0x00
        template <typename T2> node_s *search(bool (*check)(T object , T2 sample_data) , T2 sample_data) {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(check(ptr->object , sample_data)) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }

    protected:
        node_s *start_node;
        node_s *last_node;
        max_t id_index;
        max_t count;
    private:
        void connect_node(node_s *first , node_s *next) {
            first->next = next;
            next->previous = first;
        }
        bool remove_node(node_s *target) {
            if(target == 0x00) return false;

            if(target->previous == 0x00) start_node = target->next;
            else target->previous->next = target->next;

            count--;
            memory::pmem_free(target);
            return true;
        }

        inline max_t allocate_id(void) { return id_index++; }
};

#endif