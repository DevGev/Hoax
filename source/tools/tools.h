#pragma once

#include <stdlib.h>

typedef struct {
    char name[100];
    pid_t pid;
} process_entry;

process_entry* proclist();
