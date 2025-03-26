#include "ui.hpp"
#include "styler.hpp"
#include "../tools/tools.h"
#include <stdio.h> 
#include <time.h> 
#include <fcntl.h>
#include <thread>
#include <unistd.h> 
#include <string.h> 
#include <ctype.h> 

UI* UI::active = 0;

UI::UI()
{
    active = this;
    window_master = new Fl_Window(820, 580);
    window_master->label("master");

    Fl::set_box_color(FL_BLACK);
    button_open_process_picker = new Fl_Button(20, 15, 170, 20, "open process (none)");
    style_button(button_open_process_picker);
    button_open_process_picker->color(fg_accent_color);
    button_open_process_picker->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->ignore_update_scan_list = true;
            UI::active->open_process_picker();
        }
    );

    Fl_Button* button_save_state = new Fl_Button(215, 15, 70, 20, "save");
    style_button(button_save_state);
    button_save_state->color(fg_accent_color);
    button_save_state->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->save_state_as_file();
        }
    );

    Fl_Button* button_load_state = new Fl_Button(295, 15, 70, 20, "load");
    style_button(button_load_state);
    button_load_state->color(fg_accent_color);
    button_load_state->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->load_state_from_file();
        }
    );

    Fl_Button* button_refresh = new Fl_Button(600, 15, 80, 20, "refresh");
    style_button(button_refresh);
    button_refresh->color(fg_accent_color);
    button_refresh->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->update_all_address_values();
        }
    );

    Fl_Button* button_restart_servers = new Fl_Button(420, 15, 170, 20, "restart servers");
    style_button(button_restart_servers);
    button_restart_servers->color(fg_accent_color);
    button_restart_servers->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->kill_disasmd_server();
            UI::active->kill_scanmemd_server();
        }
    );

    Fl_Button* button_notepad = new Fl_Button(690, 15, 78, 20, "notepad");
    style_button(button_notepad);
    button_notepad->color(fg_accent_color);
    button_notepad->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->open_notepad();
        }
    );

    Fl_Button* button_reset_scan = new Fl_Button(420, 50, 110, 20, "reset");
    style_button(button_reset_scan);
    button_reset_scan->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->ignore_update_scan_list = true;
            UI::active->scan_command("reset");
            UI::active->scan_status_label->label("[no scan in progress]");
        }
    );

    Fl_Button* button_undo_scan = new Fl_Button(540, 50, 100, 20, "snapshot");
    style_button(button_undo_scan);
    button_undo_scan->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->ignore_update_scan_list = false;
            UI::active->scan_command("snapshot");
        }
    );

    Fl_Button* button_scan = new Fl_Button(650, 50, 118, 20, "scan");
    style_button(button_scan);
    button_scan->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->ignore_update_scan_list = false;
            UI::active->button_scan_pressed();
        }
    );

    scan_input = new Fl_Input(475, 85, 290, 30, "value: ");
    style_input(scan_input);

    scan_status_label = new Fl_Box(420, 123, 160, 24, "[no scan in progress]");
    style_box(scan_status_label);

    scan_type_choice = new Fl_Input_Choice(635, 120, 150, 30, "type:");
    scan_type_choice->labelcolor(fg_main_color);
    scan_type_choice->labelfont(FL_COURIER);
    scan_type_choice->labelsize(13);
    scan_type_choice->value("any int");
    style_input(scan_type_choice->input());
    scan_type_choice->box(_FL_OFLAT_BOX);
    style_menu_button(scan_type_choice->menubutton());
    scan_type_choice->menubutton()->position(750, 123);

    scan_type_choice->add("any int");
    scan_type_choice->add("any float");
    scan_type_choice->add("int8");
    scan_type_choice->add("int16");
    scan_type_choice->add("int32");
    scan_type_choice->add("int64");
    scan_type_choice->add("float32");
    scan_type_choice->add("float64");
    scan_type_choice->add("bytearray");
    scan_type_choice->add("string");

    button_open_memory_viewer = new Fl_Button(420, 220, 170, 20, "open memory viewer");
    style_button(button_open_memory_viewer);
    button_open_memory_viewer->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->open_memory_viewer();
        }
    );

    Fl_Button* button_new_address = new Fl_Button(600, 220, 45, 20, "+");
    style_button(button_new_address);
    button_new_address->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->push_stored_address();
        }
    );
    
    Fl_Button* button_remove_address = new Fl_Button(655, 220, 45, 20, "-");
    style_button(button_remove_address);
    button_remove_address->callback(
        [](Fl_Widget*, void*) -> void {
            UI::active->pop_stored_address();
        }
    );

    Fl_Button* button_clear_address = new Fl_Button(710, 220, 60, 20, "clear");
    style_button(button_clear_address);
    button_clear_address->callback(
        [](Fl_Widget*, void*) -> void {
            stored_address_entry* stored = UI::active->stored_addresses;
            size_t sz = sizeof(UI::active->stored_addresses);
            size_t sze = sizeof(UI::active->stored_addresses[0]);
            for (int i = 0; i < sz / sze; i++)
                memset(&stored[i], 0, sze);
            UI::active->stored_address_scroller->redraw();
        }
    );

    address_scroller = new Fl_Scroll(20, 50, 350, 510);
    for (int i = 0; i < MAX_ADDRESS_ITEMS + 1; i++) {
        char* entry_string = new char[512];
        *entry_string = 0;

        Fl_Button* address_item = new Fl_Button(20, 15 + i * 20, 345, 18,
            (i == 0) ? "  address\t\toffset\t        value\t" : entry_string);

        if (i != 0) {
            address_item->callback([](Fl_Widget* w, void*) -> void {
                UI::active->open_store_address(w);
            });
            address_items_labels[i - 1] = entry_string;
        }

        style_scroller_item(address_item, i);
    }
    style_scroller(address_scroller);
    address_scroller->end();

    stored_address_scroller = new Fl_Scroll(420, 250, 350, 310);
    for (int i = 0; i < 50; i++) {
        Fl_Button* stored_address_item = new Fl_Button(420, 15 + i * 20, 345, 18,
            (i == 0) ? "  address        off      value      type\t   name" : stored_addresses[i - 1].entry_string);
        style_scroller_item(stored_address_item, i);

        stored_address_item->user_data(new int {i});
        stored_address_item->callback([](Fl_Widget* w, void* i) -> void {
            int mx = Fl::event_x();
            int mb = Fl::event_button();
            if (mb == 1)
                UI::active->open_address_modifier(w); 
            if (mb == 3)
                UI::active->open_address_actions(w);
        });
    }
    style_scroller(stored_address_scroller);
    stored_address_scroller->end();
    stored_address_scroller->scroll_to(0, -240);
    
    create_window_process_picker();
    create_window_store_address();
    create_window_address_modifier();
    create_window_memory_viewer();
    create_window_address_actions();
    create_window_notepad();

    window_master->end();
}

