#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

template <typename T>
class LockFreeStack {
    struct Node {
        T data;
        Node* next;
        Node(T value) : data(value), next(nullptr) {}
    };

    std::atomic<Node*> head;

public:
    LockFreeStack() : head(nullptr) {}

    void push(T value) {
        Node* newNode = new Node(value);
        do {
            newNode->next = head.load(std::memory_order_relaxed);
        } while (!head.compare_exchange_weak(newNode->next, newNode, std::memory_order_release, std::memory_order_relaxed));
    }

    bool pop(T& result) {
        Node* oldHead = head.load(std::memory_order_relaxed);
        do {
            if (!oldHead) return false; // Stack is empty
        } while (!head.compare_exchange_weak(oldHead, oldHead->next, std::memory_order_acquire, std::memory_order_relaxed));
        result = oldHead->data;
        delete oldHead;
        return true;
    }
};

void testLockFreeStack() {
    LockFreeStack<int> stack;

    auto producer = [&stack]() {
        for (int i = 0; i < 100; ++i) {
            stack.push(i);
        }
    };

    auto consumer = [&stack]() {
        for (int i = 0; i < 100; ++i) {
            int value;
            if (stack.pop(value)) {
                std::cout << "Popped: " << value << std::endl;
            }
        }
    };

    std::thread t1(producer);
    std::thread t2(consumer);

    t1.join();
    t2.join();
}

int main() {
    testLockFreeStack();
    return 0;
}
