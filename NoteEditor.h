#pragma once
#include <gtkmm.h>
#include <string>
#include <functional>
#include <json/json.h> // Add this near the top

class NoteEditor : public Gtk::Box {
public:
    NoteEditor();

    void set_schedule_callback(std::function<void(const Glib::ustring&)> cb) {
        schedule_callback = cb;
    }
    
    void load(const std::string& title);
    void save();
    void add_task_row(const std::string& desc, const std::string& entry, const std::string& exit);
    std::string get_current_title() const;

    void update_summary();
    
private:

    std::function<void(const Glib::ustring&)> schedule_callback;

    int task_count = 0;
    void send_to_openrouter(const Json::Value& tasks_json);

    Gtk::Entry title_entry;

    // Task list section
    Gtk::Box task_list_box;
    Gtk::ScrolledWindow task_scroll;
    Gtk::Button add_task_button;
    Gtk::Button delete_task_button;

    Gtk::Label total_tasks_value;
    Gtk::Label total_hours_value;
    Gtk::Label boredom_index_value;

    // Submit section
    Gtk::Button submit_button;
    Gtk::Button delete_all_button;

    std::string current_title;
};
