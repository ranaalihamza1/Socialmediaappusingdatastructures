#pragma once
#include <memory>
#include <functional>

using namespace std;

// ============================================================================
// LINKED LIST TEMPLATE CLASS
// ============================================================================
// Generic singly-linked list for storing elements dynamically
// Used for friend lists in User struct

template<typename T>
class LinkedList {
public:
    struct Node {
        T value;
        shared_ptr<Node> next;

        Node(const T &v) : value(v), next(nullptr) {}
    };

    LinkedList() = default;

    // Add element to front of list
    void push_front(const T &v) {
        auto n = make_shared<Node>(v);
        n->next = head;
        head = n;
    }

    // Remove first occurrence of element
    bool remove(const T &v) {
        shared_ptr<Node> prev = nullptr;
        auto cur = head;

        while (cur) {
            if (cur->value == v) {
                if (prev)
                    prev->next = cur->next;
                else
                    head = cur->next;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    // Check if element exists in list
    bool contains(const T &v) const {
        auto cur = head;
        while (cur) {
            if (cur->value == v)
                return true;
            cur = cur->next;
        }
        return false;
    }

    // Apply function to each element
    void for_each(function<void(const T&)> fn) const {
        auto cur = head;
        while (cur) {
            fn(cur->value);
            cur = cur->next;
        }
    }

    // Check if list is empty
    bool empty() const {
        return head == nullptr;
    }

private:
    shared_ptr<Node> head = nullptr;
};
