#include "namespace.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define NS_PATH_FMT "/proc/%d/ns/%s"
const char *namespace_types[] = {"pid", "net", "mnt", "uts", "ipc", "user", "cgroup"};
const int namespace_count = 7;

static long long get_inode(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    return st.st_ino;
}

void list_process_namespaces(pid_t pid) {
    printf("Namespaces do processo %d\n", pid);
    for (int i = 0; i < namespace_count; i++) {
        char path[256];
        snprintf(path, sizeof(path), NS_PATH_FMT, pid, namespace_types[i]);
        long long inode = get_inode(path);

        if (inode != -1)
            printf("  %-6s -> inode %lld\n", namespace_types[i], inode);
    }
}
