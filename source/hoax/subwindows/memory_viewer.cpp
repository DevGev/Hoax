#include "../ui.hpp"
#include "../styler.hpp"
#include "../../tools/tools.h"
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>

#define MAX_SLAVE_WINDOWS 10

typedef struct {
    Fl_Window* window;
    Fl_Input* input;
    Fl_Button* button;
    Fl_Check_Button* check_button;
    Fl_Help_View* help_view;
    bool is_in_use;
} window_entry;

static const char* default_msg =
    "press <font color=red>debug</font> "
    "to start <font color=yellow>session</font>";

static const char* running_msg =
    "a <font color=red>debug session</font> is running";

static Fl_Input* debug_input = 0;
static Fl_Check_Button* button_open_in_new = 0;
static Fl_Button* button_debug = 0;
static window_entry slave_windows[MAX_SLAVE_WINDOWS];
static window_entry current_entry;
static uint32_t disasmd_fifo_id = 0;
static int disasmd_fifo_in_fd = -1;
static int disasmd_fifo_out_fd = -1;
static int last_spawned_id = 0;

void UI::spawn_disasmd()
{
    srand(time(NULL));
    system("sudo rm /tmp/i_disasmd_pipe_*");
    system("sudo rm /tmp/o_disasmd_pipe_*");

    disasmd_fifo_id = rand() % 0xFFF;
    char fifo_name[BUFSIZ];
    snprintf(fifo_name, BUFSIZ, "/tmp/o_disasmd_pipe_%d", disasmd_fifo_id);

    fifo_name[5] = 'o';
    mkfifo(fifo_name, 0666);
    disasmd_fifo_out_fd = open(fifo_name, O_RDONLY|O_NONBLOCK);

    char command[BUFSIZ];
    snprintf(command, BUFSIZ, "./disasmd.py %d %d & disown", disasmd_fifo_id, selected_pid);
    system(command);

    usleep(10000);

    fifo_name[5] = 'i';
    mkfifo(fifo_name, 0666);
    disasmd_fifo_in_fd = open(fifo_name, O_WRONLY);
}

static void helper_close_window(Fl_Widget* window, void*)
{
    for (int i = 0; i < MAX_SLAVE_WINDOWS; i++) {
        if (slave_windows[i].window == window)
            slave_windows[i].is_in_use = false;
    }
    window->hide();
}

void UI::helper_create_window_memory_viewer()
{
    window_memory_viewer = new Fl_Window(570, 580);
    window_memory_viewer->label("memory viewer");
    window_memory_viewer->callback(helper_close_window);

    debug_input = new Fl_Input(100, 10, 200, 20);
    style_input(debug_input);

    button_debug = new Fl_Button(10, 10, 70, 19, "debug");
    style_button(button_debug);

    button_debug->callback(
        [](Fl_Widget* w, void*) -> void {
            for (int i = 0; i < MAX_SLAVE_WINDOWS; i++) {
                if (slave_windows[i].button == w)
                    current_entry = slave_windows[i];
            }
            UI::active->send_disasm(current_entry.input->value());
        }
    );

    button_open_in_new = new Fl_Check_Button(320, 10, 110, 20, " open in new window");
    style_button(button_open_in_new);
    button_open_in_new->box(FL_NO_BOX);

    char* id_as_string = new char[5];
    snprintf(id_as_string, sizeof(id_as_string), "%d", last_spawned_id + 1);
    Fl_Box* label_id = new Fl_Box(555, 0, 15, 20,
        (last_spawned_id == MAX_SLAVE_WINDOWS - 1) ? "M" : id_as_string);
    style_box(label_id);

    instruction_view = new Fl_Help_View(10, 40, 550, 525);
    style_help_view(instruction_view);
    instruction_view->value(default_msg);

    window_memory_viewer->end();
    window_memory_viewer->hide();
}

