#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <limits>
#include "../include/Database.h"
#include "../include/Feed.h"
#include "../include/History.h"
#include "../include/User.h"

static void clr() { std::cout << "\n----------------------------------------\n"; }

std::string now_str() {
    auto t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss; ss<<std::put_time(&tm, "%Y-%m-%d %H:%M:%S"); return ss.str();
}

// Read a line and parse integer safely; returns -1 on parse failure
int promptInt(const std::string &msg) {
    while (true) {
        std::cout << msg;
        std::string line;
        if (!std::getline(std::cin, line)) return -1;
        try {
            size_t idx;
            int v = std::stoi(line, &idx);
            if (idx != line.size()) throw std::invalid_argument("trailing");
            return v;
        } catch (...) {
            std::cout << "Please enter a valid integer.\n";
        }
    }
}

int readChoice() {
    std::string line;
    if (!std::getline(std::cin, line)) return -1;
    try { return std::stoi(line); } catch(...) { return -1; }
}

int main(){
    std::string dataDir = ".";
    Database db(dataDir + "\\data");
    db.load();
    // if no users/posts present, seed sample data
    if (db.users.empty() && db.posts.empty()) {
        std::cout << "No DB found or empty — creating sample data in ./data\n";
        try { std::filesystem::create_directories(dataDir + "\\data"); } catch(...) {}
        User u; u.id = 1; u.name = "Alice"; u.bio = "CS student"; db.users.push_back(u);
        User v; v.id = 2; v.name = "Bob"; v.bio = "DS enthusiast"; db.users.push_back(v);
        Post p; p.id = 1; p.authorId = 1; p.timestamp = now_str(); p.content = "Hello from Alice"; db.posts.push_back(p);
        db.save();
    }

    FeedManager feed(db);
    History history;

    int currentUser = 1; // simple default

    while (true) {
        clr();
        std::cout<<"MiniSocialDS - Console UI\n";
        std::cout<<"Logged in as user id: "<<currentUser<<"\n";
        std::cout<<"1) Switch user  2) Create user  3) View profile\n";
        std::cout<<"4) Create post   5) View feed (FIFO)  6) Trending\n";
        std::cout<<"7) Search posts  8) Like post  9) Delete post\n";
        std::cout<<"10) Undo last    11) Save & Exit\n";
        std::cout << "Choice: ";
        int c = readChoice();
        if (c < 0) { std::cout << "Invalid choice. Please enter a number.\n"; continue; }

        if (c==1) {
            int id = promptInt("Enter user id: ");
            bool found = std::any_of(db.users.begin(), db.users.end(), [&](const User &u){ return u.id==id; });
            if (found) currentUser = id; else std::cout << "User id not found.\n";
        } else if (c==2) {
            User u; u.id = db.nextUserId();
            std::cout << "Name: "; std::getline(std::cin, u.name);
            if (u.name.empty()) { std::cout << "Name cannot be empty.\n"; std::cin.get(); continue; }
            std::cout << "Bio: "; std::getline(std::cin, u.bio);
            db.users.push_back(u); db.save(); std::cout<<"User created with id "<<u.id<<"\n";
        } else if (c==3) {
            auto it = std::find_if(db.users.begin(), db.users.end(), [&](const User &u){ return u.id==currentUser; });
            if (it==db.users.end()) { std::cout<<"User not found\n"; }
            else { std::cout<<"Name: "<<it->name<<"\nBio: "<<it->bio<<"\nFriends:\n"; it->friends.for_each([&](const int &fid){ std::cout<<" - "<<fid<<"\n"; }); }
            std::cout<<"Press Enter..."; std::cin.get();
        } else if (c==4) {
            Post p; p.id = db.nextPostId(); p.authorId = currentUser; p.timestamp = now_str();
            std::cout<<"Post content: "; std::getline(std::cin, p.content);
            if (p.content.empty()) { std::cout<<"Post cannot be empty.\n"; std::cin.get(); continue; }
            db.posts.push_back(p); feed.buildQueue();
            Action a{"create", currentUser, p.id, p.content}; history.push(a);
            db.save();
            std::cout<<"Posted id="<<p.id<<"\n"; std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==5) {
            feed.buildQueue(); auto arr = feed.getFIFO(20);
            for (auto &p: arr) {
                std::cout<<"["<<p.id<<"] ("<<p.likes.size()<<" likes) "<<p.content<<" — by "<<p.authorId<<"\n";
                Action a{"view", currentUser, p.id, ""}; history.push(a);
            }
            std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==6) {
            auto arr = feed.getTrending(10);
            for (auto &p: arr) std::cout<<"["<<p.id<<"] ("<<p.likes.size()<<" likes) "<<p.content<<"\n";
            std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==7) {
            std::cout<<"Keyword: "; std::string k; std::getline(std::cin, k);
            auto r = feed.search(k);
            for (auto &p: r) std::cout<<"["<<p.id<<"] "<<p.content<<"\n";
            std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==8) {
            int pid = promptInt("Post id to like: ");
            if (pid < 0) { std::cout<<"Invalid post id\n"; std::cin.get(); continue; }
            bool found = std::any_of(db.posts.begin(), db.posts.end(), [&](const Post &p){ return p.id==pid; });
            if (!found) { std::cout<<"Post not found\n"; std::cin.get(); continue; }
            if (feed.likePost(pid, currentUser)) { std::cout<<"Liked\n"; history.push({"like", currentUser, pid, ""}); db.save(); }
            else std::cout<<"Already liked or post not found\n";
            std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==9) {
            int pid = promptInt("Post id to delete: ");
            if (pid < 0) { std::cout<<"Invalid post id\n"; std::cin.get(); continue; }
            auto it = std::find_if(db.posts.begin(), db.posts.end(), [&](const Post &p){ return p.id==pid; });
            if (it!=db.posts.end()) {
                std::string serialized = it->content;
                if (feed.deletePost(pid)) {
                    Action a{"delete", currentUser, pid, serialized}; history.push(a); db.save();
                    std::cout<<"Deleted\n";
                } else std::cout<<"Delete failed\n";
            } else std::cout<<"Not found\n";
            std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==10) {
            Action act;
            if (!history.undo(act)) { std::cout<<"Nothing to undo\n"; std::cin.get(); continue; }
            if (act.type=="like") { if (feed.unlikePost(act.postId, act.userId)) { db.save(); } std::cout<<"Undid like\n"; }
            else if (act.type=="delete") {
                // restore
                Post p; p.id = act.postId; p.authorId = act.userId; p.timestamp = now_str(); p.content = act.payload;
                db.posts.push_back(p); db.save(); std::cout<<"Restored post id="<<p.id<<"\n";
            } else if (act.type=="create") {
                // undo create -> delete the created post
                if (feed.deletePost(act.postId)) { db.save(); std::cout<<"Undid create (deleted post)\n"; }
                else std::cout<<"Undo create failed (post missing)\n";
            } else if (act.type=="view") { std::cout<<"Undid view (no-op)\n"; }
            std::cout<<"Enter to continue"; std::cin.get();
        } else if (c==11) {
            db.save(); std::cout<<"Saved. Exiting.\n"; break;
        }
    }

    return 0;
}
