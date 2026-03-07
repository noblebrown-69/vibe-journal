# vibe — simple journaling

Lightweight journaling app using Tkinter and SQLite.

Requirements
- Python 3
- On Linux: `python3-tk` (package providing Tkinter)

Run

```bash
python3 /home/franklin/vibe-journal/main.py
```

Build a single-file executable (Linux/Ubuntu)

1. Install pipx if missing:

```bash
sudo apt update && sudo apt install pipx
```

2. Run the provided build script:

```bash
./build.sh
```

The resulting single-file executable will be at `dist/vibe-journal` (about 13MB).

Run the executable:

```bash
./dist/vibe-journal
```

Copy `dist/vibe-journal` to your Dropbox or any Ubuntu machine and run it directly (no Python needed).

Notes
- The app stores entries in `vibe_journal.db` in the same folder as the executable (for portability, e.g., in Dropbox).
- Search uses simple LIKE matching across title, content, and tags.
- Tags are stored as a comma-separated string on save.

If you want, I can:
- add export/import (JSON)
- use per-day files instead of SQLite
- add quick keyboard shortcuts
