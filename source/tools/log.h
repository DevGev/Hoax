#pragma once

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define DEBUG_INFO 0
#define DEBUG_WARNING 1
#define DEBUG_ERROR 2

void debug(int level, const char* message)
{
    if (level == 0) printf("\033[32mINFO");
    if (level == 1) printf("\033[33mWARNING");
    if (level == 2) printf("\033[34mERROR");
    printf(":\033[0m %s\n", message);
}
