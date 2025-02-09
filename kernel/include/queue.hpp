/**
 * @file queue.hpp
 * @author Ian Juha Cho (ianisnumber2027@gmail.com)
 * @brief The general queue structure for kernel
 * @date 2024-03-08
 * 
 * @copyright Copyright (c) 2024 Ian Juha Cho.
 * 
 */
#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_

#include <kernel/essentials.hpp>
#include <kernel/mem/kmem_manager.hpp>

template <typename T> class Queue { // circular queue
    public:
        void init(int c) {
            front = rear = 0;
            capacity = c;
            queue_list = (T *)memory::pmem_alloc(c*sizeof(T));
        }
        inline bool is_empty(void) { return this->front == this->rear; }
        inline int queue_size(void) { return this->front-this->rear; }
        
        void enqueue(T obj) {
            queue_list[front] = obj;            // Stores data
            front = (front+1)%capacity;
        }
        T dequeue(void) {
            T result;
            if(is_empty() == true) return 0x00;
            result = queue_list[rear];
            rear = (rear+1)%capacity;
            return result;
        }
        
    private:
        T *queue_list;
        int capacity;
        int front, rear;
};

template <typename T> class StructQueue { // circular queue
    public:
        void init(int c) {
            front = rear = 0;
            capacity = c;
            queue_list = (T *)memory::pmem_alloc(c*sizeof(T));
        }
        bool is_empty(void) { return this->front == this->rear; }
        
        void enqueue(T obj) {
            memcpy(&queue_list[front] , &obj , sizeof(T));            // Stores data
            front = (front+1)%capacity;
        }
        bool dequeue(T &out) {
            if(is_empty() == true) return false;
            memcpy(&out , &queue_list[rear] , sizeof(T));
            rear = (rear+1)%capacity;

            return true;
        }
        
    private:
        T *queue_list;
        int capacity;
        int front, rear;
};

#endif