#include "hoax/ui.hpp"
#include "tools/log.h"
#include <unistd.h>

int main(int argc, char **argv)
{
    if (getuid())
        debug(DEBUG_WARNING, "run as sudo");

    UI ui;
    ui.show(argc, argv);
}
