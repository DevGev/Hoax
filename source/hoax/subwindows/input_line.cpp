#include "../ui.hpp"
#include "../styler.hpp"
#include "../../tools/tools.h"

static const char* cb_data_entry_string;

void UI::create_window_store_address()
{
    window_address_store = new Fl_Window(300, 50);
    window_address_store->label("enter name for address");

    Fl_Input* stored_address_name_input = new Fl_Input(70, 10, 220, 30, "name: ");
    style_input(stored_address_name_input);

    stored_address_name_input->user_data(this);
    stored_address_name_input->callback(
        [](Fl_Widget* w, void* ui) -> void {
            ((UI*)ui)->close_store_address(w);
            ((Fl_Input*)w)->static_value("");
        }
    );

    window_address_store->hide();
    window_address_store->end();
}

void UI::open_store_address(Fl_Widget* widget)
{
    cb_data_entry_string = widget->label();
    window_address_store->show();
}

void UI::update_address_entry_string(void* entry_ptr)
{
    stored_address_entry* entry = (stored_address_entry*)entry_ptr;
    if (entry->type_switch == VALUE_TYPE_INT) {
        sscanf(cb_data_entry_string, "  %lx		%x		%ld",
            &(entry->address), &(entry->offset), &(entry->value));
        snprintf(entry->entry_string, 255, "  %12lx:%5x%11ld%10s%14s", entry->address, entry->offset, entry->value, scan_type_choice->value(), entry->name);
    }
    if (entry->type_switch == VALUE_TYPE_FLOAT) {
        sscanf(cb_data_entry_string, "  %lx		%x		%lf",
            &(entry->address), &(entry->offset), &entry->value_float);
        snprintf(entry->entry_string, 255, "  %12lx:%5x%11lf%10s%14s", entry->address, entry->offset, (double)entry->value_float, scan_type_choice->value(), entry->name);
    }
    if (entry->type_switch == VALUE_TYPE_STRING) {
        char* value_as_string = (char*)malloc(50);
        sscanf(cb_data_entry_string, "  %lx		%x		%s",
            &(entry->address), &(entry->offset), value_as_string);
        entry->value = (uint64_t)value_as_string;
        snprintf(entry->entry_string, 255, "  %12lx:%5x%11s%10s%14s", entry->address, entry->offset, (const char*)entry->value, scan_type_choice->value(), entry->name);
    }

    stored_address_scroller->redraw();
}

void UI::close_store_address(Fl_Widget* widget)
{
    window_address_store->hide();
    const char* entry_string = ((Fl_Input*)widget)->value();
    int stored_address_index = 0;
    for (; stored_address_index < sizeof(stored_addresses) / sizeof(stored_addresses[0]);) {
        if (stored_addresses[stored_address_index].address == 0 &&
            stored_addresses[stored_address_index].entry_string[0] == 0)
           break;
        stored_address_index++;
    }

    stored_address_entry* entry = &stored_addresses[stored_address_index];
    strcpy(entry->name, entry_string);
    strcpy(entry->type, scan_type_choice->value());

    if (entry->type[0] == 'f' && entry->type[1] == 'l' && entry->type[2] == 'o')
        entry->type_switch = VALUE_TYPE_FLOAT;
    else if (entry->type[0] == 'i' && entry->type[1] == 'n' && entry->type[2] == 't')
        entry->type_switch = VALUE_TYPE_INT;
    else if (strcmp(entry->type, "any int") == 0)
        entry->type_switch = VALUE_TYPE_INT;
    else if (strcmp(entry->type, "any float") == 0)
        entry->type_switch = VALUE_TYPE_FLOAT;
    else if (strcmp(entry->type, "string") == 0)
        entry->type_switch = VALUE_TYPE_STRING;
    else if (strcmp(entry->type, "bytearray") == 0)
        entry->type_switch = VALUE_TYPE_STRING;
    update_address_entry_string(entry);
}
