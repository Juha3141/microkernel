#ifndef _OBJECT_LIST_HPP_
#define _OBJECT_LIST_HPP_

#include <kernel/interface_type.hpp>
#include <kernel/kmem_manager.hpp>

template <typename T> class ObjectLinkedList {
    public:
        struct node_s {
            max_t id;
            T *object;

            node_s *previous;
            node_s *next;
        };
        void init(void) {
            count = 0;
            id_index = 0;
            start_node = 0x00;
            
            last_node = start_node;
        }
        max_t add_object_front(T *object) { // id
            node_s *new_node = (node_s *)memory::pmem_alloc(sizeof(node_s));
            new_node->object = object;
            new_node->previous = 0x00;
            if(start_node == 0x00) {
                start_node = new_node;
                last_node = start_node;
            }
            else {
                connect_node(new_node , start_node);
                new_node->id = allocate_id();
                
                start_node = new_node;
            }

            count++;
            return new_node->id;
        }
        max_t add_object_rear(T *object) {
            node_s *new_node = (node_s *)memory::pmem_alloc(sizeof(node_s));
            
            new_node->object = object;
            new_node->next = 0x00;
            if(start_node == 0x00) {
                start_node = new_node;
                start_node->previous = 0x00;
                last_node = start_node;
            }
            else {
                connect_node(last_node , new_node);
                last_node = new_node;
            }

            count++;
            return new_node->id;
        }
        bool remove_object(T *object) { return remove_node(get_node(object)); }
        bool remove_object(max_t id) { return remove_node(get_node(id)); }

        bool remove_node(node_s *target) {
            if(target == 0x00) return false;
            if(target->previous == 0x00) {
                start_node = target->next;
            }
            else {
                target->previous->next = target->next;
            }

            count--;
            memory::pmem_free(target);
            return true;
        }
        
        node_s *get_node(T *object) {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(ptr->object == object) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }
        node_s *get_node(max_t id) {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(ptr->id == id) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }
        template <typename T2> node_s *search(bool (*check)(T *object , T2 sample_data) , T2 sample_data) {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(check(ptr->object , sample_data)) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }

        inline node_s *get_start_node(void) { return start_node; }

        max_t count;
        max_t id_index;
    protected:
        node_s *start_node;
        node_s *last_node;
    private:
        void connect_node(node_s *first , node_s *next) {
            first->next = next;
            next->previous = first;
        }
        inline max_t allocate_id(void) { return id_index++; }
};

template <typename T> class DataLinkedList {
    public:
        struct node_s {
            max_t id;
            T data;

            node_s *previous;
            node_s *next;
        };
        void init(void) {
            count = 0;
            id_index = 0;
            start_node = 0x00;
            
            last_node = start_node;
        }
        node_s *register_data_front(void) { // id
            node_s *new_node = (node_s *)memory::pmem_alloc(sizeof(node_s));
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
            return new_node;
        }
        node_s *register_data_rear(void) {
            node_s *new_node = (node_s *)memory::pmem_alloc(sizeof(node_s));
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

            return new_node;
        }
        bool remove_data(node_s *node) { return remove_node(node); }
        bool remove_data(max_t id) { return remove_node(get_node(id)); }

        node_s *get_node(max_t id) {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(ptr->id == id) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }
        template <typename T2> node_s *search(bool (*check)(T object , T2 sample_data) , T2 sample_data) {
            node_s *ptr = start_node;
            while(ptr != 0x00) {
                if(check(ptr->data , sample_data)) {
                    return ptr;
                }
                ptr = ptr->next;
            }
            return 0x00;
        }

        inline node_s *get_start_node(void) { return start_node; }

        max_t count;
        max_t id_index;
    protected:
        node_s *start_node;
        node_s *last_node;
    private:
        void connect_node(node_s *first , node_s *next) {
            first->next = next;
            next->previous = first;
        }
        bool remove_node(node_s *target) {
            if(target == 0x00) return false;
            if(target->previous == 0x00) {
                start_node = target->next;
            }
            else {
                target->previous->next = target->next;
            }
            memory::pmem_free(target);
            return true;
        }
        inline max_t allocate_id(void) { return id_index++; }
};

#endif