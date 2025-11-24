#pragma once
#include <string>
#include "LinkedList.h"

struct User {
    int id;
    std::string name;
    std::string bio;
    LinkedList<int> friends; // store friend user ids

    User(): id(0) {}
};
