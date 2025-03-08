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

static uint32_t scanmemd_fifo_id = 0;
static int scanmemd_fifo_in_fd = -1;
static int scanmemd_fifo_out_fd = -1;

int UI::update_scan_list(const char* data)
{
    for (int i = 0; i < MAX_ADDRESS_ITEMS; i++)
        address_items_labels[i][0] = 0;

    int i = 0;
    int item_i = 0;
    char line[1024];

    while (data[i]) {
        int line_i = 0;
        for (; data[i] != '\n' && data[i] != 0;)
            line[line_i++] = data[i++];
        line[line_i] = 0;
        i++;

        if (item_i >= MAX_ADDRESS_ITEMS) break;
        strcpy(address_items_labels[item_i], line);
        item_i++;
    }
    return item_i;
}

void UI::scan_command(const char* command, bool is_search, bool update_status)
{
    if (update_status) {
        scan_status_label->label("running scan");
        scan_status_label->redraw();
        Fl::flush();
    }

    if (scanmemd_fifo_in_fd < 0 || scanmemd_fifo_out_fd < 0) {
        system("sudo rm /tmp/i_scanmemd_pipe_*");
        system("sudo rm /tmp/o_scanmemd_pipe_*");

        srand(time(NULL));
        scanmemd_fifo_id = rand() % 0xFFF;
        char fifo_name[BUFSIZ];
        snprintf(fifo_name, BUFSIZ, "/tmp/o_scanmemd_pipe_%d", scanmemd_fifo_id);

        fifo_name[5] = 'o';
        mkfifo(fifo_name, 0666);
        scanmemd_fifo_out_fd = open(fifo_name, O_RDONLY|O_NONBLOCK);

        char command[BUFSIZ];
        snprintf(command, BUFSIZ, "./scanmemd.py %d %d & disown", scanmemd_fifo_id, selected_pid);
        system(command);

        usleep(10000);

        fifo_name[5] = 'i';
        mkfifo(fifo_name, 0666);
        scanmemd_fifo_in_fd = open(fifo_name, O_WRONLY);
    }

    char scan_type[50];
    if (strlen(scan_type_choice->value()) < 3)
        strncpy(scan_type, "int\0", 4);
    else if (scan_type_choice->value()[0] == 'a' &&
        scan_type_choice->value()[1] == 'n' &&
        scan_type_choice->value()[2] == 'y')
        strcpy(scan_type, scan_type_choice->value() + 4);
    else
        strcpy(scan_type, scan_type_choice->value());

    static char buffer[4096 * 4096];
    int buffer_offset = 0;

    if (changed_pid) {
        changed_pid = false;
        buffer_offset = snprintf(buffer, sizeof(buffer), "pid %d|", selected_pid);
    }

    snprintf(buffer + buffer_offset, sizeof(buffer),
        "option scan_data_type %s|%s%s\n",
        scan_type,
        (strcmp(scan_type, "string") == 0 && is_search)
        ? "\" " : "", command);

    write(scanmemd_fifo_in_fd, buffer, strlen(buffer));

    pollfd polls[1];
    polls[0].fd = scanmemd_fifo_out_fd;
    polls[0].events = POLLIN;
    int ret = poll(polls, 1, -1);

    int s = 1;
    size_t offset = 0;
    while (s > 0) {
        s = read(scanmemd_fifo_out_fd, buffer + offset, sizeof(buffer) - offset);
        offset += s;
    }
    int items = update_scan_list(buffer);

    static char status[512];
    snprintf(status, sizeof(status), "[matches (%d%s)]",
        items, (items >= 500) ? "+" : "");

    if (update_status)
        scan_status_label->label(status);
}

void UI::button_scan_pressed()
{
    scan_command(scan_input->value(), true);
}

void UI::kill_scanmemd_server()
{
    scanmemd_fifo_id = -1;
    scanmemd_fifo_in_fd = -1;
    scanmemd_fifo_out_fd = -1;
}
