#ifndef _OBJECT_LIST_HPP_
#define _OBJECT_LIST_HPP_

#include <kernel/essentials.hpp>

template <typename T> class LinkedList {
public:
    struct node_s {
        max_t id;
        T object;

        node_s *previous = nullptr;
        node_s *next = nullptr;

        node_s() = default;
        node_s(const node_s &n) : previous(nullptr) , next(nullptr) , id(n.id) , object(n.object) {}
        node_s(const node_s *n) : previous(nullptr) , next(nullptr) , id(n->id) , object(n->object) {}
        ~node_s() {}
    };

    LinkedList() = default;
    ~LinkedList();

    void init(void);
    node_s *add_front(T obj);
    node_s *add_rear(T obj);
    bool remove(node_s *node) { return remove_node(node); }
    bool remove(T obj) { return remove_node(get_node(obj)); }
    bool remove(max_t id) { return remove_node(get_node(id)); }
        
    /// @brief Get the node_s structure from the given id
    /// @tparam T 
    /// @param id ID
    /// @return The node_s structure corresponding to the provided id
    node_s *get_node(max_t id) const;
    node_s *get_node(const T &obj) const;
    inline node_s *get_start_node(void) const { return start_node; }
    max_t size() const { return count; }

    /// @brief Scan through the entire list, call the provided check() function for every element
    /// @tparam T 
    /// @param check The checker function, user-provided, it must compare data with the given sample_data
    /// @param sample_data the sample data that will be used when calling the check() function internally
    /// @return The first element that the check() function returned true, otherwise 0x00
    template <typename F>
    node_s *search(F check) {
        node_s *ptr = start_node;
        while(ptr != nullptr) {
            if(check(ptr->object)) {
                return ptr;
            }
            ptr = ptr->next;
        }
        return nullptr;
    }

protected:
    node_s *start_node = nullptr;
    node_s *last_node = nullptr;
    max_t id_index = 0;
    max_t count = 0;

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
        delete target;
        return true;
    }
    inline max_t allocate_id(void) { return id_index++; }
};

template <typename T>
void LinkedList<T>::init() {
    count = 0;
    id_index = 0;
    start_node = nullptr;
    
    last_node = start_node;
}

template <typename T>
LinkedList<T>::node_s *LinkedList<T>::add_front(T obj) { // id
    node_s *new_node = new node_s();
    new_node->object = obj;

    if(start_node == 0x00) {
        start_node = new_node;
        new_node->previous = 0x00;
        new_node->next = 0x00;
        last_node = new_node;
        count = 1;
        return new_node;
    }
    new_node->previous = 0x00;
    connect_node(new_node , start_node);
    new_node->id = allocate_id();
        
    start_node = new_node;
    count++;
    return new_node;
}

template <typename T>
LinkedList<T>::node_s *LinkedList<T>::add_rear(T obj) { // id{
    node_s *new_node = new node_s();
    new_node->object = obj;
    if(start_node == 0x00) {
        start_node = new_node;
        new_node->previous = 0x00;
        new_node->next = 0x00;
        last_node = new_node;

        count = 1;
        return new_node;
    }
    new_node->next = 0x00;
    connect_node(last_node , new_node);
    last_node = new_node;
    count++;
    return new_node;
}

template <typename T>
LinkedList<T>::node_s *LinkedList<T>::get_node(max_t id) const {
    node_s *ptr = start_node;
    while(ptr != 0x00) {
        if(ptr->id == id) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return 0x00;
}

template <typename T>
LinkedList<T>::node_s *LinkedList<T>::get_node(const T &obj) const {
    node_s *ptr = start_node;
    while(ptr != 0x00) {
        if(ptr->object == obj) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return 0x00;
}

template <typename T>
LinkedList<T>::~LinkedList() {
    node_s *ptr = start_node;
    if(ptr == nullptr) return;
    
    while(ptr != nullptr) {
        auto next = ptr->next;
        delete ptr;
        ptr = next;
    }
}

#endif