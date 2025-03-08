#include "../ui.hpp"
#include "../styler.hpp"
#include "../../tools/tools.h"
#include <ctype.h>

static Fl_Widget* store_widget;
static Fl_Box* address_box;
static int location = -1;

void UI::create_window_address_actions()
{
    window_address_actions = new Fl_Window(350, 200);
    window_address_actions->label("modify address");

    address_box = new Fl_Box(10, 10, 330, 25, "not known");
    style_box(address_box);
    address_box->align(FL_ALIGN_CENTER);
    address_box->color(fl_rgb_color(0xe, 0xe, 0xe));

    Fl_Button* button_remove = new Fl_Button(10, 45, 330, 23, "remove");
    style_button(button_remove);
    button_remove->callback(
        [](Fl_Widget* w, void*) -> void {
            if (location == -1) return;
            UI::active->delete_stored_address(location);
            UI::active->window_address_actions->hide();
        }
    );

    Fl_Button* button_watch = new Fl_Button(10, 75, 330, 25, "watch");
    style_button(button_watch);
    button_watch->callback(
        [](Fl_Widget* w, void*) -> void {
            if (location == -1) return;
            UI::active->window_address_actions->hide();
            UI::active->open_memory_debugger("watch", location);
        }
    );

    Fl_Button* button_rwatch = new Fl_Button(10, 105, 330, 25, "read watch");
    style_button(button_rwatch);
    button_rwatch->callback(
        [](Fl_Widget* w, void*) -> void {
            if (location == -1) return;
            UI::active->window_address_actions->hide();
            UI::active->open_memory_debugger("rwatch", location);
        }
    );

    Fl_Button* button_wwatch = new Fl_Button(10, 135, 330, 25, "write watch");
    style_button(button_wwatch);
    button_wwatch->callback(
        [](Fl_Widget* w, void*) -> void {
            if (location == -1) return;
            UI::active->window_address_actions->hide();
            UI::active->open_memory_debugger("wwatch", location);
        }
    );

    Fl_Button* button_memview = new Fl_Button(10, 165, 330, 25, "memory view");
    style_button(button_memview);
    button_memview->callback(
        [](Fl_Widget* w, void*) -> void {
            if (location == -1) return;
            UI::active->open_memory_viewer(location);
            UI::active->window_address_actions->hide();
        }
    );

    window_address_actions->hide();
    window_address_actions->end();
}

void UI::open_address_actions(Fl_Widget* widget)
{
    store_widget = widget;

    const char* label_addr = widget->label();
    for (int i = 0; i < sizeof(stored_addresses) / sizeof(stored_addresses[0]); i++) {
        if (stored_addresses[i].entry_string == label_addr)
            location = i;
    }
    if (location < 0) return;

    static char buffer[512];
    snprintf(buffer, sizeof(buffer), "[0x%lx]", stored_addresses[location].address);
    address_box->label(buffer);

    window_address_actions->show();
}
