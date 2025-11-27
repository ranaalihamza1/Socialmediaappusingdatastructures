#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <limits>
#include <thread>
#include "../include/Database.h"
#include "../include/Feed.h"
#include "../include/History.h"
#include "../include/User.h"

using namespace std;

// ============================================================================
// COLOR CODES FOR BEAUTIFUL OUTPUT
// ============================================================================

const string RESET = "\033[0m";
const string BOLD = "\033[1m";
const string DIM = "\033[2m";

// Foreground Colors
const string BLACK = "\033[30m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";

// Bright Colors
const string BRIGHT_RED = "\033[91m";
const string BRIGHT_GREEN = "\033[92m";
const string BRIGHT_YELLOW = "\033[93m";
const string BRIGHT_BLUE = "\033[94m";
const string BRIGHT_MAGENTA = "\033[95m";
const string BRIGHT_CYAN = "\033[96m";

// Background Colors
const string BG_BLUE = "\033[44m";
const string BG_CYAN = "\033[46m";

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void displayLogo() {
    cout << BRIGHT_BLUE << BOLD;
    cout << R"(  __  __ _       _        _____           _ _   ____  _____
 |  \/  (_)_ __ (_)_ __  |  ___|_ _  __ _ (_) |_|  _ \| ____|
 | |\/| | | '_ \| | '_ \ | |_ / _` |/ _` || | __| | | |  _|  
 | |  | | | | | | | | | ||  _| (_| | (_| || | |_| |_| | |___ 
 |_|  |_|_|_| |_|_|_| |_||_|  \__,_|\__, |/ |_|\__|____/|_____|
                                    |___/ |__/               )" << RESET << "\n";
    cout << BRIGHT_CYAN << BOLD << "               MiniSocialDS - Data Structures Feed Demo\n" << RESET;
    cout << "\n";
}

static void clr() {
    clearScreen();
    displayLogo();
}

// Loading animation with colors
static void showLoading(const string &message = "Loading") {
    cout << BRIGHT_YELLOW << BOLD << message;
    cout.flush();
    for (int i = 0; i < 3; i++) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << BRIGHT_YELLOW << "." << RESET;
        cout.flush();
    }
    cout << RESET << "\n";
}

