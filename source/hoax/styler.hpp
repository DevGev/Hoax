#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Input.H>

static Fl_Color fg_main_color = fl_rgb_color(0xe2, 0x1b, 0x5a);
static Fl_Color fg_accent_color = fl_rgb_color(0x37, 0x52, 0x50);

static void style_scroller(Fl_Scroll* scroller)
{
    scroller->scrollbar_size(5);
    scroller->scroll_to(0, -35);
}

static void style_scroller_item(Fl_Widget* item, int index)
{
    item->color(((index % 2) == 0) ? fl_rgb_color(0x0f, 0x0f, 0x0f) : fl_rgb_color(0x14, 0x14, 0x14));
    item->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    item->labelcolor((index == 0) ? fg_main_color : fl_rgb_color(0xcd, 0xcd, 0xcd));
    item->labelsize(10);
    item->labelfont(FL_COURIER);
    item->visible_focus(false);
    item->box(_FL_RFLAT_BOX);
}

static void style_button(Fl_Button* button)
{
    button->labelcolor(fl_rgb_color(0xdd, 0xdd, 0xdd));
    button->labelfont(FL_COURIER);
    button->labelsize(12);
    button->color(fg_main_color);
    button->visible_focus(false);
    button->box(_FL_OVAL_FRAME);
}

static void style_input(Fl_Input* input)
{
    input->textcolor(FL_WHITE);
    input->textfont(FL_COURIER);
    input->textsize(12);
    input->labelcolor(fg_main_color);
    input->labelfont(FL_COURIER);
    input->labelsize(12);
    input->box(_FL_RFLAT_BOX);
    input->color(fl_rgb_color(0x12, 0x12, 0x12));
}

static void style_menu_button(Fl_Menu_Button* item)
{
    item->color(fl_rgb_color(0x0d, 0x0d, 0x0d), FL_BLUE);
    item->size(15, 25);
    item->textfont(FL_COURIER);
    item->textsize(11);
    item->textcolor(FL_WHITE);
    item->box(_FL_RFLAT_BOX);
    item->visible_focus(0);
}

static void style_box(Fl_Box* box)
{
    box->labelcolor(fl_rgb_color(0xdd, 0xdd, 0xdd));
    box->labelfont(FL_COURIER);
    box->labelsize(11);
    box->labelcolor(fl_rgb_color(0xcc, 0xcc, 0xcc));
    box->color(fl_rgb_color(0x12, 0x12, 0x12));
    box->box(_FL_RFLAT_BOX);
    box->align(FL_ALIGN_CENTER | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
}

static void style_help_view(Fl_Help_View* hw)
{
    hw->textfont(FL_COURIER);
    hw->textsize(12);
    hw->textcolor(fl_rgb_color(0xee, 0xee, 0xee));
    hw->color(fl_rgb_color(0x8, 0x8, 0x8));
    hw->box(_FL_RFLAT_BOX);
    hw->scrollbar_size(5);
}
