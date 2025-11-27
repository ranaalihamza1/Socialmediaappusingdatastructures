#include "../include/Feed.h"
#include <algorithm>
#include <ctime>
#include <sstream>

using namespace std;

// ============================================================================
// FEED MANAGER IMPLEMENTATION
// ============================================================================

FeedManager::FeedManager(Database &db) : db(db) {}

// ============================================================================
// QUEUE MANAGEMENT
// ============================================================================

void FeedManager::buildQueue() {
    // Build queue in chronological order (by post ID)
    vector<Post> tmp = db.posts;
    sort(tmp.begin(), tmp.end(), [](const Post &a, const Post &b) {
        return a.id < b.id;
    });

    // Clear and rebuild queue
    while (!q.empty())
        q.pop();

    for (auto &p : tmp)
        q.push(p.id);
}

// ============================================================================
// FEED RETRIEVAL
// ============================================================================

vector<Post> FeedManager::getFIFO(int limit) {
    // Return posts in FIFO order (oldest first)
    vector<Post> out;
    auto qq = q; // Copy of queue

    while (!qq.empty() && (int)out.size() < limit) {
        int id = qq.front();
        qq.pop();

        auto it = find_if(db.posts.begin(), db.posts.end(),
                         [&](const Post &p) { return p.id == id; });
        if (it != db.posts.end())
            out.push_back(*it);
    }

    return out;
}

vector<Post> FeedManager::getTrending(int limit) {
    // Return posts sorted by likes (most liked first)
    auto tmp = db.posts;
    sort(tmp.begin(), tmp.end(), [](const Post &a, const Post &b) {
        return a.likes.size() > b.likes.size();
    });

    if ((int)tmp.size() > limit)
        tmp.resize(limit);

    return tmp;
}

vector<Post> FeedManager::search(const string &keyword) {
    // Search for posts containing keyword in content
    vector<Post> out;
    for (auto &p : db.posts) {
        if (p.content.find(keyword) != string::npos)
            out.push_back(p);
    }
    return out;
}

// ============================================================================
// LIKE MANAGEMENT
// ============================================================================

bool FeedManager::likePost(int postId, int userId) {
    // Add like to post if user hasn't already liked it
    for (auto &p : db.posts) {
        if (p.id == postId) {
            auto it = find(p.likes.begin(), p.likes.end(), userId);
            if (it == p.likes.end()) {
                // Not yet liked by this user
                p.likes.push_back(userId);
                return true;
            }
            // Already liked
            return false;
        }
    }
    return false;
}

bool FeedManager::unlikePost(int postId, int userId) {
    // Remove like from post
    for (auto &p : db.posts) {
        if (p.id == postId) {
            auto it = find(p.likes.begin(), p.likes.end(), userId);
            if (it != p.likes.end()) {
                p.likes.erase(it);
                return true;
            }
            return false;
        }
    }
    return false;
}

// ============================================================================
// POST DELETION
// ============================================================================

bool FeedManager::deletePost(int postId) {
    // Remove post from database
    auto it = remove_if(db.posts.begin(), db.posts.end(),
                       [&](const Post &p) { return p.id == postId; });

    if (it != db.posts.end()) {
        db.posts.erase(it, db.posts.end());
        return true;
    }

    return false;
}
