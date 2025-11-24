#pragma once
#include <string>
#include <vector>
#include "User.h"
#include "Post.h"

class Database {
public:
    Database(const std::string &dataDir);
    bool load();
    bool save();

    std::vector<User> users;
    std::vector<Post> posts;

    int nextUserId() const;
    int nextPostId() const;

private:
    std::string dir;
    std::string usersPath() const;
    std::string postsPath() const;
    // simple CSV helpers
    static std::vector<std::string> split(const std::string &s, char d);
    static std::string join(const std::vector<std::string> &parts, char d);
};
