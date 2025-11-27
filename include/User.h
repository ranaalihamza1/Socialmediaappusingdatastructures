#pragma once
#include <string>
#include "LinkedList.h"

using namespace std;

// ============================================================================
// USER STRUCT
// ============================================================================
// Represents a social media user with profile information and friend list

struct User {
    int id;                      // Unique user identifier
    string username;             // Login username (unique)
    string name;                 // Display name
    string bio;                  // User biography
    string password;             // Password (plain text for lab)
    LinkedList<int> friends;     // LinkedList of friend user IDs

    // Default constructor
    User() : id(0) {}
};
