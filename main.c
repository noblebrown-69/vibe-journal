#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>

// Global widgets
GtkWidget *window;
GtkWidget *search_entry;
GtkWidget *listbox;
GtkWidget *title_entry;
GtkWidget *tags_entry;
GtkWidget *text_view;
GtkListStore *list_store;
GtkTreeView *tree_view;
GtkTextBuffer *text_buffer;
sqlite3 *db;
char *db_path;

// Entry structure
typedef struct {
    int id;
    char *title;
    char *content;
    char *tags;
    char *created;
} Entry;

// Function declarations
void init_db();
void create_ui();
void apply_wordperfect_theme();
void refresh_list(const char *query);
void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void new_entry();
void save_entry();
void delete_entry();
void on_search_changed(GtkEditable *editable, gpointer user_data);
gboolean on_text_view_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void show_context_menu(GdkEventButton *event);
void on_copy_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_cut_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_paste_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

// Apply WordPerfect 5.1 color scheme
void apply_wordperfect_theme() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "window { background-color: #003366; color: #FFFFFF; }\n"
        "frame { background-color: #003366; color: #FFFFFF; }\n"
        "box { background-color: #003366; color: #FFFFFF; }\n"
        "label { background-color: #003366; color: #FFFFFF; }\n"
        "entry { background-color: #003366; color: #FFFFFF; border-color: #005f87; }\n"
        "entry:selected { background-color: #FFFF00; color: #000000; }\n"
        "button { background-color: #005f87; color: #FFFFFF; border-color: #005f87; }\n"
        "button:hover { background-color: #006699; }\n"
        "textview { background-color: #003366; color: #FFFFFF; }\n"
        "textview text { background-color: #003366; color: #FFFFFF; }\n"
        "textview:selected { background-color: #FFFF00; color: #000000; }\n"
        "treeview { background-color: #003366; color: #FFFFFF; }\n"
        "treeview:selected { background-color: #FFFF00; color: #000000; }\n"
        "scrolledwindow { background-color: #003366; }\n"
        "paned { background-color: #003366; }\n";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

// Database initialization
void init_db() {
    char *err_msg = 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS entries("
                      "id INTEGER PRIMARY KEY,"
                      "title TEXT,"
                      "content TEXT,"
                      "tags TEXT,"
                      "created TIMESTAMP"
                      ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// Create the user interface
void create_ui() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "vibe — journaling");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    apply_wordperfect_theme();

    // Main horizontal paned window
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(window), paned);

    // Left panel
    GtkWidget *left_frame = gtk_frame_new(NULL);
    gtk_paned_pack1(GTK_PANED(paned), left_frame, TRUE, FALSE);

    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(left_frame), left_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(left_vbox), 10);

    // Search
    GtkWidget *search_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(left_vbox), search_hbox, FALSE, FALSE, 0);

    search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(search_hbox), search_entry, TRUE, TRUE, 0);
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), NULL);

    // List
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(left_vbox), scrolled_window, TRUE, TRUE, 0);

    list_store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));
    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(tree_view));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Entries", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(tree_view, column);

    g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_row_activated), NULL);

    // Buttons
    GtkWidget *button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(left_vbox), button_hbox, FALSE, FALSE, 0);

    GtkWidget *new_button = gtk_button_new_with_label("New");
    gtk_box_pack_start(GTK_BOX(button_hbox), new_button, TRUE, TRUE, 0);
    g_signal_connect(new_button, "clicked", G_CALLBACK(new_entry), NULL);

    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    gtk_box_pack_start(GTK_BOX(button_hbox), delete_button, TRUE, TRUE, 0);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(delete_entry), NULL);

    // Right panel
    GtkWidget *right_frame = gtk_frame_new(NULL);
    gtk_paned_pack2(GTK_PANED(paned), right_frame, TRUE, FALSE);

    GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(right_frame), right_vbox);
    gtk_container_set_border_width(GTK_CONTAINER(right_vbox), 10);

    // Title
    GtkWidget *title_label = gtk_label_new("Title");
    gtk_misc_set_alignment(GTK_MISC(title_label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(right_vbox), title_label, FALSE, FALSE, 0);

    title_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(right_vbox), title_entry, FALSE, FALSE, 0);

    // Tags
    GtkWidget *tags_label = gtk_label_new("Tags (comma-separated)");
    gtk_misc_set_alignment(GTK_MISC(tags_label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(right_vbox), tags_label, FALSE, FALSE, 0);

    tags_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(right_vbox), tags_entry, FALSE, FALSE, 0);

    // Text view
    GtkWidget *text_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(text_scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(right_vbox), text_scrolled, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_container_add(GTK_CONTAINER(text_scrolled), text_view);

    // Connect keyboard shortcuts for copy/paste
    g_signal_connect(text_view, "key-press-event", G_CALLBACK(on_key_press), NULL);

    // Connect right-click context menu
    g_signal_connect(text_view, "button-press-event", G_CALLBACK(on_text_view_button_press), NULL);

    // Save button
    GtkWidget *save_button = gtk_button_new_with_label("Save");
    gtk_box_pack_start(GTK_BOX(right_vbox), save_button, FALSE, FALSE, 0);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_entry), NULL);

    // Initialize with new entry
    new_entry();
    refresh_list(NULL);
}