void UI::create_window_memory_viewer()
{
    for (int i = 0; i < MAX_SLAVE_WINDOWS; i++) {
        last_spawned_id = i;
        helper_create_window_memory_viewer();
        slave_windows[i].window = window_memory_viewer;
        slave_windows[i].check_button = button_open_in_new;
        slave_windows[i].button = button_debug;
        slave_windows[i].input = debug_input;
        slave_windows[i].help_view = instruction_view;
        slave_windows[i].is_in_use = 0;
        current_entry = slave_windows[i];
    }

    current_entry.is_in_use = true;
}

void UI::close_memory_viewer(Fl_Widget* widget)
{
    window_memory_viewer->hide();
}

static void open_new_window()
{
    for (int i = 0; i < MAX_SLAVE_WINDOWS; i++) {
        if (slave_windows[i].is_in_use)
            continue;
        slave_windows[i].is_in_use = true;
        current_entry = slave_windows[i];
        current_entry.window->show();
        break;
    }
}

void UI::send_disasm(const char* command)
{
    if (disasmd_fifo_in_fd < 0 || disasmd_fifo_out_fd < 0)
        spawn_disasmd();

    if (current_entry.check_button->value() == 1) {
        open_new_window();
        current_entry.input->value(command);
    }

    static char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s\n", command);
    write(disasmd_fifo_in_fd, buffer, strlen(buffer));
    disasmd_requires_check = true;
    current_entry.help_view->value(running_msg);
}

void UI::update_address_value(size_t address, int data_type, void* update_at)
{
    if (disasmd_fifo_in_fd < 0 || disasmd_fifo_out_fd < 0)
        spawn_disasmd();

    static char buffer[1024];
    snprintf(buffer, sizeof(buffer), "address_value %s %lx\n", 
        ((data_type == VALUE_TYPE_INT) ? "int" :
         (data_type == VALUE_TYPE_FLOAT) ? "float" :
         (data_type == VALUE_TYPE_STRING) ? "string" : "int"),
        address);
    write(disasmd_fifo_in_fd, buffer, strlen(buffer));

    pollfd polls[1];
    polls[0].fd = disasmd_fifo_out_fd;
    polls[0].events = POLLIN;
    int ret = poll(polls, 1, -1);

    int s = 1;
    size_t offset = 0;
    while (s > 0) {
        s = read(disasmd_fifo_out_fd, buffer + offset, sizeof(buffer) - offset);
        offset += s;
    }

    if (data_type == VALUE_TYPE_INT)
        sscanf(buffer, "%ld", (size_t*)update_at);
    if (data_type == VALUE_TYPE_FLOAT)
        sscanf(buffer, "%lf", (double*)update_at);
    if (data_type == VALUE_TYPE_STRING)
        sscanf(buffer, "%s", (char*)update_at);
}

void UI::update_disasm_output()
{
    pollfd polls[1];
    polls[0].fd = disasmd_fifo_out_fd;
    polls[0].events = POLLIN;
    int ret = poll(polls, 1, 0);
    if (ret < 1) return;

    static char buffer[4096 * 4096];
    int s = 1;
    size_t offset = 0;
    while (s > 0) {
        s = read(disasmd_fifo_out_fd, buffer + offset, sizeof(buffer) - offset);
        offset += s;
    }

    current_entry.help_view->value(buffer);
    disasmd_requires_check = false;
}

void UI::open_memory_debugger(const char* command, uint32_t location)
{
    window_memory_viewer->show();
    char buffer[512];
    size_t address = stored_addresses[location].address;
    snprintf(buffer, sizeof(buffer), "%s %lx", command, address);
    debug_input->value(buffer);
    instruction_view->value(default_msg);
    Fl::flush();
}

void UI::open_memory_viewer(uint32_t location)
{
    size_t address = stored_addresses[location].address;
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "view %lx", address);
    send_disasm(buffer);
    debug_input->value(buffer);
    window_memory_viewer->show();
}

void UI::kill_disasmd_server()
{
    disasmd_fifo_id = -1;
    disasmd_fifo_in_fd = -1;
    disasmd_fifo_out_fd = -1;
    disasmd_requires_check = false;
}