// Get current timestamp as string
string now_str() {
    auto t = time(nullptr);
    tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    ostringstream ss;
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Read a line and parse integer safely; returns -1 on parse failure
int promptInt(const string &msg) {
    while (true) {
        cout << BRIGHT_CYAN << msg << RESET;
        string line;
        if (!getline(cin, line))
            return -1;

        try {
            size_t idx;
            int v = stoi(line, &idx);
            if (idx != line.size())
                throw invalid_argument("trailing");
            return v;
        }
        catch (...) {
            cout << BRIGHT_RED << "Please enter a valid integer.\n" << RESET;
        }
    }
}

// Read user's menu choice
int readChoice() {
    string line;
    if (!getline(cin, line))
        return -1;
    try {
        return stoi(line);
    }
    catch (...) {
        return -1;
    }
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    // ========== INITIALIZATION ==========
    string dataDir = ".";
    Database db(dataDir + "\\data");
    db.load();

    // Seed sample data if database is empty
    if (db.users.empty() && db.posts.empty()) {
        try {
            filesystem::create_directories(dataDir + "\\data");
        }
        catch (...) {}

        User u;
        u.id = 1;
        u.username = "ali";
        u.name = "Ali";
        u.bio = "CS student";
        u.password = "alipass";
        db.users.push_back(u);

        User v;
        v.id = 2;
        v.username = "babar";
        v.name = "Babar";
        v.bio = "DS enthusiast";
        v.password = "babarpass";
        db.users.push_back(v);

        Post p;
        p.id = 1;
        p.authorId = 1;
        p.timestamp = now_str();
        p.content = "Hello from Ali";
        db.posts.push_back(p);

        db.save();
    }

    FeedManager feed(db);
    History history;
    int currentUser = -1; // not logged in

    // ========== HELPER LAMBDAS ==========

    // Authenticate user by username and password
    auto authenticate = [&](const string &username, const string &password) -> int {
        for (auto &u : db.users) {
            if (u.username == username && u.password == password)
                return u.id;
        }
        return -1;
    };

    // Get username by user ID
    auto getUsernameById = [&](int id) -> string {
        for (auto &u : db.users) {
            if (u.id == id)
                return u.username.empty() ? u.name : u.username;
        }
        return "(unknown)";
    };

    // Get display name by user ID
    auto getDisplayNameById = [&](int id) -> string {
        for (auto &u : db.users) {
            if (u.id == id)
                return u.name.empty() ? u.username : u.name;
        }
        return "(unknown)";
    };

    // Find user ID by username
    auto findUserIdByUsername = [&](const string &username) -> int {
        for (auto &u : db.users) {
            if (u.username == username)
                return u.id;
        }
        return -1;
    };

    // Display feed preview (top 3 trending posts)
    auto displayFeedPreview = [&]() {
        feed.buildQueue();
        auto preview = feed.getTrending(3);
        if (preview.empty())
            return;

        cout << BRIGHT_CYAN << BOLD << "\n━━━ Feed Preview (top posts) ━━━\n" << RESET;
        for (auto &p : preview) {
            cout << BRIGHT_MAGENTA << "[" << p.id << "] " << RESET
                 << BRIGHT_YELLOW << "(" << p.likes.size() << " likes) " << RESET
                 << GREEN << p.content << RESET
                 << BRIGHT_CYAN << " — " << getDisplayNameById(p.authorId) << "\n" << RESET;
        }
        cout << BRIGHT_CYAN << BOLD << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << RESET;
    };

    // ========== MAIN LOOP ==========

    while (true) {
        clr();
        cout << BRIGHT_BLUE << BOLD << "MiniSocialDS - Console UI\n" << RESET;

        if (currentUser > 0) {
            // ===== LOGGED IN MENU =====
            cout << BRIGHT_GREEN << BOLD << "✓ Logged in as: " << RESET
                 << BRIGHT_CYAN << getUsernameById(currentUser)
                 << RESET << BRIGHT_GREEN << " (" << RESET
                 << BRIGHT_CYAN << getDisplayNameById(currentUser) << RESET
                 << BRIGHT_GREEN << ")\n" << RESET;
            displayFeedPreview();

            cout << BRIGHT_MAGENTA << BOLD << "╔════════════════════════════════╗\n";
            cout << "║      MAIN MENU (LOGGED IN)     ║\n";
            cout << "╚════════════════════════════════╝\n" << RESET;

            cout << YELLOW << "1) Logout\n";
            cout << "2) View profile\n";
            cout << "3) My posts\n";
            cout << "4) Friends\n";
            cout << "5) Create post\n";
            cout << "6) View feed (Full)\n";
            cout << "7) Trending (by likes)\n";
            cout << "8) Search posts\n";
            cout << "9) Like post\n";
            cout << "10) Delete post\n";
            cout << "11) Undo last\n";
            cout << "12) Save & Exit\n" << RESET;
            cout << BRIGHT_CYAN << "Choice: " << RESET;

            int c = readChoice();
            if (c < 0) {
                cout << BRIGHT_RED << "Invalid choice.\n" << RESET;
                continue;
            }

            // ===== OPTION 1: LOGOUT =====
            if (c == 1) {
                currentUser = -1;
                cout << BRIGHT_GREEN << BOLD << "✓ Logged out successfully.\n" << RESET;
                cin.get();
                continue;
            }

            // ===== OPTION 2: VIEW PROFILE =====
            if (c == 2) {
                auto it = find_if(db.users.begin(), db.users.end(),
                                  [&](const User &u) { return u.id == currentUser; });
                if (it == db.users.end()) {
                    cout << "User not found\n";
                }
                else {
                    cout << "Name: " << it->name << "\n";
                    cout << "Username: " << it->username << "\n";
                    cout << "Bio: " << it->bio << "\n";
                    cout << "Friends:\n";
                    it->friends.for_each([&](const int &fid) {
                        cout << " - " << getDisplayNameById(fid) << " ("
                             << getUsernameById(fid) << ")\n";
                    });
                }
                cout << "Press Enter...";
                cin.get();
                continue;
            }

            // ===== OPTION 3: MY POSTS =====
            if (c == 3) {
                cout << "-- My Posts --\n";
                bool any = false;
                for (auto &p : db.posts) {
                    if (p.authorId == currentUser) {
                        any = true;
                        cout << "[" << p.id << "] (" << p.likes.size()
                             << " likes) " << p.content << "\n";
                    }
                }
                if (!any)
                    cout << "You have no posts.\n";

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 4: FRIENDS =====
            if (c == 4) {
                while (true) {
                    cout << "-- Friends --\n";
                    cout << "1) View friends\n";
                    cout << "2) Add friend\n";
                    cout << "3) Remove friend\n";
                    cout << "4) Back\n";

                    int fc = promptInt("Choice: ");

                    // View friends
                    if (fc == 1) {
                        auto it = find_if(db.users.begin(), db.users.end(),
                                          [&](const User &u) { return u.id == currentUser; });
                        if (it == db.users.end()) {
                            cout << "User not found\n";
                        }
                        else {
                            it->friends.for_each([&](const int &fid) {
                                cout << " - " << getDisplayNameById(fid) << " ("
                                     << getUsernameById(fid) << ")\n";
                            });
                        }
                        cout << "Enter to continue";
                        cin.get();
                    }

                    // Add friend
                    else if (fc == 2) {
                        string uname;
                        cout << "Enter username to add: ";
                        getline(cin, uname);

                        int fid = findUserIdByUsername(uname);
                        if (fid <= 0) {
                            cout << "User not found\n";
                            cin.get();
                            continue;
                        }
                        if (fid == currentUser) {
                            cout << "Cannot add yourself.\n";
                            cin.get();
                            continue;
                        }

                        auto uit = find_if(db.users.begin(), db.users.end(),
                                           [&](const User &u) { return u.id == currentUser; });
                        if (uit->friends.contains(fid)) {
                            cout << "Already friends.\n";
                            cin.get();
                            continue;
                        }

                        uit->friends.push_front(fid);
                        db.save();
                        cout << "Friend added.\n";
                        cin.get();
                    }

                    // Remove friend
                    else if (fc == 3) {
                        string uname;
                        cout << "Enter username to remove: ";
                        getline(cin, uname);

                        int fid = findUserIdByUsername(uname);
                        if (fid <= 0) {
                            cout << "User not found\n";
                            cin.get();
                            continue;
                        }

                        auto uit = find_if(db.users.begin(), db.users.end(),
                                           [&](const User &u) { return u.id == currentUser; });
                        if (!uit->friends.remove(fid)) {
                            cout << "Friend not in list.\n";
                            cin.get();
                            continue;
                        }

                        db.save();
                        cout << "Friend removed.\n";
                        cin.get();
                    }

                    // Back to main menu
                    else {
                        break;
                    }
                }
                continue;
            }

            // ===== OPTION 5: CREATE POST =====
            if (c == 5) {
                Post p;
                p.id = db.nextPostId();
                p.authorId = currentUser;
                p.timestamp = now_str();

                cout << BRIGHT_CYAN << "Post content: " << RESET;
                getline(cin, p.content);

                if (p.content.empty()) {
                    cout << BRIGHT_RED << "✗ Post cannot be empty.\n" << RESET;
                    cin.get();
                    continue;
                }

                db.posts.push_back(p);
                feed.buildQueue();
                history.push({"create", currentUser, p.id, p.content});
                db.save();

                showLoading(BRIGHT_GREEN + "Creating post");
                cout << BRIGHT_GREEN << BOLD << "✓ Posted successfully (id=" << p.id << ")\n" << RESET;
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 6: VIEW FEED (FIFO) =====
            if (c == 6) {
                feed.buildQueue();
                auto arr = feed.getFIFO(50);
                for (auto &p : arr) {
                    cout << "[" << p.id << "] (" << p.likes.size()
                         << " likes) " << p.content << " — "
                         << getDisplayNameById(p.authorId) << "\n";
                }
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 7: TRENDING (BY LIKES) =====
            if (c == 7) {
                auto arr = feed.getTrending(50);
                for (auto &p : arr) {
                    cout << "[" << p.id << "] (" << p.likes.size()
                         << " likes) " << p.content << " — "
                         << getDisplayNameById(p.authorId) << "\n";
                }
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 8: SEARCH POSTS =====
            if (c == 8) {
                cout << "Keyword: ";
                string k;
                getline(cin, k);
                auto r = feed.search(k);
                for (auto &p : r) {
                    cout << "[" << p.id << "] " << p.content << " — "
                         << getDisplayNameById(p.authorId) << "\n";
                }
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 9: LIKE POST =====
            if (c == 9) {
                int pid = promptInt("Post id to like: ");
                if (pid < 0) {
                    cout << BRIGHT_RED << "✗ Invalid post id\n" << RESET;
                    cin.get();
                    continue;
                }

                bool found = any_of(db.posts.begin(), db.posts.end(),
                                    [&](const Post &p) { return p.id == pid; });
                if (!found) {
                    cout << BRIGHT_RED << "✗ Post not found\n" << RESET;
                    cin.get();
                    continue;
                }

                if (feed.likePost(pid, currentUser)) {
                    showLoading(BRIGHT_GREEN + "Liking post");
                    cout << BRIGHT_GREEN << BOLD << "❤ Liked successfully!\n" << RESET;
                    history.push({"like", currentUser, pid, ""});
                    db.save();
                }
                else {
                    cout << BRIGHT_YELLOW << "⊙ Already liked or post not found\n" << RESET;
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 10: DELETE POST =====
            if (c == 10) {
                int pid = promptInt("Post id to delete: ");
                if (pid < 0) {
                    cout << "Invalid post id\n";
                    cin.get();
                    continue;
                }

                auto it = find_if(db.posts.begin(), db.posts.end(),
                                  [&](const Post &p) { return p.id == pid; });
                if (it != db.posts.end()) {
                    if (it->authorId != currentUser) {
                        cout << "You can only delete your own posts.\n";
                        cin.get();
                        continue;
                    }

                    string serialized = it->content;
                    if (feed.deletePost(pid)) {
                        history.push({"delete", currentUser, pid, serialized});
                        db.save();
                        cout << "Deleted\n";
                    }
                    else {
                        cout << "Delete failed\n";
                    }
                }
                else {
                    cout << "Not found\n";
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 11: UNDO LAST =====
            if (c == 11) {
                Action act;
                if (!history.undo(act)) {
                    cout << "Nothing to undo\n";
                    cin.get();
                    continue;
                }

                if (act.type == "like") {
                    if (feed.unlikePost(act.postId, act.userId))
                        db.save();
                    cout << "Undid like\n";
                }
                else if (act.type == "delete") {
                    Post p;
                    p.id = act.postId;
                    p.authorId = act.userId;
                    p.timestamp = now_str();
                    p.content = act.payload;
                    db.posts.push_back(p);
                    db.save();
                    cout << "Restored post id=" << p.id << "\n";
                }
                else if (act.type == "create") {
                    if (feed.deletePost(act.postId))
                        db.save();
                    cout << "Undid create\n";
                }
                else if (act.type == "view") {
                    cout << "Undid view (no-op)\n";
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 12: SAVE & EXIT =====
            if (c == 12) {
                showLoading(BRIGHT_YELLOW + "Saving data");
                db.save();
                cout << BRIGHT_GREEN << BOLD << "✓ Saved successfully. Exiting...\n" << RESET;
                break;
            }
        }
        else {
            // ===== GUEST (NOT LOGGED IN) MENU =====
            cout << BRIGHT_RED << BOLD << "✗ Not logged in. Please Login or Create an account.\n" << RESET;

            cout << BRIGHT_MAGENTA << BOLD << "╔════════════════════════════════╗\n";
            cout << "║      MAIN MENU (GUEST MODE)    ║\n";
            cout << "╚════════════════════════════════╝\n" << RESET;

            cout << YELLOW << "1) Login\n";
            cout << "2) Create user\n";
            cout << "3) View profile\n";
            cout << "4) Create post\n";
            cout << "5) View feed (FIFO)\n";
            cout << "6) Trending (by likes)\n";
            cout << "7) Search posts\n";
            cout << "8) Like post\n";
            cout << "9) Delete post\n";
            cout << "10) Undo last\n";
            cout << "11) Save & Exit\n" << RESET;
            cout << BRIGHT_CYAN << "Choice: " << RESET;

            int c = readChoice();
            if (c < 0) {
                cout << "Invalid choice\n";
                continue;
            }

            // ===== OPTION 1: LOGIN =====
            if (c == 1) {
                string username;
                cout << BRIGHT_CYAN << "Username: " << RESET;
                getline(cin, username);

                string password;
                cout << BRIGHT_CYAN << "Password: " << RESET;
                getline(cin, password);

                showLoading(BRIGHT_YELLOW + "Authenticating");
                int id = authenticate(username, password);
                if (id > 0) {
                    currentUser = id;
                    cout << BRIGHT_GREEN << BOLD << "✓ Logged in as " << username << "\n" << RESET;
                }
                else {
                    cout << BRIGHT_RED << "✗ Invalid credentials\n" << RESET;
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 2: CREATE USER =====
            if (c == 2) {
                User u;
                u.id = db.nextUserId();

                cout << BRIGHT_CYAN << "Choose a username: " << RESET;
                getline(cin, u.username);

                if (u.username.empty()) {
                    cout << BRIGHT_RED << "✗ Username cannot be empty\n" << RESET;
                    cin.get();
                    continue;
                }

                bool exists = any_of(db.users.begin(), db.users.end(),
                                     [&](const User &x) { return x.username == u.username; });
                if (exists) {
                    cout << BRIGHT_RED << "✗ Username already taken\n" << RESET;
                    cin.get();
                    continue;
                }

                cout << BRIGHT_CYAN << "Password: " << RESET;
                getline(cin, u.password);
                if (u.password.empty()) {
                    cout << BRIGHT_RED << "✗ Password cannot be empty\n" << RESET;
                    cin.get();
                    continue;
                }

                cout << BRIGHT_CYAN << "Name: " << RESET;
                getline(cin, u.name);
                if (u.name.empty())
                    u.name = u.username;

                cout << BRIGHT_CYAN << "Bio: " << RESET;
                getline(cin, u.bio);

                db.users.push_back(u);
                db.save();
                currentUser = u.id;

                showLoading(BRIGHT_YELLOW + "Creating account");
                cout << BRIGHT_GREEN << BOLD << "✓ User created and logged in (id=" << u.id << ")\n" << RESET;
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 3: VIEW PROFILE =====
            if (c == 3) {
                auto it = find_if(db.users.begin(), db.users.end(),
                                  [&](const User &u) { return u.id == currentUser; });
                if (it == db.users.end()) {
                    cout << "User not found\n";
                }
                else {
                    cout << "Name: " << it->name << "\n";
                    cout << "Bio: " << it->bio << "\n";
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 4: CREATE POST =====
            if (c == 4) {
                Post p;
                p.id = db.nextPostId();
                p.authorId = currentUser;
                p.timestamp = now_str();

                cout << "Post content: ";
                getline(cin, p.content);

                if (p.content.empty()) {
                    cout << "Post cannot be empty\n";
                    cin.get();
                    continue;
                }

                db.posts.push_back(p);
                feed.buildQueue();
                history.push({"create", currentUser, p.id, p.content});
                db.save();

                cout << "Posted id=" << p.id << "\n";
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 5: VIEW FEED (FIFO) =====
            if (c == 5) {
                feed.buildQueue();
                auto arr = feed.getFIFO(50);
                for (auto &p : arr) {
                    cout << "[" << p.id << "] (" << p.likes.size()
                         << " likes) " << p.content << " — "
                         << getDisplayNameById(p.authorId) << "\n";
                }
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 6: TRENDING (BY LIKES) =====
            if (c == 6) {
                auto arr = feed.getTrending(50);
                for (auto &p : arr) {
                    cout << "[" << p.id << "] (" << p.likes.size()
                         << " likes) " << p.content << " — "
                         << getDisplayNameById(p.authorId) << "\n";
                }
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 7: SEARCH POSTS =====
            if (c == 7) {
                cout << "Keyword: ";
                string k;
                getline(cin, k);
                auto r = feed.search(k);
                for (auto &p : r) {
                    cout << "[" << p.id << "] " << p.content << " — "
                         << getDisplayNameById(p.authorId) << "\n";
                }
                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 8: LIKE POST =====
            if (c == 8) {
                int pid = promptInt("Post id to like: ");
                if (pid < 0) {
                    cout << "Invalid post id\n";
                    cin.get();
                    continue;
                }

                if (feed.likePost(pid, currentUser)) {
                    history.push({"like", currentUser, pid, ""});
                    db.save();
                    cout << "Liked\n";
                }
                else {
                    cout << "Already liked or not found\n";
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 9: DELETE POST =====
            if (c == 9) {
                cout << "You must be logged in to delete posts.\n";
                cin.get();
                continue;
            }

            // ===== OPTION 10: UNDO LAST =====
            if (c == 10) {
                Action act;
                if (!history.undo(act)) {
                    cout << "Nothing to undo\n";
                    cin.get();
                    continue;
                }

                if (act.type == "like") {
                    feed.unlikePost(act.postId, act.userId);
                    db.save();
                    cout << "Undid like\n";
                }
                else if (act.type == "delete") {
                    Post p;
                    p.id = act.postId;
                    p.authorId = act.userId;
                    p.timestamp = now_str();
                    p.content = act.payload;
                    db.posts.push_back(p);
                    db.save();
                    cout << "Restored post id=" << p.id << "\n";
                }
                else if (act.type == "create") {
                    feed.deletePost(act.postId);
                    db.save();
                    cout << "Undid create\n";
                }
                else {
                    cout << "Undid view (no-op)\n";
                }

                cout << "Enter to continue";
                cin.get();
                continue;
            }

            // ===== OPTION 11: SAVE & EXIT =====
            if (c == 11) {
                db.save();
                cout << "Saved. Exiting.\n";
                break;
            }
        }
    }

    return 0;
}
