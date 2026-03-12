# Changelog

All notable changes to Vibe Journal are documented here.

## [1.2] - March 11, 2026

### Added
- **Window State Persistence**: Application now saves and restores window dimensions and paned divider position across sessions
  - Introduces `save_window_state()` and `load_window_state()` methods
  - Stores window width, height, and pane position in SQLite prefs table
  - delete-event handler captures state on application close
- **Resizable Paned Layout**: Replaced fixed Grid layout with GtkPaned for dynamic left/right split
  - User can now drag divider to adjust list vs. editor width
  - Initial divider position: 280px
  - Left pane: scrollable entry list
  - Right pane: vertical layout with title, tags, editor, buttons, statusbar
- **Improved TreeView Display**: Refactored entry list with separate columns
  - Date column (130px, fixed width): displays timestamp in "YYYY-MM-DD HH:MM" format
  - Title column (expandable): auto-sizes with ellipsize for long titles
  - Removed combined display string; cleaner data model
- **TreeView Performance Optimizations**:
  - Fixed height mode enabled (`gtk_tree_view_set_fixed_height_mode`)
  - Minimum title column width set to 200px for post-drag visibility
  - Faster rendering with uniform row heights
- **Build System Updates**:
  - Executable renamed from `vibe-journal-cpp` to `vj` (short CLI name, avoids conflicts)
  - Makefile updated to use native `gtk+-3.0` instead of gtkmm for C++ target
  - Added `-std=c++11` flag to CXXFLAGS for explicit standard compliance

### Changed
- Window title updated to "Vibe Journal" (was "vibe — journaling")
- Date formatting in TreeView compressed to compact "YYYY-MM-DD HH:MM" (was full timestamp with milliseconds before truncation)
- Paned widget now stored as member variable `paned_` for state access
- `load_preferences()` now also loads window geometry values (window_width, window_height, pane_position)

### Fixed
- TreeView now displays without duplicate/redundant date information per row

### Technical Details
- Class member additions: `paned_`, `saved_width_` (int), `saved_height_` (int), `saved_pane_pos_` (int)
- New static callback: `on_delete_event()` to trap window close signal
- State methods use SQLite REPLACE INTO prefs for transactional storage
- Sane minimums enforced: window ≥200×200px, pane position ≥100px
- RAII pattern maintained: state saved before window destruction

## [1.1] - Previous Release

### Features
- Create, edit, and delete journal entries
- Search across title, content, and tags
- Tag support (comma-separated)
- Dark mode toggle with CSS theming
- Font size selector (10pt–18pt)
- Autosave drafts every 5 minutes
- Export entries to Markdown
- Search highlighting in editor
- Keyboard accelerators (Ctrl+S to save, etc.)
- Persistent preferences (dark mode, font size)
- SQLite database storage
- Toolbar with New/Save/Delete/Export buttons

---

**Version 1.2** represents a major UX polish pass, focusing on layout flexibility, display clarity, and state persistence for a more professional desktop experience.
