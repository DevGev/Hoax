#include "../ui.hpp"
#include "../styler.hpp"
#include "../../tools/tools.h"
#include <ctype.h>

static Fl_Window* notepad_window = 0;

void UI::create_window_notepad()
{
    notepad_window = new Fl_Window(340, 480);
    window_address_modifier->label("notepad");
    Fl_Multiline_Input* notepad = new Fl_Multiline_Input(10, 10, 320, 460);
    notepad->value("notes...");
    notepad->textfont(FL_COURIER);
    notepad->box(_FL_RFLAT_BOX);
    notepad->textsize(13);
    notepad_window->end();
    notepad_window->hide();
}

void UI::open_notepad()
{
    notepad_window->show();
}

void UI::close_notepad()
{
    notepad_window->hide();
}
