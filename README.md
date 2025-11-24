# MiniSocialDS (C++ Console)

This is a modular C++ implementation of a simplified social media feed system for the Data Structures lab project.

Build (Windows with g++):

```powershell
g++ -std=c++17 -Iinclude src\*.cpp -o MiniSocialDS.exe
```

Run:

```powershell
.\MiniSocialDS.exe
```

Data files are in `data/` as CSV: `users.csv` and `posts.csv`.

Features implemented:
- User create/view/edit (simple)
- Friends stored in a linked list (in-memory)
- Posts stored in CSV, queue-based FIFO feed
- Trending (sort by likes)
- Like/unlike, delete, undo (stack)
- Search by keyword

Project structure:
- `include/` headers
- `src/` implementation
- `data/` CSV persistence
