#pragma once
#include <stack>
#include <string>
#include <variant>

using namespace std;

// ============================================================================
// ACTION STRUCT & HISTORY CLASS
// ============================================================================
// Tracks user actions for undo functionality

struct Action {
    string type;      // Action type: "like", "delete", "create", "view"
    int userId;       // User who performed the action
    int postId;       // Post affected by the action
    string payload;   // Additional data (e.g., post content for undo-delete)
};

class History {
public:
    // Push an action onto the history stack
    void push(const Action &a) {
        st.push(a);
    }

    // Pop and retrieve the last action
    bool undo(Action &out) {
        if (st.empty())
            return false;
        out = st.top();
        st.pop();
        return true;
    }

    // Check if history is empty
    bool empty() const {
        return st.empty();
    }

private:
    stack<Action> st;
};
