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

void compare_namespaces(pid_t p1, pid_t p2) {
    printf("Comparando namespaces entre %d e %d\n", p1, p2);

    for (int i = 0; i < namespace_count; i++) {
        char path1[256], path2[256];
        snprintf(path1, sizeof(path1), NS_PATH_FMT, p1, namespace_types[i]);
        snprintf(path2, sizeof(path2), NS_PATH_FMT, p2, namespace_types[i]);

        long long i1 = get_inode(path1);
        long long i2 = get_inode(path2);

        printf("%-6s: %s\n", namespace_types[i], (i1 == i2 ? "Compartilham" : "Diferentes"));
    }
}

void list_namespace_members(const char *type, long long inode) {
    printf("Processos no namespace %s inode=%lld:\n", type, inode);

    DIR *d = opendir("/proc");
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!isdigit(ent->d_name[0])) continue;

        pid_t pid = atoi(ent->d_name);

        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/ns/%s", pid, type);

        if (get_inode(path) == inode)
            printf("  PID %d\n", pid);
    }

    closedir(d);
}

void measure_namespace_overhead(void) {
    printf("Medindo overhead de criação de namespaces...\n");

    // Você deve implementar clone() com cada flag
    // Exemplo de tabela final:
    // pid     | 145 us
    // net     | 320 us
    // user    | 190 us

    // Use CLOCK_MONOTONIC
}

void generate_namespace_report(const char *file) {
    FILE *f = fopen(file, "w");
    if (!f) return;

    fprintf(f, "namespace,inode,pid_count\n");

    // Você deve coletar todos os namespaces existentes no sistema
    // e contar quantos processos pertencem a cada um.

    fclose(f);
}