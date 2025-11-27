#pragma once
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// POST STRUCT
// ============================================================================
// Represents a social media post with content, author, timestamp, and likes

struct Post {
    int id;                  // Unique post identifier
    int authorId;            // User ID of post author
    string timestamp;        // Post creation timestamp
    string content;          // Post text content
    vector<int> likes;       // List of user IDs who liked this post

    // Default constructor
    Post() : id(0), authorId(0) {}
};
