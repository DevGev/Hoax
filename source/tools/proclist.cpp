#include "tools.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <sys/stat.h>

#define PROC_DIR "/proc/"
#define EXE_LINK "/exe"

bool isnumber(const char *string)
{
	while (string[0] != '\0') {
		if (!isdigit(string[0])) {
			return false;
		}

		++string;
	}

	return true;
}

bool getlinkedpath(const char *linkpath, char *linkedpath)
{
	if (!realpath(linkpath, linkedpath))
		return false;

	return true;
}

process_entry* proclist()
{
    static process_entry list[1024];
    memset(list, 0, sizeof(list));
    list[0].name[0] = 0;
    list[0].pid = 0;
    size_t list_index = 0;

	DIR* dir = opendir(PROC_DIR);
	if (dir == NULL)
		return NULL;

	struct dirent *de;
	while ((de = readdir(dir))) {
		const char *pid = de->d_name;
		if (!isnumber(pid)) {
			continue;
		}

		char exelinkpath[PATH_MAX];
		snprintf(exelinkpath, sizeof(exelinkpath), "%s%s%s", PROC_DIR, pid, EXE_LINK);

		struct stat st;
		if (stat(exelinkpath, &st) == -1) {
			continue;
		}

		char path[PATH_MAX];
		if (!getlinkedpath(exelinkpath, path)) {
			continue;
		}

        if (list_index >= sizeof(list) / sizeof(list[0]) - 1)
            break;

        strcpy(list[list_index].name, basename(path));
        list[list_index].pid = atoi(pid);
        list_index++;
	}

	closedir(dir);

	return list;
}
