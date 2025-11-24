#pragma once
#include <memory>
#include <functional>

template<typename T>
class LinkedList {
public:
    struct Node {
        T value;
        std::shared_ptr<Node> next;
        Node(const T &v): value(v), next(nullptr) {}
    };

    LinkedList() = default;

    void push_front(const T &v) {
        auto n = std::make_shared<Node>(v);
        n->next = head;
        head = n;
    }

    bool remove(const T &v) {
        std::shared_ptr<Node> prev = nullptr;
        auto cur = head;
        while (cur) {
            if (cur->value == v) {
                if (prev) prev->next = cur->next;
                else head = cur->next;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    bool contains(const T &v) const {
        auto cur = head;
        while (cur) {
            if (cur->value == v) return true;
            cur = cur->next;
        }
        return false;
    }

    void for_each(std::function<void(const T&)> fn) const {
        auto cur = head;
        while (cur) { fn(cur->value); cur = cur->next; }
    }

    bool empty() const { return head == nullptr; }

private:
    std::shared_ptr<Node> head = nullptr;
};