// Refresh the list of entries
void refresh_list(const char *query) {
    gtk_list_store_clear(list_store);

    sqlite3_stmt *stmt;
    const char *sql;

    if (query && strlen(query) > 0) {
        sql = "SELECT id, title, created, tags FROM entries WHERE title LIKE ? OR content LIKE ? OR tags LIKE ? ORDER BY created DESC";
    } else {
        sql = "SELECT id, title, created, tags FROM entries ORDER BY created DESC";
    }

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (query && strlen(query) > 0) {
        char like_query[256];
        snprintf(like_query, sizeof(like_query), "%%%s%%", query);
        sqlite3_bind_text(stmt, 1, like_query, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, like_query, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, like_query, -1, SQLITE_STATIC);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        const char *created = (const char *)sqlite3_column_text(stmt, 2);
        const char *tags = (const char *)sqlite3_column_text(stmt, 3);

        char display[512];
        char title_str[256] = "(untitled)";
        if (title && strlen(title) > 0) {
            strncpy(title_str, title, sizeof(title_str) - 1);
        }

        char created_str[20];
        if (created) {
            // Extract date part only
            char *dot_pos = strchr(created, '.');
            if (dot_pos) {
                size_t len = dot_pos - created;
                if (len < sizeof(created_str)) {
                    strncpy(created_str, created, len);
                    created_str[len] = '\0';
                } else {
                    strcpy(created_str, created);
                }
            } else {
                strcpy(created_str, created);
            }
        } else {
            strcpy(created_str, "unknown");
        }

        if (tags && strlen(tags) > 0) {
            snprintf(display, sizeof(display), "%s — %s [%s]", created_str, title_str, tags);
        } else {
            snprintf(display, sizeof(display), "%s — %s", created_str, title_str);
        }

        GtkTreeIter iter;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, id, 1, display, 2, title, 3, tags, -1);
    }

    sqlite3_finalize(stmt);
}

// Handle row selection
void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store), &iter, path)) {
        GValue value = G_VALUE_INIT;
        gtk_tree_model_get_value(GTK_TREE_MODEL(list_store), &iter, 0, &value);
        int id = g_value_get_int(&value);
        g_value_unset(&value);

        sqlite3_stmt *stmt;
        const char *sql = "SELECT title, content, tags FROM entries WHERE id = ?";
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            return;
        }

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *title = (const char *)sqlite3_column_text(stmt, 0);
            const char *content = (const char *)sqlite3_column_text(stmt, 1);
            const char *tags = (const char *)sqlite3_column_text(stmt, 2);

            gtk_entry_set_text(GTK_ENTRY(title_entry), title ? title : "");
            gtk_entry_set_text(GTK_ENTRY(tags_entry), tags ? tags : "");

            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buffer, content ? content : "", -1);
        }

        sqlite3_finalize(stmt);
    }
}

// Create new entry
void new_entry() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    gtk_entry_set_text(GTK_ENTRY(title_entry), timestamp);
    gtk_entry_set_text(GTK_ENTRY(tags_entry), "");

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
}

