#ifndef __THREADQUEUE_HPP__
#define __THREADQUEUE_HPP__

class _thread;

class ThreadQueue {
public:
    constexpr ThreadQueue(): head(nullptr), tail(nullptr) {}

    void put(_thread* t);
    _thread* get();

    _thread* peek() const { return head; }
    bool isEmpty() const { return head == nullptr; }

private:
    _thread* head;
    _thread* tail;
};





#endif // __THREADQUEUE_HPP__