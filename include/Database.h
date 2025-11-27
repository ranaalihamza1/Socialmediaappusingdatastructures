#pragma once
#include <string>
#include <vector>
#include "User.h"
#include "Post.h"

using namespace std;

// ============================================================================
// DATABASE CLASS
// ============================================================================
// Manages loading and saving user and post data from/to CSV files
// Provides utility functions for ID generation and CSV parsing

class Database {
public:
    // Constructor: initialize with data directory path
    Database(const string &dataDir);

    // Load users and posts from CSV files
    bool load();

    // Save users and posts to CSV files
    bool save();

    // Data storage
    vector<User> users;
    vector<Post> posts;

    // Generate next available user ID
    int nextUserId() const;

    // Generate next available post ID
    int nextPostId() const;

private:
    string dir;

    // Get full path to users CSV file
    string usersPath() const;

    // Get full path to posts CSV file
    string postsPath() const;

    // CSV helper functions
    static vector<string> split(const string &s, char d);
    static string join(const vector<string> &parts, char d);
};
