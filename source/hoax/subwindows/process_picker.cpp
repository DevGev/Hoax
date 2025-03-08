#include "../ui.hpp"
#include "../styler.hpp"
#include "../../tools/tools.h"
#include <ctype.h>

void UI::create_window_process_picker()
{
    window_process_picker = new Fl_Window(300, 600);
    window_process_picker->label("process picker");
    Fl_Scroll* process_scroller = new Fl_Scroll(0, 0, 300, 600);

    process_entry* process_list = proclist();
    char* entry_string = 0;
    for (int i = 0; i < 500; i++) {
        Fl_Button* process_item = new Fl_Button(0, 15 + i * 20, 295, 18,
            (entry_string == 0) ? "  pid\t\t\t\t\t  name" : entry_string);

        process_item->user_data(this);
        process_item->callback(
            [](Fl_Widget* w, void* ui) -> void {
                ((UI*)ui)->close_process_picker(w);
            }
        );

        style_scroller_item(process_item, i);

        entry_string = new char[512];
        snprintf(entry_string, 512, "  %4d     %35s",
            process_list[i].pid, process_list[i].name);
    }

    process_scroller->scrollbar_size(5);
    process_scroller->scroll_to(0, 15);
    process_scroller->end();
    window_process_picker->end();
}

void UI::close_process_picker(Fl_Widget* widget)
{
    char pid[25];
    memset(pid, 0, sizeof(pid));
    const char* label = widget->label();
    for (; *label != 0; label++) {
        if (isdigit(*label))
            strncat(pid, label, 1);
    }

    selected_pid = atoi(pid);
    changed_pid = true;
    static char button_label[100];
    memset(button_label, 0, 100);
    snprintf(button_label, sizeof(button_label), "pid (%s)", pid);
    button_open_process_picker->label(button_label);
    button_open_process_picker->hide();
    window_process_picker->hide();
    button_open_process_picker->Fl_Widget::redraw();
}

void UI::open_process_picker()
{
    window_process_picker->show();
}
