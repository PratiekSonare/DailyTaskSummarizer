#include "NoteEditor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <curl/curl.h>
#include <json/json.h>

NoteEditor::NoteEditor() : Gtk::Box(Gtk::ORIENTATION_VERTICAL) {
    set_spacing(10);
    set_margin_top(10);
    set_margin_bottom(10);
    set_margin_start(10);   
    set_margin_end(10);

    // Add task button
    add_task_button.set_label("➕ Add Task");
    add_task_button.get_style_context()->add_class("custom-button");
    add_task_button.signal_clicked().connect([this]() {
        add_task_row("", "", "");
    });
    pack_start(add_task_button, Gtk::PACK_SHRINK);

    // Task list inside scroll
    task_list_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    task_scroll.add(task_list_box);
    task_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    task_scroll.set_min_content_height(200);

    pack_start(task_scroll);

    auto* summaryGrid = Gtk::make_managed<Gtk::Grid>();
    summaryGrid->set_column_spacing(10);
    summaryGrid->set_margin_top(5);
    summaryGrid->set_margin_bottom(5);

    // Helper function
    auto create_summary_box = [](const Glib::ustring& label_text, Gtk::Label& value_label) {
        auto* vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 2);
        auto* label = Gtk::make_managed<Gtk::Label>(label_text);
        label->set_halign(Gtk::ALIGN_START);
        value_label.set_text("0");
        value_label.set_halign(Gtk::ALIGN_START);
        value_label.get_style_context()->add_class("valuebox");
        vbox->pack_start(*label, Gtk::PACK_SHRINK);
        vbox->pack_start(value_label, Gtk::PACK_SHRINK);
        return vbox;
    };

    // Create boxes
    auto* totalTasksBox    = create_summary_box("🧮 Total Tasks", total_tasks_value);
    auto* totalHoursBox    = create_summary_box("⏱ Total Hours", total_hours_value);
    auto* boredomIndexBox  = create_summary_box("📉 Boredom Index (<10)", boredom_index_value);

    totalTasksBox->get_style_context()->add_class("totalTasksBox");
    totalHoursBox->get_style_context()->add_class("totalTasksBox");
    boredomIndexBox->get_style_context()->add_class("totalTasksBox");

    // Attach them to the grid
    summaryGrid->attach(*totalTasksBox,    0, 0, 1, 1);
    summaryGrid->attach(*totalHoursBox,    1, 0, 1, 1);
    summaryGrid->attach(*boredomIndexBox,  2, 0, 1, 1);

    // Add it to main layout
    pack_start(*summaryGrid, Gtk::PACK_SHRINK);

    //how can i create these components?
    //total_tasks = Gtk::Box - type of = flex flex-col with <p>Total Tasks</p> <p>{total number of tasks in tasks_lists}</p>
    //total_hours = Gtk::Box - type of = flex flex-col with <p>Total Hourse</p> <p>{sum of duration_input}</p>
    //boredom_index = Gtk::Box - type of = flex flex-col with <p>Boredom Index</p> <p>{avg. of boredom_input}</p>

    auto* finalRow = Gtk::make_managed<Gtk::Grid>();
    finalRow->set_column_spacing(5);

    // Set buttons
    submit_button.set_label("SUBMIT");
    submit_button.get_style_context()->add_class("submit-button");
    submit_button.set_hexpand(true);
    submit_button.signal_clicked().connect([this]() {

        Json::Value tasks_json(Json::arrayValue); // use JsonCpp or nlohmann/json

        for (auto* row : task_list_box.get_children()) {
            auto* vbox = dynamic_cast<Gtk::Box*>(row);
            if (!vbox) continue;

            auto children = vbox->get_children();
            if (children.empty()) continue;

            auto* grid = dynamic_cast<Gtk::Grid*>(children[0]);
            if (!grid) continue;

            auto grid_children = grid->get_children();

            // Index: [0]=label, [1]=desc_entry, [2]=duration_input, [3]=boredom_score
            auto* desc_entry  = dynamic_cast<Gtk::Entry*>(grid->get_child_at(1, 0));
            auto* duration_input  = dynamic_cast<Gtk::Entry*>(grid->get_child_at(4, 0));
            auto* boredom_score   = dynamic_cast<Gtk::Entry*>(grid->get_child_at(5, 0));

            if (desc_entry && duration_input && boredom_score) {
                Json::Value task;
                task["task_description"]     = desc_entry->get_text().raw();
                task["duration"]             = duration_input->get_text().raw();
                task["boredom_score"]        = boredom_score->get_text().raw();

                tasks_json.append(task);
            }
        }

        if (!tasks_json.empty()) {
            std::cerr << "Here is the input tasks: " << tasks_json.toStyledString() << std::endl;
            send_to_openrouter(tasks_json);
        } else {
            std::cerr << "❌ No tasks to submit." << std::endl;
        }
    });

    delete_all_button.set_label("DELETE ALL");
    delete_all_button.get_style_context()->add_class("del-button");
    delete_all_button.set_hexpand(true);
    delete_all_button.signal_clicked().connect([this]() {
        // Remove all children from task_list_box
        for (auto* child : task_list_box.get_children()) {
            task_list_box.remove(*child);
        }

        task_count = 0;
        add_task_button.set_sensitive(true);
    });

    // Attach buttons to grid with widths 3:1
    finalRow->attach(submit_button,      0, 0, 3, 1);  // left, top, width (3/4)
    finalRow->attach(delete_all_button,  3, 0, 1, 1);  // right-most 1/4

    pack_start(*finalRow, Gtk::PACK_SHRINK);


    show_all_children();
}

