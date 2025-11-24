#include "../include/Feed.h"
#include <algorithm>
#include <ctime>
#include <sstream>

FeedManager::FeedManager(Database &db): db(db) {}

void FeedManager::buildQueue() {
    // build queue in chronological order from posts vector (assumes timestamp string compare works)
    std::vector<Post> tmp = db.posts;
    std::sort(tmp.begin(), tmp.end(), [](const Post &a, const Post &b){ return a.id < b.id; });
    while (!q.empty()) q.pop();
    for (auto &p: tmp) q.push(p.id);
}

std::vector<Post> FeedManager::getFIFO(int limit) {
    std::vector<Post> out;
    auto qq = q; // copy
    while (!qq.empty() && (int)out.size()<limit) {
        int id = qq.front(); qq.pop();
        auto it = std::find_if(db.posts.begin(), db.posts.end(), [&](const Post &p){ return p.id==id; });
        if (it!=db.posts.end()) out.push_back(*it);
    }
    return out;
}

std::vector<Post> FeedManager::getTrending(int limit) {
    auto tmp = db.posts;
    std::sort(tmp.begin(), tmp.end(), [](const Post &a, const Post &b){ return a.likes.size() > b.likes.size(); });
    if ((int)tmp.size()>limit) tmp.resize(limit);
    return tmp;
}

std::vector<Post> FeedManager::search(const std::string &keyword) {
    std::vector<Post> out;
    for (auto &p: db.posts) {
        if (p.content.find(keyword) != std::string::npos) out.push_back(p);
    }
    return out;
}

bool FeedManager::likePost(int postId, int userId) {
    for (auto &p: db.posts) if (p.id==postId) {
        if (std::find(p.likes.begin(), p.likes.end(), userId)==p.likes.end()) {
            p.likes.push_back(userId);
            return true;
        }
        return false;
    }
    return false;
}

bool FeedManager::unlikePost(int postId, int userId) {
    for (auto &p: db.posts) if (p.id==postId) {
        auto it = std::find(p.likes.begin(), p.likes.end(), userId);
        if (it!=p.likes.end()) { p.likes.erase(it); return true; }
        return false;
    }
    return false;
}

bool FeedManager::deletePost(int postId) {
    auto it = std::remove_if(db.posts.begin(), db.posts.end(), [&](const Post &p){ return p.id==postId; });
    if (it!=db.posts.end()) { db.posts.erase(it, db.posts.end()); return true; }
    return false;
}
