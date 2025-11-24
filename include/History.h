#pragma once
#include <stack>
#include <string>
#include <variant>

struct Action {
    std::string type; // view, like, delete
    int userId;
    int postId;
    // payload for undo could be stored here (e.g., serialized post)
    std::string payload;
};

class History {
public:
    void push(const Action &a) { st.push(a); }
    bool undo(Action &out) {
        if (st.empty()) return false;
        out = st.top(); st.pop(); return true;
    }
    bool empty() const { return st.empty(); }
private:
    std::stack<Action> st;
};
