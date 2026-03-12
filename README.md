# vibe — simple journaling

**Version 1.2** — Lightweight journaling app built in C++ using GTK+ 3 and SQLite.

## Requirements
- GTK+ 3.0 development libraries
- SQLite3 development libraries
- On Linux: `libgtk-3-dev`, `libsqlite3-dev`, `pkg-config`

Install dependencies (Ubuntu/Debian):

```bash
sudo apt update && sudo apt install libgtk-3-dev libsqlite3-dev pkg-config build-essential
```

## Build

```bash
# C++ version (current, recommended)
make cpp      # builds vj executable

# OR C version (legacy)
./build-c.sh
```

## Run

```bash
./vj            # C++ binary (v1.2)
# or
./vibe-journal  # C binary (legacy)
```

## Features

### Journaling
- Create, edit, and delete journal entries
- Search across title, content, and tags
- Tag support (comma-separated)
- Portable SQLite database storage (same folder as executable)

### UI & Theming
- Dark mode toggle with CSS theming
- Resizable paned layout: adjust list/editor width via divider drag
- Date/Title columns with auto-sizing and ellipsize
- WordPerfect 5.1-inspired color scheme (deep blue default, black dark mode)

### Editing & Export
- Font size selector (10pt–18pt)
- Search highlighting in editor
- Export entries to Markdown files
- Autosave drafts every 5 minutes

### Preferences & Persistence
- Persistent preferences (dark mode, font size, window size, pane position)
- Window state saved on close, restored on launch
- Keyboard accelerators (Ctrl+S to save, etc.)

### Architecture
- Single-file C++ application using GTK+ C API
- RAII pattern for resource management
- Encapsulated in `JournalApp` class
- No external dependencies beyond GTK+ and SQLite
- Fully portable across Linux systems

## Notes
- Original Python/Tkinter version ported to C, now rewritten in C++/GTK+
- Database file: `vibe_journal.db` (created in app directory)
- See **CHANGELOG.md** for version history and detailed feature changes