// Save entry
void save_entry() {
    const char *title = gtk_entry_get_text(GTK_ENTRY(title_entry));
    const char *tags = gtk_entry_get_text(GTK_ENTRY(tags_entry));

    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    if (!title || strlen(title) == 0) {
        title = timestamp;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO entries(title, content, tags, created) VALUES(?, ?, ?, ?)";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        g_free(content);
        return;
    }

    sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, content, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, tags, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, timestamp, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert entry: %s\n", sqlite3_errmsg(db));
    } else {
        refresh_list(gtk_entry_get_text(GTK_ENTRY(search_entry)));
    }

    sqlite3_finalize(stmt);
    g_free(content);
}

// Delete entry
void delete_entry() {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GValue value = G_VALUE_INIT;
        gtk_tree_model_get_value(model, &iter, 0, &value);
        int id = g_value_get_int(&value);
        g_value_unset(&value);

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_QUESTION,
                                                   GTK_BUTTONS_YES_NO,
                                                   "Delete selected entry?");
        int result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (result == GTK_RESPONSE_YES) {
            sqlite3_stmt *stmt;
            const char *sql = "DELETE FROM entries WHERE id = ?";
            int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                return;
            }

            sqlite3_bind_int(stmt, 1, id);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "Failed to delete entry: %s\n", sqlite3_errmsg(db));
            } else {
                new_entry();
                refresh_list(gtk_entry_get_text(GTK_ENTRY(search_entry)));
            }

            sqlite3_finalize(stmt);
        }
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_OK,
                                                   "No entry selected.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

// Handle search
void on_search_changed(GtkEditable *editable, gpointer user_data) {
    const char *query = gtk_entry_get_text(GTK_ENTRY(search_entry));
    refresh_list(query);
}

// Handle keyboard shortcuts for copy/paste
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_c) {
        // Ctrl+C - Copy
        GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard(text_buffer, clipboard);
        return TRUE;
    } else if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_x) {
        // Ctrl+X - Cut
        GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_cut_clipboard(text_buffer, clipboard, TRUE);
        return TRUE;
    } else if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_v) {
        // Ctrl+V - Paste
        GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_paste_clipboard(text_buffer, clipboard, NULL, TRUE);
        return TRUE;
    }
    return FALSE;
}

// Handle right-click for context menu
gboolean on_text_view_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) { // Right-click
        show_context_menu(event);
        return TRUE; // Stop event propagation
    }
    return FALSE; // Continue with default handling
}

// Show custom context menu
void show_context_menu(GdkEventButton *event) {
    GtkWidget *menu = gtk_menu_new();
    
    GtkWidget *copy_item = gtk_menu_item_new_with_label("Copy");
    GtkWidget *cut_item = gtk_menu_item_new_with_label("Cut");
    GtkWidget *paste_item = gtk_menu_item_new_with_label("Paste");
    
    g_signal_connect(copy_item, "activate", G_CALLBACK(on_copy_activate), NULL);
    g_signal_connect(cut_item, "activate", G_CALLBACK(on_cut_activate), NULL);
    g_signal_connect(paste_item, "activate", G_CALLBACK(on_paste_activate), NULL);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), copy_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), cut_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), paste_item);
    
    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
}

// Copy callback
void on_copy_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(text_buffer, clipboard);
}

// Cut callback
void on_cut_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(text_buffer, clipboard, TRUE);
}

// Paste callback
void on_paste_activate(GtkMenuItem *menuitem, gpointer user_data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(text_buffer, clipboard, NULL, TRUE);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Determine database path relative to executable
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        char *exe_dir = dirname(exe_path);
        db_path = malloc(strlen(exe_dir) + strlen("/vibe_journal.db") + 1);
        sprintf(db_path, "%s/vibe_journal.db", exe_dir);
    } else {
        // Fallback to current directory
        db_path = strdup("vibe_journal.db");
    }

    // Open database
    int rc = sqlite3_open(db_path, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        free(db_path);
        return 1;
    }

    init_db();
    create_ui();

    gtk_widget_show_all(window);
    gtk_main();

    sqlite3_close(db);
    free(db_path);
    return 0;
}