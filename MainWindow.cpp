#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow() : note_editor() {

    set_title("TaskSummarizer");

    // ✅ Fixed size (e.g., 400x400 square)
    set_default_size(450, 450);
    set_resizable(false);
    set_keep_above(true);
    set_decorated(true);
    set_type_hint(Gdk::WINDOW_TYPE_HINT_UTILITY);

    main_stack.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    add(main_stack);

    // LLM schedule view
    schedule_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    schedule_box.get_style_context()->add_class("llm-response");
    schedule_text_view.set_wrap_mode(Gtk::WRAP_WORD);
    schedule_scroll.add(schedule_text_view);
    schedule_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    back_button.set_label("⬅ Back");
    back_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::go_back));

    schedule_box.pack_start(schedule_scroll, Gtk::PACK_EXPAND_WIDGET);
    schedule_box.pack_start(back_button, Gtk::PACK_SHRINK);

    // Add widgets to stack
    main_stack.add(note_editor);
    main_stack.add(schedule_box);

    // Set up callback so NoteEditor can call us when needed
    note_editor.set_schedule_callback([this](const Glib::ustring& schedule) {
        show_schedule(schedule);
    });

    auto css_provider = Gtk::CssProvider::create();
    try {   
        css_provider->load_from_path("/home/pratiek/Downloads/Bakchod_Projects/bakchod-notetaking/style.css");
    } catch (const Gtk::CssProviderError& e) {
        std::cerr << "❌ CSS load failed: " << e.what() << std::endl;
    }

    Gtk::StyleContext::add_provider_for_screen(
        Gdk::Screen::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_USER 
    );

    get_style_context()->add_class("main-window");

    add_events(Gdk::KEY_PRESS_MASK);
    signal_key_press_event().connect([this](GdkEventKey* event) {
        if (event->keyval == GDK_KEY_F5) {
            reload_css();
            return true;
        }
        return false;
    });


    show_all_children();
}

void MainWindow::show_schedule(const Glib::ustring& raw_schedule_text) {
    std::string cleaned_text = raw_schedule_text;

    // 🧹 Strip ```json and ``` if present
    const std::string start_tag = "```json";
    const std::string end_tag = "```";

    auto start_pos = cleaned_text.find(start_tag);
    if (start_pos != std::string::npos)
        cleaned_text = cleaned_text.substr(start_pos + start_tag.length());

    auto end_pos = cleaned_text.rfind(end_tag);
    if (end_pos != std::string::npos)
        cleaned_text = cleaned_text.substr(0, end_pos);

    // Trim whitespace
    cleaned_text.erase(0, cleaned_text.find_first_not_of(" \n\r\t"));
    cleaned_text.erase(cleaned_text.find_last_not_of(" \n\r\t") + 1);

    // ✅ Now parse the cleaned JSON
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream ss(cleaned_text);

    if (!Json::parseFromStream(reader, ss, &root, &errs)) {
        std::cerr << "❌ Failed to parse schedule JSON: " << errs << std::endl;
        return;
    }

    // ✅ Continue with rendering...
    Gtk::Grid* grid = build_schedule_grid(root["schedule"]);
    Gtk::Box* wrapper = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    wrapper->set_spacing(10);
    wrapper->pack_start(*grid, Gtk::PACK_EXPAND_WIDGET);

    if (root.isMember("remarks")) {
        auto* label = Gtk::make_managed<Gtk::Label>("<b>📝 Remarks:</b>");
        label->set_use_markup(true);
        wrapper->pack_start(*label, Gtk::PACK_SHRINK);

        for (const auto& remark : root["remarks"]) {
            auto* remark_label = Gtk::make_managed<Gtk::Label>("• " + remark.asString());
            remark_label->set_halign(Gtk::ALIGN_START);
            wrapper->pack_start(*remark_label, Gtk::PACK_SHRINK);
        }
    }

    schedule_scroll.remove();
    schedule_scroll.add(*wrapper);
    main_stack.set_visible_child(schedule_box);
    show_all_children();
}


Gtk::Grid* MainWindow::build_schedule_grid(const Json::Value& schedule_array) {
    auto* grid = Gtk::make_managed<Gtk::Grid>();
    grid->set_column_spacing(10);
    grid->set_row_spacing(5);

    // Header row
    std::vector<std::string> headers = {
        "Time Slot", "Task", "Duration", "Technique", "Notes"
    };
    for (size_t col = 0; col < headers.size(); ++col) {
        auto* label = Gtk::make_managed<Gtk::Label>("<b>" + headers[col] + "</b>");
        label->set_use_markup(true);
        grid->attach(*label, col, 0, 1, 1);
    }

    // Fill in rows
    for (Json::ArrayIndex i = 0; i < schedule_array.size(); ++i) {
        const Json::Value& item = schedule_array[i];
        std::vector<std::string> row_data = {
            item["time_slot"].asString(),
            item["task"].asString(),
            item["duration"].asString(),
            item["technique_applied"].asString(),
            item["notes"].asString()
        };

        for (size_t col = 0; col < row_data.size(); ++col) {
            auto* cell = Gtk::make_managed<Gtk::Label>(row_data[col]);
            cell->set_halign(Gtk::ALIGN_START);
            grid->attach(*cell, col, i + 1, 1, 1);
        }
    }

    return grid;
}


void MainWindow::go_back() {
    main_stack.set_visible_child(note_editor);
}

void MainWindow::show_note_editor(const std::string& title) {
    note_editor.load(title);
    main_stack.set_visible_child(note_editor);
}

void MainWindow::reload_css() {
    auto css_provider = Gtk::CssProvider::create();
    try {
        css_provider->load_from_path("/home/pratiek/Downloads/Bakchod_Projects/bakchod-notetaking/style.css");
        auto screen = Gdk::Screen::get_default();
        Gtk::StyleContext::add_provider_for_screen(
            screen,
            css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );
        std::cout << "CSS reloaded.\n";
    } catch (const Gtk::CssProviderError& e) {
        std::cerr << "Failed to reload CSS: " << e.what() << std::endl;
    }
}