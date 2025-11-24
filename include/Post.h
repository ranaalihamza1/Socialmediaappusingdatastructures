#pragma once
#include <string>
#include <vector>

struct Post {
    int id;
    int authorId;
    std::string timestamp;
    std::string content;
    std::vector<int> likes; // user ids

    Post(): id(0), authorId(0) {}
};
