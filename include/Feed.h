#pragma once
#include <queue>
#include <vector>
#include <string>
#include "Post.h"
#include "Database.h"

using namespace std;

// ============================================================================
// FEED MANAGER CLASS
// ============================================================================
// Manages feed generation: FIFO ordering, trending/popularity sorting, and search
// Provides like/unlike and post deletion functionality

class FeedManager {
public:
    // Constructor: initialize with database reference
    FeedManager(Database &db);

    // Build the feed queue (populate with post IDs in chronological order)
    void buildQueue();

    // Get posts in FIFO order (oldest first)
    vector<Post> getFIFO(int limit = 20);

    // Get posts sorted by likes (trending)
    vector<Post> getTrending(int limit = 20);

    // Search posts by keyword
    vector<Post> search(const string &keyword);

    // Like a post (add user ID to post's likes list)
    bool likePost(int postId, int userId);

    // Unlike a post (remove user ID from post's likes list)
    bool unlikePost(int postId, int userId);

    // Delete a post
    bool deletePost(int postId);

private:
    Database &db;
    queue<int> q; // Queue of post IDs
};