void NoteEditor::add_task_row(const std::string& desc, const std::string& entry, const std::string& exit) {

    if (task_count >= 8) {
        return;
    }
    task_count++; // Increment task counter

    update_summary();

    auto* row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 2);
    auto* task_grid = Gtk::make_managed<Gtk::Grid>();
    task_grid->set_column_spacing(5);
    task_grid->set_row_spacing(5);

    // Row number label
    auto* number_label = Gtk::make_managed<Gtk::Label>(std::to_string(task_count) + ")");
    number_label->set_halign(Gtk::ALIGN_START);
    number_label->set_valign(Gtk::ALIGN_CENTER);
    number_label->get_style_context()->add_class("row-number");

    // Entry widgets
    auto* task_entry = Gtk::make_managed<Gtk::Entry>();
    task_entry->set_placeholder_text("Task description");
    task_entry->set_text(desc);

    auto* duration_input = Gtk::make_managed<Gtk::Entry>();
    duration_input->set_placeholder_text("Duration");
    duration_input->set_text(entry);
    duration_input->get_style_context()->add_class("time-label");

    auto* boredom_score = Gtk::make_managed<Gtk::Entry>();
    boredom_score->set_placeholder_text("Boredom Score (<10)");
    boredom_score->set_text(exit);
    boredom_score->get_style_context()->add_class("time-label");

    // Delete button with icon
    auto pixbuf = Gdk::Pixbuf::create_from_file("/home/pratiek/Downloads/Bakchod_Projects/bakchod-notetaking/trash-solid.svg", 16, 16, true);
    auto* icon = Gtk::make_managed<Gtk::Image>(pixbuf);
    icon->set_size_request(16, 16);

    auto* delete_task_button = Gtk::make_managed<Gtk::Button>();
    delete_task_button->set_image(*icon);
    delete_task_button->set_tooltip_text("Delete task");
    delete_task_button->set_relief(Gtk::RELIEF_NONE);
    delete_task_button->set_size_request(24, 24);
    delete_task_button->set_hexpand(false);
    delete_task_button->set_vexpand(false);
    delete_task_button->set_focus_on_click(false);

    // Delete logic
    delete_task_button->signal_clicked().connect([this, row]() {
        task_list_box.remove(*row);
        // Optional: decrement count or re-render numbers
    });

    // Attach widgets to grid (shift everything right by 1 column)
    task_grid->attach(*number_label,        0, 0, 1, 1);
    task_grid->attach(*task_entry,          1, 0, 3, 1);
    task_grid->attach(*duration_input,         4, 0, 1, 1);
    task_grid->attach(*boredom_score,          5, 0, 1, 1);
    task_grid->attach(*delete_task_button,  6, 0, 1, 1);

    number_label->get_style_context()->add_class("def-mar");
    task_entry->get_style_context()->add_class("def-mar");
    duration_input->get_style_context()->add_class("def-mar");
    boredom_score->get_style_context()->add_class("def-mar");

    // Optional styling
    task_entry->set_width_chars(23);
    duration_input->set_width_chars(8);
    boredom_score->set_width_chars(8);

    row->pack_start(*task_grid, Gtk::PACK_EXPAND_WIDGET);
    task_list_box.pack_start(*row, Gtk::PACK_SHRINK);
    task_list_box.show_all_children();
}

void NoteEditor::load(const std::string& title) {
    current_title = title;
    title_entry.set_text(title);

    std::ifstream file(std::string(std::getenv("HOME")) + "/.noteontop_" + title + ".txt");
    std::stringstream buffer;
    if (file.is_open()) buffer << file.rdbuf();
    // You could parse saved task data here
}

void NoteEditor::save() {
    std::ofstream file(std::string(std::getenv("HOME")) + "/.noteontop_" + current_title + ".txt");
    // You could serialize task data here
}

std::string NoteEditor::get_current_title() const {
    return current_title;
}

std::string read_api_key(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string key;
    if (file.is_open()) {
        std::getline(file, key);
        file.close();
    }
    return key;
}

