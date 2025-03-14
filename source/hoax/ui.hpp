#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Help_View.H>
#include <stdint.h>

#define MAX_ADDRESS_ITEMS 500
#define VALUE_TYPE_INT 0
#define VALUE_TYPE_FLOAT 1
#define VALUE_TYPE_STRING 2

class UI {
public:
    UI();
    ~UI();

    int show(int argc, char** argv);
    void update_all_address_values_task();

private:
    static UI* active;
    Fl_Window* window_master;
    Fl_Window* window_memory_viewer;
    Fl_Window* window_address_store;
    Fl_Window* window_process_picker;
    Fl_Window* window_address_modifier;
    Fl_Window* window_address_actions;

    Fl_Button* button_open_process_picker;
    Fl_Button* button_open_memory_viewer;
    Fl_Scroll* address_scroller;
    Fl_Scroll* stored_address_scroller;
    Fl_Input* scan_input;
    Fl_Input_Choice* scan_type_choice;
    Fl_Box* scan_status_label;
    Fl_Help_View* instruction_view;
    char* address_items_labels[MAX_ADDRESS_ITEMS];

    void open_notepad();
    void close_notepad();
    void update_process_picker();
    void open_process_picker();
    void close_process_picker(Fl_Widget* widget);
    void open_store_address(Fl_Widget* widget);
    void close_store_address(Fl_Widget* widget);
    void open_address_modifier(Fl_Widget* widget, int locs=-1);
    void close_address_modifier(Fl_Widget* widget);
    void close_memory_viewer(Fl_Widget* widget);
    void open_address_actions(Fl_Widget* widget);
    void helper_create_window_memory_viewer();
    void create_window_memory_viewer();
    void create_window_address_actions();
    void create_window_notepad();

    void open_memory_viewer(uint32_t location = 0);
    void open_memory_debugger(const char* command, uint32_t location);

    void spawn_disasmd();
    void send_disasm(const char* command);
    void update_disasm_output();

    void button_scan_pressed();
    void scan_command(const char* command, bool is_search=false, bool update_status=true);
    int update_scan_list(const char* data);
    void delete_stored_address(uint32_t position);
    void pop_stored_address();
    void push_stored_address();

    void update_all_address_values();
    void update_address_value(size_t address, int data_type, void* update_at);
    void update_address_entry_string(void* entry);

    void kill_disasmd_server();
    void kill_scanmemd_server();

    void create_window_process_picker();
    void create_window_store_address();
    void create_window_address_modifier();

    bool changed_pid = false;
    pid_t selected_pid = 0;
    int scan_data_type = 0;
    bool disasmd_requires_check = false;
    bool ignore_update_scan_list = true;

    typedef struct {
        size_t address;
        uint32_t offset;
        size_t first_value;
        size_t previous_value;
        int type_switch;

        uint64_t value;
        double value_float;

        char type[255];
        char name[255];
        char entry_string[255];
    } stored_address_entry;

    stored_address_entry stored_addresses[100];
};
