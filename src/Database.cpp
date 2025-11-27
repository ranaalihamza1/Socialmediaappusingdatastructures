#include "../include/Database.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>

using namespace std;

// ============================================================================
// DATABASE IMPLEMENTATION
// ============================================================================

Database::Database(const string &dataDir) : dir(dataDir) {}

string Database::usersPath() const {
    return dir + "\\users.csv";
}

string Database::postsPath() const {
    return dir + "\\posts.csv";
}

int Database::nextUserId() const {
    int maxid = 0;
    for (auto &u : users) {
        if (u.id > maxid)
            maxid = u.id;
    }
    return maxid + 1;
}

int Database::nextPostId() const {
    int maxid = 0;
    for (auto &p : posts) {
        if (p.id > maxid)
            maxid = p.id;
    }
    return maxid + 1;
}

// ============================================================================
// CSV PARSING HELPERS
// ============================================================================

// Split CSV line by delimiter, handling quoted fields
vector<string> Database::split(const string &s, char d) {
    vector<string> out;
    string cur;
    bool inQuotes = false;

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];

        if (c == '"') {
            if (inQuotes && i + 1 < s.size() && s[i + 1] == '"') {
                // Escaped quote
                cur.push_back('"');
                ++i;
            }
            else {
                inQuotes = !inQuotes;
            }
        }
        else if (c == d && !inQuotes) {
            out.push_back(cur);
            cur.clear();
        }
        else {
            cur.push_back(c);
        }
    }

    out.push_back(cur);
    return out;
}

// Join fields into CSV line with proper quoting
string Database::join(const vector<string> &parts, char d) {
    ostringstream ss;

    for (size_t i = 0; i < parts.size(); ++i) {
        if (i)
            ss << d;

        const string &f = parts[i];
        bool needQuotes = f.find(d) != string::npos ||
                          f.find('"') != string::npos ||
                          f.find('\n') != string::npos;

        if (!needQuotes) {
            ss << f;
        }
        else {
            ss << '"';
            for (char c : f) {
                if (c == '"')
                    ss << '"' << '"';
                else
                    ss << c;
            }
            ss << '"';
        }
    }

    return ss.str();
}

// ============================================================================
// FILE I/O
// ============================================================================

bool Database::load() {
    users.clear();
    posts.clear();

    // Ensure data directory exists
    try {
        filesystem::create_directories(dir);
    }
    catch (...) {}

    // Ensure CSV files exist
    if (!filesystem::exists(usersPath())) {
        ofstream out(usersPath());
        out.close();
    }
    if (!filesystem::exists(postsPath())) {
        ofstream out(postsPath());
        out.close();
    }

    // Load users
    ifstream fu(usersPath());
    if (!fu.is_open())
        return false;

    string line;
    while (getline(fu, line)) {
        if (line.empty())
            continue;

        auto cols = split(line, ',');
        User u;

        try {
            u.id = stoi(cols[0]);
        }
        catch (...) {
            continue;
        }

        // CSV columns: id, username, name, bio, password, friends (semicolon-separated)
        if (cols.size() > 1)
            u.username = cols[1];
        if (cols.size() > 2)
            u.name = cols[2];
        if (cols.size() > 3)
            u.bio = cols[3];
        if (cols.size() > 4)
            u.password = cols[4];

        // Parse friends list
        if (cols.size() > 5 && !cols[5].empty()) {
            auto fr = split(cols[5], ';');
            for (auto &s : fr) {
                try {
                    u.friends.push_front(stoi(s));
                }
                catch (...) {}
            }
        }

        users.push_back(u);
    }
    fu.close();

    // Load posts
    ifstream fp(postsPath());
    if (!fp.is_open())
        return false;

    while (getline(fp, line)) {
        if (line.empty())
            continue;

        auto cols = split(line, ',');
        Post p;

        try {
            p.id = stoi(cols[0]);
        }
        catch (...) {
            continue;
        }

        try {
            p.authorId = stoi(cols[1]);
        }
        catch (...) {
            p.authorId = 0;
        }

        // CSV columns: id, authorId, timestamp, content, likes (semicolon-separated)
        if (cols.size() > 2)
            p.timestamp = cols[2];
        if (cols.size() > 3)
            p.content = cols[3];

        // Parse likes list
        if (cols.size() > 4 && !cols[4].empty()) {
            auto lk = split(cols[4], ';');
            for (auto &s : lk) {
                try {
                    p.likes.push_back(stoi(s));
                }
                catch (...) {}
            }
        }

        posts.push_back(p);
    }
    fp.close();

    return true;
}

bool Database::save() {
    // Save users
    ofstream fu(usersPath(), ios::trunc);
    if (!fu.is_open())
        return false;

    for (auto &u : users) {
        vector<string> fr;
        u.friends.for_each([&](const int &id) {
            fr.push_back(to_string(id));
        });

        fu << u.id << "," << u.username << "," << u.name << ","
           << u.bio << "," << u.password << "," << join(fr, ';') << "\n";
    }
    fu.close();

    // Save posts
    ofstream fp(postsPath(), ios::trunc);
    if (!fp.is_open())
        return false;

    for (auto &p : posts) {
        vector<string> lk;
        for (auto id : p.likes)
            lk.push_back(to_string(id));

        fp << p.id << "," << p.authorId << "," << p.timestamp << ","
           << p.content << "," << join(lk, ';') << "\n";
    }
    fp.close();

    return true;
}
