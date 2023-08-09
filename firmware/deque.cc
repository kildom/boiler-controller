
#include <stdint.h>
#include "deque.hh"

bool Deque::add(DequeItem *item)
{
    if (item->next != nullptr || item->prev != nullptr || item == first) {
        return false;
    }
    item->next = first;
    if (first != nullptr) {
        first->prev = item;
    }
    first = item;
    return true;
}

bool Deque::remove(DequeItem *item)
{
    if (item->prev != nullptr) {
        item->prev->next = item->next;
    } else if (first == item) {
        first = item->next;
    } else {
        return false;
    }
    if (item->next != nullptr) {
        item->next->prev = item->prev;
    }
    item->next = nullptr;
    item->prev = nullptr;
    return true;
}
