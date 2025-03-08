#include "../ui.hpp"
#include "../styler.hpp"
#include "../../tools/tools.h"
#include <ctype.h>

static Fl_Input* input_modify_type;
static Fl_Input* input_modify_address;
static Fl_Input* input_modify_value;
static Fl_Input* input_modify_offset;
static Fl_Input* input_modify_name;
static int location = -1;

void UI::create_window_address_modifier()
{
    window_address_modifier = new Fl_Window(350, 210);
    window_address_modifier->label("modify address");

    input_modify_address = new Fl_Input(90, 10, 220, 30, "address: ");
    style_input(input_modify_address);

    input_modify_offset = new Fl_Input(90, 50, 220, 30, "offset: ");
    style_input(input_modify_offset);

    input_modify_name = new Fl_Input(90, 90, 220, 30, "name: ");
    style_input(input_modify_name);

    input_modify_type = new Fl_Input(90, 130, 220, 30, "type: ");
    style_input(input_modify_type);

    input_modify_value = new Fl_Input(90, 170, 220, 30, "value: ");
    style_input(input_modify_value);

    window_address_modifier->callback(
        [](Fl_Widget* w, void* ui) -> void {
            UI::active->close_address_modifier(w);
        }
    );

    window_address_modifier->hide();
    window_address_modifier->end();
}

void UI::open_address_modifier(Fl_Widget* widget, int locs)
{
    const char* label_addr = (widget) ? widget->label() : "";
    size_t sz = (widget) ? (sizeof(stored_addresses) / sizeof(stored_addresses[0])) : 0;
    for (int i = 0; i < sz; i++) {
        if (stored_addresses[i].entry_string == label_addr)
            location = i;
    }
    if (locs >= 0) location = locs;
    if (location < 0) return;

    stored_address_entry* entry = &stored_addresses[location];
    size_t siz_name = strlen(entry->name);

    input_modify_name->value(entry->name);
    input_modify_type->value(entry->type);

    char as_string[512];
    snprintf(as_string, sizeof(as_string), "%lx", entry->address);
    input_modify_address->value((siz_name) ? as_string : "");
    snprintf(as_string, sizeof(as_string), "%x", entry->offset);
    input_modify_offset->value((siz_name) ? as_string : "");

    if (!siz_name) {
        input_modify_value->value("");
    }
    else if (entry->type_switch == VALUE_TYPE_INT) {
        snprintf(as_string, sizeof(as_string), "%ld", entry->value);
        input_modify_value->value(as_string);
    }
    else if (entry->type_switch == VALUE_TYPE_FLOAT) {
        snprintf(as_string, sizeof(as_string), "%lf", (double)entry->value_float);
        input_modify_value->value(as_string);
    }
    else if (entry->type_switch == VALUE_TYPE_STRING) {
        input_modify_value->value((const char*)entry->value);
    }

    window_address_modifier->show();
}

void UI::close_address_modifier(Fl_Widget* widget)
{
    window_address_modifier->hide();
    if (location < 0) return;

    if (strlen(input_modify_name->value()) == 0 ||
        strlen(input_modify_address->value()) == 0)
        return delete_stored_address(location);

    stored_address_entry* entry = &stored_addresses[location];

    size_t address;
    uint32_t offset;
    sscanf(input_modify_address->value(), "%lx", &address);
    sscanf(input_modify_offset->value(), "%x", &offset);

    entry->address = address;
    entry->offset = offset;
    strcpy(entry->name, input_modify_name->value());
    strcpy(entry->type, input_modify_type->value());

    if (entry->type_switch == VALUE_TYPE_INT)
        sscanf(input_modify_value->value(), "%ld", &(entry->value));
    if (entry->type_switch == VALUE_TYPE_FLOAT)
        sscanf(input_modify_value->value(), "%lf", &(entry->value_float));
    if (entry->type_switch == VALUE_TYPE_STRING)
        strcpy((char*)entry->value, input_modify_value->value());

    snprintf(entry->entry_string, sizeof(entry->entry_string),
        "  %12lx:%5x%11s%10s%14s", address, offset,
        input_modify_value->value(), input_modify_type->value(), input_modify_name->value());

    static char command[512];
    static char type[255];
    strncpy(type, entry->type, 255);
    if (strcmp(type, "any int") == 0)
        strcpy(type, "int32");
    if (strcmp(type, "any float") == 0)
        strcpy(type, "float32");

    if (entry->type_switch == VALUE_TYPE_STRING)
        snprintf(command, sizeof(command), "write %s %lx %s", type, entry->address, (const char*)entry->value);
    if (entry->type_switch == VALUE_TYPE_INT)
        snprintf(command, sizeof(command), "write %s %lx %ld", type, entry->address, entry->value);
    if (entry->type_switch == VALUE_TYPE_FLOAT)
        snprintf(command, sizeof(command), "write %s %lx %lf", type, entry->address, entry->value_float);
    scan_command(command);

    stored_address_scroller->redraw();
}
