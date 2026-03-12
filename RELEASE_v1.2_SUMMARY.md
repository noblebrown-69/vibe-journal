# Vibe Journal v1.2 Release Summary

## Session Overview: March 11, 2026

This session focused on **UX polish, layout flexibility, and state persistence** for the C++/GTK+ journaling app.

### Major Changes

#### 1. **Resizable Paned Layout**
- Replaced fixed GtkGrid with GtkPaned for dynamic left/right split
- User can now drag divider to adjust entry list vs. editor width
- Left pane: scrollable TreeView with date/title columns
- Right pane: vertical box with editor components
- Initial divider position: 280px

#### 2. **Window State Persistence** ⭐
- Saves window dimensions and paned divider position on close
- Restores state on app launch via `load_window_state()`
- delete-event handler captures state just before window destruction
- Values stored in SQLite prefs table with sane minimums (200×200px, 100px pane)

#### 3. **Improved TreeView Display**
- Separate Date and Title columns (instead of combined string)
- Date: fixed 130px width, "YYYY-MM-DD HH:MM" format (compressed timestamp)
- Title: expandable column with auto-sizing and ellipsize for long names
- Fixed height mode enabled for faster rendering
- Minimum title column width: 200px

#### 4. **Build System Updates**
- Executable renamed: `vibe-journal-cpp` → `vj` (shorter, CLI-friendly)
- Makefile: C++ target now uses native `gtk+-3.0` instead of gtkmm
- Added `-std=c++11` flag explicitly
- Cleaner build configuration

#### 5. **Application Title**
- Window title: "vibe — journaling" → "Vibe Journal"

### Code Quality
- **RAII maintained**: resource cleanup in destructor
- **GTK signal lifecycle**: delete-event hook for graceful shutdown
- **Persistent storage**: SQLite REPLACE INTO for prefs (idempotent)
- **Type safety**: `static_cast<>` in callbacks, std::to_string() for int→string
- Class member additions `paned_`, `saved_*` variables for state tracking

### Build & Test
✅ Compiles cleanly with `make cpp`  
✅ Binary: `/home/franklin/vibe-journal/vj` (804KB, static linked)  
⚠️ Pre-existing deprecation warnings: gtk_misc_set_alignment (non-blocking)

### Files Modified/Added

**Modified:**
- `Makefile` — executable name, compiler flags
- `README.md` — v1.2 features, build instructions, detailed feature list
- `build-c.sh` — documentation note
- `main.cpp` — 600+ lines of new window state logic, paned layout, refactored TreeView

**New:**
- `CHANGELOG.md` — comprehensive version history (1.1 → 1.2)
- `.gitignore` — build artifacts, database, environment, IDE files

**Not committed (by design):**
- Binary artifacts: `vj`, `vibe-journal`, `vibe-journal-cpp`
- Database: `vibe_journal.db`
- Docs: `vibe_journal_summary.docx`
- Environment: `.venv/`

### Recommended GitHub Push Steps

```bash
# 1. Stage all changes
git add -A

# 2. Verify staging (should show main.cpp, Makefile, README.md, CHANGELOG.md, .gitignore)
git status

# 3. Commit with v1.2 message
git commit -m "v1.2: Resizable paned layout, window state persistence, improved TreeView

- Add GtkPaned for flexible left/right split with user-adjustable divider
- Implement window geometry & pane position persistence across sessions
- Refactor TreeView: separate Date (130px) and Title (expandable) columns
- Compress date format to YYYY-MM-DD HH:MM (16 chars)
- Rename executable to 'vj' for short CLI runs
- Update Makefile: gtk+-3.0 native, explicit -std=c++11
- Add window state methods: load_window_state(), save_window_state()
- Add delete-event handler for graceful state capture on close
- Update README with v1.2 features and build instructions
- Add comprehensive CHANGELOG with version history
- Add .gitignore for build artifacts and environment files

Fixes:
- Window state (size, layout) now saved/restored per session
- TreeView no longer shows redundant/combined date-title strings"

# 4. Push to GitHub
git push origin master
```

### What Users Get in v1.2

1. **Professional Layout**: Drag divider to resize list/editor
2. **Memory**: App remembers window size & layout from last session
3. **Clarity**: Date and Title now separate, easier to scan
4. **Speed**: Fixed-height TreeView rendering faster on large lists
5. **Polish**: Compact executable name, cleaner build system

---

**This release represents a significant UX improvement while maintaining code simplicity and portability.**
