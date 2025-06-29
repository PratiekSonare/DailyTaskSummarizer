#pragma once
#include <gtkmm.h>
#include "NoteEditor.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();

    void show_schedule(const Glib::ustring& schedule_text);
    void go_back();

private:
    Gtk::Stack main_stack;
    NoteEditor note_editor;

    void show_note_editor(const std::string& title);
    void reload_css();

    Gtk::Grid* build_schedule_grid(const Json::Value& schedule_array);

    // LLM response view
    Gtk::Box schedule_box;
    Gtk::ScrolledWindow schedule_scroll;
    Gtk::TextView schedule_text_view;

    Gtk::Button back_button;
    // std::string convert_markdown_to_html(const std::string& markdown_text);
    // Glib::RefPtr<Gtk::Widget> web_view_widget; // Gtkmm wrapper for WebKit
    // GtkWidget* web_view_raw = nullptr; // C-style WebKit view (raw)

};
