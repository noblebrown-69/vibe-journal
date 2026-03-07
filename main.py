#!/usr/bin/env python3
"""vibe — simple journaling text editor with SQLite indexing

Lightweight, portable: uses Tkinter and Python stdlib only.
"""
import sqlite3
import datetime
from pathlib import Path
import sys
try:
    import tkinter as tk
    from tkinter import ttk, messagebox
except Exception:
    print("Tkinter is required. On Linux install python3-tk.")
    raise

DB_PATH = Path(sys.executable).parent / 'vibe_journal.db'


def init_db(conn):
    cur = conn.cursor()
    cur.execute(
        """CREATE TABLE IF NOT EXISTS entries(
            id INTEGER PRIMARY KEY,
            title TEXT,
            content TEXT,
            tags TEXT,
            created TIMESTAMP
        )"""
    )
    conn.commit()


class VibeApp:
    def __init__(self, root, conn):
        self.root = root
        self.conn = conn
        root.title('vibe — journaling')
        root.geometry('900x600')

        # WordPerfect-style color scheme
        wp_bg = '#003366'  # deep blue
        wp_text = '#FFFFFF'  # white text
        wp_panel = '#005f87'  # panel shade
        wp_select_bg = '#FFFF00'  # selection highlight (yellow)
        style = ttk.Style()
        try:
            style.theme_use('clam')
        except Exception:
            pass
        style.configure('TFrame', background=wp_bg)
        style.configure('TLabel', background=wp_bg, foreground=wp_text)
        style.configure('TButton', background=wp_panel, foreground=wp_text)
        style.configure('TEntry', fieldbackground=wp_bg, foreground=wp_text)
        root.configure(bg=wp_bg)

        self.ids = []

        paned = ttk.Panedwindow(root, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True)

        left = ttk.Frame(paned, width=280)
        right = ttk.Frame(paned)
        paned.add(left, weight=1)
        paned.add(right, weight=4)

        # Left: search + list
        search_frame = ttk.Frame(left)
        search_frame.pack(fill=tk.X, padx=6, pady=6)
        self.search_var = tk.StringVar()
        search_entry = ttk.Entry(search_frame, textvariable=self.search_var)
        search_entry.pack(side=tk.LEFT, fill=tk.X, expand=True)
        search_entry.bind('<Return>', lambda e: self.refresh_list())
        ttk.Button(search_frame, text='Search', command=self.refresh_list).pack(side=tk.LEFT, padx=4)

        self.listbox = tk.Listbox(left, bg=wp_bg, fg=wp_text, selectbackground=wp_select_bg, selectforeground=wp_bg)
        self.listbox.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)
        self.listbox.bind('<<ListboxSelect>>', lambda e: self.on_select())

        btn_frame = ttk.Frame(left)
        btn_frame.pack(fill=tk.X, padx=6, pady=6)
        ttk.Button(btn_frame, text='New', command=self.new_entry).pack(side=tk.LEFT)
        ttk.Button(btn_frame, text='Delete', command=self.delete_entry).pack(side=tk.LEFT, padx=6)

        # Right: editor
        meta = ttk.Frame(right)
        meta.pack(fill=tk.X, padx=6, pady=6)
        ttk.Label(meta, text='Title').pack(anchor=tk.W)
        self.title_var = tk.StringVar()
        ttk.Entry(meta, textvariable=self.title_var).pack(fill=tk.X)
        ttk.Label(meta, text='Tags (comma-separated)').pack(anchor=tk.W, pady=(6,0))
        self.tags_var = tk.StringVar()
        ttk.Entry(meta, textvariable=self.tags_var).pack(fill=tk.X)

        self.text = tk.Text(right, bg=wp_bg, fg=wp_text, insertbackground=wp_text, selectbackground=wp_select_bg, selectforeground=wp_bg)
        self.text.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        save_frame = ttk.Frame(right)
        save_frame.pack(fill=tk.X, padx=6, pady=6)
        ttk.Button(save_frame, text='Save', command=self.save_entry).pack(side=tk.LEFT)
        ttk.Button(save_frame, text='Refresh List', command=self.refresh_list).pack(side=tk.LEFT, padx=6)

        self.current_id = None
        self.refresh_list()
        # Populate the editor with a new, timestamped title by default
        self.new_entry()

    def fetch_entries(self, query=None):
        cur = self.conn.cursor()
        if query:
            q = f"%{query}%"
            cur.execute(
                "SELECT id, title, created, tags FROM entries WHERE title LIKE ? OR content LIKE ? OR tags LIKE ? ORDER BY created DESC",
                (q, q, q),
            )
        else:
            cur.execute("SELECT id, title, created, tags FROM entries ORDER BY created DESC")
        return cur.fetchall()

    def refresh_list(self):
        q = self.search_var.get().strip()
        rows = self.fetch_entries(q if q else None)
        self.listbox.delete(0, tk.END)
        self.ids = []
        for r in rows:
            eid, title, created, tags = r
            title = title or '(untitled)'
            created = created.split('.')[0] if isinstance(created, str) else str(created)
            display = f"{created} — {title}"
            if tags:
                display += f"  [{tags}]"
            self.listbox.insert(tk.END, display)
            self.ids.append(eid)

    def on_select(self):
        sel = self.listbox.curselection()
        if not sel:
            return
        idx = sel[0]
        eid = self.ids[idx]
        cur = self.conn.cursor()
        cur.execute('SELECT title, content, tags FROM entries WHERE id=?', (eid,))
        row = cur.fetchone()
        if row:
            title, content, tags = row
            self.current_id = eid
            self.title_var.set(title or '')
            self.tags_var.set(tags or '')
            self.text.delete('1.0', tk.END)
            self.text.insert(tk.END, content or '')

    def new_entry(self):
        self.current_id = None
        # Auto-populate title with current date/time (editable)
        now_title = datetime.datetime.now().isoformat(sep=' ', timespec='seconds')
        self.title_var.set(now_title)
        self.tags_var.set('')
        self.text.delete('1.0', tk.END)

    def save_entry(self):
        title = self.title_var.get().strip()
        tags = self.tags_var.get().strip()
        content = self.text.get('1.0', tk.END).rstrip('\n')
        now = datetime.datetime.now().isoformat(sep=' ', timespec='seconds')
        if not title:
            title = now
        cur = self.conn.cursor()
        if self.current_id:
            cur.execute(
                'UPDATE entries SET title=?, content=?, tags=? WHERE id=?',
                (title, content, tags, self.current_id),
            )
        else:
            cur.execute(
                'INSERT INTO entries(title, content, tags, created) VALUES(?,?,?,?)',
                (title, content, tags, now),
            )
            self.current_id = cur.lastrowid
        self.conn.commit()
        self.refresh_list()
        messagebox.showinfo('Saved', 'Entry saved.')

    def delete_entry(self):
        sel = self.listbox.curselection()
        if not sel:
            messagebox.showwarning('Delete', 'No entry selected.')
            return
        idx = sel[0]
        eid = self.ids[idx]
        if not messagebox.askyesno('Delete', 'Delete selected entry?'):
            return
        cur = self.conn.cursor()
        cur.execute('DELETE FROM entries WHERE id=?', (eid,))
        self.conn.commit()
        self.new_entry()
        self.refresh_list()


def main():
    db = DB_PATH
    conn = sqlite3.connect(str(db))
    init_db(conn)
    root = tk.Tk()
    app = VibeApp(root, conn)
    try:
        root.mainloop()
    finally:
        conn.close()


if __name__ == '__main__':
    print("Launching vibe-journal from:", __file__)
    main()
