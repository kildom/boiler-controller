#ifndef _DEQUE_H_
#define _DEQUE_H_

class DequeItem {
public:
    DequeItem* next;
    DequeItem* prev;
    DequeItem(): next(nullptr), prev(nullptr) {}
};

class Deque {
public:
    DequeItem* first;
    bool add(DequeItem* item);
    bool remove(DequeItem* item);

    template<typename T>
    T* get_first() {
        return (T*)first;
    }

    template<typename T>
    static T* get_next(T* item) {
        return (T*)((DequeItem*)item)->next;
    }
};


#endif
