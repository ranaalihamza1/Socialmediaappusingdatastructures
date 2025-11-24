#include "../include/Database.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>

// Robust CSV helpers: simple parser handling quoted fields and joining with quoting

Database::Database(const std::string &dataDir): dir(dataDir) {}

std::string Database::usersPath() const { return dir + "\\users.csv"; }
std::string Database::postsPath() const { return dir + "\\posts.csv"; }

int Database::nextUserId() const {
    int maxid = 0;
    for (auto &u: users) if (u.id > maxid) maxid = u.id;
    return maxid + 1;
}

int Database::nextPostId() const {
    int maxid = 0;
    for (auto &p: posts) if (p.id > maxid) maxid = p.id;
    return maxid + 1;
}

std::vector<std::string> Database::split(const std::string &s, char d) {
    std::vector<std::string> out;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '"') {
            if (inQuotes && i + 1 < s.size() && s[i+1] == '"') {
                // escaped quote
                cur.push_back('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == d && !inQuotes) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

std::string Database::join(const std::vector<std::string> &parts, char d) {
    std::ostringstream ss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) ss << d;
        const std::string &f = parts[i];
        bool needQuotes = f.find(d) != std::string::npos || f.find('"') != std::string::npos || f.find('\n') != std::string::npos;
        if (!needQuotes) ss << f;
        else {
            ss << '"';
            for (char c: f) {
                if (c == '"') ss << '"' << '"'; else ss << c;
            }
            ss << '"';
        }
    }
    return ss.str();
}

bool Database::load() {
    users.clear(); posts.clear();
    // ensure data directory exists
    try { std::filesystem::create_directories(dir); } catch(...) {}

    // ensure files exist
    if (!std::filesystem::exists(usersPath())) {
        std::ofstream out(usersPath()); out.close();
    }
    if (!std::filesystem::exists(postsPath())) {
        std::ofstream out(postsPath()); out.close();
    }

    std::ifstream fu(usersPath());
    if (!fu.is_open()) return false;
    std::string line;
    while (std::getline(fu, line)) {
        if (line.empty()) continue;
        auto cols = split(line, ',');
        User u;
        try { u.id = std::stoi(cols[0]); } catch(...) { continue; }
        if (cols.size()>1) u.name = cols[1];
        if (cols.size()>2) u.bio = cols[2];
        if (cols.size()>3 && !cols[3].empty()) {
            auto fr = split(cols[3], ';');
            for (auto &s: fr) {
                try { u.friends.push_front(std::stoi(s)); } catch(...) {}
            }
        }
        users.push_back(u);
    }
    fu.close();

    std::ifstream fp(postsPath());
    if (!fp.is_open()) return false;
    while (std::getline(fp, line)) {
        if (line.empty()) continue;
        auto cols = split(line, ',');
        Post p;
        try { p.id = std::stoi(cols[0]); } catch(...) { continue; }
        try { p.authorId = std::stoi(cols[1]); } catch(...) { p.authorId = 0; }
        if (cols.size()>2) p.timestamp = cols[2];
        if (cols.size()>3) p.content = cols[3];
        if (cols.size()>4 && !cols[4].empty()) {
            auto lk = split(cols[4], ';');
            for (auto &s: lk) {
                try { p.likes.push_back(std::stoi(s)); } catch(...) {}
            }
        }
        posts.push_back(p);
    }
    fp.close();
    return true;
}

bool Database::save() {
    std::ofstream fu(usersPath(), std::ios::trunc);
    if (!fu.is_open()) return false;
    for (auto &u: users) {
        std::vector<std::string> fr;
        u.friends.for_each([&](const int &id){ fr.push_back(std::to_string(id)); });
        fu<<u.id<<","<<u.name<<","<<u.bio<<","<<join(fr,';')<<"\n";
    }
    fu.close();

    std::ofstream fp(postsPath(), std::ios::trunc);
    if (!fp.is_open()) return false;
    for (auto &p: posts) {
        std::vector<std::string> lk;
        for (auto id: p.likes) lk.push_back(std::to_string(id));
        fp<<p.id<<","<<p.authorId<<","<<p.timestamp<<","<<p.content<<","<<join(lk,';')<<"\n";
    }
    fp.close();
    return true;
}