UI::~UI() {}

int UI::show(int argc, char** argv)
{
    window_master->show(argc, argv);

    size_t tspec = 0;
    while (Fl::wait()) {
        if (!button_open_process_picker->visible())
            button_open_process_picker->show();

        if (disasmd_requires_check)
            update_disasm_output();

        size_t ltime = time(NULL) - tspec;
        if (!ltime) continue;

        update_process_picker();
        address_scroller->redraw();
        stored_address_scroller->redraw();
        window_process_picker->redraw();
        button_open_process_picker->clear();

        tspec = time(NULL);
    }
    return 0;
}

void UI::update_all_address_values()
{
    if (!ignore_update_scan_list)
        scan_command("update|list", false, false);

    size_t sz = sizeof(stored_addresses) / sizeof(stored_addresses[0]);
    for (int i = 0; i < sz; i++) {
        if (strlen(stored_addresses[i].name) == 0)
            break;
        stored_address_entry* entry = &stored_addresses[i];
        update_address_value(entry->address, entry->type_switch,
            ((entry->type_switch == VALUE_TYPE_FLOAT) ?
             (void*)&entry->value_float : (void*)&entry->value));
        update_address_entry_string(entry);
    }
}

void UI::pop_stored_address()
{
    int location = -1;
    size_t sz = sizeof(stored_addresses) / sizeof(stored_addresses[0]);
    for (int i = 0; i < sz; i++) {
        if (strlen(stored_addresses[i].name) == 0)
            break;
        location = i;
    }
    if (location == -1) return;
    memset(&stored_addresses[location], 0, sizeof(stored_addresses[location]));
    stored_address_scroller->redraw();
    Fl::flush();
}

