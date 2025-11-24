#pragma once
#include <queue>
#include <vector>
#include <string>
#include "Post.h"
#include "Database.h"

class FeedManager {
public:
    FeedManager(Database &db);
    void buildQueue();
    std::vector<Post> getFIFO(int limit=20);
    std::vector<Post> getTrending(int limit=20);
    std::vector<Post> search(const std::string &keyword);
    bool likePost(int postId, int userId);
    bool unlikePost(int postId, int userId);
    bool deletePost(int postId);

private:
    Database &db;
    std::queue<int> q; // post ids
};
