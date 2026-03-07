# vibe — simple journaling

Lightweight journaling app built in C using GTK+ and SQLite.

Requirements
- GTK+ 3.0 development libraries
- SQLite3 development libraries
- On Linux: `libgtk-3-dev`, `libsqlite3-dev`, `pkg-config`

Install dependencies (Ubuntu/Debian):

```bash
sudo apt update && sudo apt install libgtk-3-dev libsqlite3-dev pkg-config build-essential
```

Build

```bash
./build-c.sh
```

Run

```bash
./vibe-journal
```

Features
- Create, edit, and delete journal entries
- Search across title, content, and tags
- Tag support (comma-separated)
- WordPerfect 5.1-inspired color scheme (deep blue background, white text)
- Copy/paste functionality via keyboard shortcuts (Ctrl+C/Ctrl+V/Ctrl+X) and right-click menu
- Portable SQLite database storage (in same folder as executable)

The app stores entries in `vibe_journal.db` in the same folder as the executable.

Notes
- This is a C port of the original Python/Tkinter version
- Maintains the same functionality and user interface design
- Fully portable - can be copied to any Linux system with GTK+ installed