void NoteEditor::send_to_openrouter(const Json::Value& tasks_json) {
    CURL* curl = curl_easy_init();
    if (!curl) return;

    // ✅ Prepare JSON payload as per OpenRouter API syntax
    Json::Value req_json;
    req_json["model"] = "deepseek/deepseek-chat-v3-0324:free";
    req_json["messages"] = Json::arrayValue;

    std::string prompt = 
        "You are an expert time-table scheduler, experienced in providing daily task optimisation techniques such as applying the Pomodoro method. "
        "I am a person who finds doing routine tasks (such as studying daily at the same time) pretty boring and mundane. "
        "Using my daily tasks mentioned here, generate a sample optimized day schedule incorporating optimization techniques (such as the Pomodoro technique). "
        "For each task, duration and boredom score has been defined by me. If the boredom score for a task is greater, apply Pomodoro technique more aggressively for that task (meaning, add more short breaks for me to not get bored while doing it). "
        "Duration is the total number of hours that I intend on spending on a given task. "
        "Consider my active time of the day to be between 10am and 10pm. Do not allot tasks between 1pm–2pm and 5pm–8pm. "
        "Strictly return only a valid JSON object with no explanation or markdown. The object should have a `schedule` key containing an array of tasks, and a `remarks` key containing 5 suggestions. "
        "Each schedule item must have these keys: `time_slot`, `task`, `duration`, `technique_applied`, and `notes`.\n\n"

        "Example format:\n"
        "{\n"
        "  \"schedule\": [\n"
        "    {\n"
        "      \"time_slot\": \"10:00–10:50\",\n"
        "      \"task\": \"Practice DSA\",\n"
        "      \"duration\": \"50 min\",\n"
        "      \"technique_applied\": \"Pomodoro (25/5 x 2)\",\n"
        "      \"notes\": \"High boredom = more breaks\"\n"
        "    },\n"  
        "    {\n"
        "      \"time_slot\": \"11:00–11:50\",\n"
        "      \"task\": \"Freelancing\",\n"
        "      \"duration\": \"50 min\",\n"
        "      \"technique_applied\": \"Standard Work\",\n"
        "      \"notes\": \"Low boredom\"\n"
        "    }\n"
        "  ],\n"
        "  \"remarks\": [\n"
        "    \"Use breaks to hydrate and relax.\",\n"
        "    \"Switch tasks after intense sessions.\",\n"
        "    \"Prioritize high-boredom tasks early.\",\n"
        "    \"Avoid task overlaps.\",\n"
        "    \"Be consistent with the routine.\"\n"
        "  ]\n"
        "}\n\n";

    prompt += tasks_json.toStyledString();


    Json::Value message;
    message["role"] = "user";
    message["content"] = prompt;

    req_json["messages"].append(message);

    std::string json_str = req_json.toStyledString();

    // ✅ Setup headers
    struct curl_slist* headers = nullptr;

    std::string api_key = read_api_key("/home/pratiek/Downloads/Bakchod_Projects/bakchod-notetaking/apikey.txt");
    std::string header = "Authorization: Bearer " + api_key;
    headers = curl_slist_append(headers, header.c_str());

    // ✅ Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, "https://openrouter.ai/api/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());

    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
        +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
            std::string* str = static_cast<std::string*>(userdata);
            str->append(ptr, size * nmemb);
            return size * nmemb;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // ✅ Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        std::cout << "LLM Response:\n" << response_data << std::endl;

        // ✅ Parse and show response content in GTK dialog
        Json::Value response_json;
        Json::CharReaderBuilder reader;
        std::istringstream stream(response_data);
        std::string errs;
        if (Json::parseFromStream(reader, stream, &response_json, &errs)) {
            auto content = response_json["choices"][0]["message"]["content"].asString();
            
            // Instead of dialog, call the main callback
            if (schedule_callback) {
                schedule_callback(content);
            }

        } else {
            std::cerr << "Failed to parse JSON: " << errs << std::endl;
        }
    } else {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    // ✅ Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void NoteEditor::update_summary() {
    int total_tasks = 0;
    double total_hours = 0;
    double total_boredom = 0.0;

    for (auto* row : task_list_box.get_children()) {
        auto* vbox = dynamic_cast<Gtk::Box*>(row);
        if (!vbox) continue;

        auto* grid = dynamic_cast<Gtk::Grid*>(vbox->get_children()[0]);
        if (!grid) continue;

        auto* duration_input  = dynamic_cast<Gtk::Entry*>(grid->get_child_at(4, 0));
        auto* boredom_score   = dynamic_cast<Gtk::Entry*>(grid->get_child_at(5, 0));

        if (duration_input && boredom_score) {
            try {
                double duration = std::stod(duration_input->get_text());
                double boredom = std::stod(boredom_score->get_text());

                total_hours += duration;
                total_boredom += boredom;
                total_tasks++;
            } catch (...) {
                // Ignore invalid entries
            }
        }
    }

    total_tasks_value.set_text(std::to_string(total_tasks));
    total_hours_value.set_text(std::to_string(total_hours));
    if (total_tasks > 0)
        boredom_index_value.set_text(std::to_string(total_boredom / total_tasks).substr(0, 4));
    else
        boredom_index_value.set_text("0");
}