void UI::push_stored_address()
{
    int location = -1;
    size_t sz = sizeof(stored_addresses) / sizeof(stored_addresses[0]);
    for (int i = 0; i < sz; i++) {
        if (strlen(stored_addresses[i].name) != 0)
            continue;
        location = i;
        break;
    }
    if (location == -1) return;
    open_address_modifier(0, location);
    stored_address_scroller->redraw();
    Fl::flush();
}

void UI::delete_stored_address(uint32_t position)
{
    size_t sz = sizeof(stored_addresses) / sizeof(stored_addresses[0]);
    for (int i = position; i < sz - 1; i++) {
        stored_address_entry* dest = &stored_addresses[i];
        stored_address_entry* src = &stored_addresses[i + 1];
        dest->address = src->address;
        dest->first_value = src->first_value;
        dest->offset = src->offset;
        dest->previous_value = src->previous_value;
        dest->type_switch = src->type_switch;
        dest->value = src->value;
        dest->value_float = src->value_float;
        strcpy(dest->name, src->name);
        strcpy(dest->type, src->type);
        strcpy(dest->entry_string, src->entry_string);
    }

    stored_address_scroller->redraw();
}

static char zenity_line[1024];
char* zenity_dialog(const char* cmd)
{
    memset(zenity_line, 0, sizeof(zenity_line));
    FILE* fp = popen(cmd, "r");

    if (fp == NULL) {
        perror("Pipe returned a error");
    } else {
        while (fgets(zenity_line, sizeof(zenity_line), fp))
            if (zenity_line[0] != '/') continue;
        if (WEXITSTATUS(pclose(fp)) != 0)
            return NULL;
    }

    for (int i = 0; i < sizeof(zenity_line); i++) {
        if (zenity_line[i] == '\n') zenity_line[i] = 0;
        if (zenity_line[i] == '\r') zenity_line[i] = 0;
        if (zenity_line[i] == '\b') zenity_line[i] = 0;
    }
    return zenity_line;
}

void UI::load_state_from_file()
{
    char* file_name = zenity_dialog("zenity --file-selection --title=]\"Load state\"");
    if (!file_name) return;

    save_state_t state;
    memcpy(&state.entries, &stored_addresses, sizeof(stored_addresses));

    int fd = open(file_name, O_RDONLY, 0666);
    read(fd, &state, sizeof(state));
    close(fd);

    if (strncmp(state.magic, "hoax", 4) != 0) return;
    memcpy(&stored_addresses, &state.entries, sizeof(stored_addresses));
    notepad->value(state.notepad);
}

void UI::save_state_as_file()
{
    char* file_name = zenity_dialog("zenity --file-selection --save --title=]\"Save state\"");
    if (!file_name) return;

    save_state_t state;
    strncpy(state.magic, "hoax", 4);
    memcpy(&state.entries, &stored_addresses, sizeof(stored_addresses));
    strncpy(state.notepad, notepad->value(), sizeof(state.notepad));

    int fd = open(file_name, O_RDWR | O_CREAT, 0666);
    write(fd, &state, sizeof(state));
    close(fd);
}
