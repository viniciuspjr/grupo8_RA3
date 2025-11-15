#define _GNU_SOURCE
#include "namespace.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sched.h>
#include <sys/wait.h>
#include <ctype.h>

#define NS_PATH_FMT "/proc/%d/ns/%s"
#define STACK_SIZE 8192

const char *namespace_types[] = {
    "pid", "net", "mnt", "uts", "ipc", "user", "cgroup"
};

const int namespace_count = 7;

typedef struct NSNode {
    long long inode;
    int count;
    struct NSNode *next;
} NSNode;

/* ----------------------------- HELPERS ----------------------------- */

static void add_inode(NSNode **list, long long inode);

static long long get_inode(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    return st.st_ino;
}

static int dummy_child(void *arg) {
    (void)arg;
    return 0;
}

static long long micro_diff(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000000LL +
           (b.tv_nsec - a.tv_nsec) / 1000;
}

/* -------------------- FUNÇÃO 1: LISTAR NAMESPACES -------------------- */

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

/* ---------------- FUNÇÃO 2: COMPARAR NAMESPACES ---------------- */

void compare_namespaces(pid_t p1, pid_t p2) {
    printf("Comparando namespaces entre %d e %d\n", p1, p2);

    for (int i = 0; i < namespace_count; i++) {
        char path1[256], path2[256];

        snprintf(path1, sizeof(path1), NS_PATH_FMT, p1, namespace_types[i]);
        snprintf(path2, sizeof(path2), NS_PATH_FMT, p2, namespace_types[i]);

        long long i1 = get_inode(path1);
        long long i2 = get_inode(path2);

        printf("%-6s: %s\n",
            namespace_types[i],
            (i1 == i2 ? "Compartilham" : "Diferentes"));
    }
}

/* ------------- FUNÇÃO 3: LISTAR PROCESSOS NO MESMO NS ------------- */

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

/* ----------------- FUNÇÃO 4: OVERHEAD DE NAMESPACES ----------------- */

void measure_namespace_overhead(void) {
    printf("=== Overhead de criação de namespaces ===\n");

    struct {
        const char *name;
        int flag;
    } tests[] = {
        {"pid",  CLONE_NEWPID  | SIGCHLD},
        {"net",  CLONE_NEWNET  | SIGCHLD},
        {"mnt",  CLONE_NEWNS   | SIGCHLD},
        {"uts",  CLONE_NEWUTS  | SIGCHLD},
        {"ipc",  CLONE_NEWIPC  | SIGCHLD},
        {"user", CLONE_NEWUSER | SIGCHLD},
    };

    int test_count = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < test_count; i++) {
        long long total = 0;
        int iterations = 50;

        for (int n = 0; n < iterations; n++) {
            char *stack = malloc(STACK_SIZE);
            char *stack_top = stack + STACK_SIZE;

            struct timespec t1, t2;
            clock_gettime(CLOCK_MONOTONIC, &t1);

            pid_t pid = clone(dummy_child, stack_top, tests[i].flag, NULL);

            clock_gettime(CLOCK_MONOTONIC, &t2);

            if (pid > 0)
                waitpid(pid, NULL, 0);

            total += micro_diff(t1, t2);
            free(stack);
        }

        long long avg = total / iterations;
        printf("%-6s | %lld us\n", tests[i].name, avg);
    }
}

/* ----------------- FUNÇÃO 5: GERAR RELATÓRIO COMPLETO ----------------- */

void generate_namespace_report(const char *output) {
    FILE *f = fopen(output, "w");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return;
    }

    fprintf(f, "namespace,inode,pid_count\n");

    for (int i = 0; i < namespace_count; i++) {
        const char *type = namespace_types[i];
        NSNode *list = NULL;

        DIR *d = opendir("/proc");
        if (!d) continue;

        struct dirent *ent;
        while ((ent = readdir(d))) {
            if (!isdigit(ent->d_name[0])) continue;

            pid_t pid = atoi(ent->d_name);

            char path[256];
            snprintf(path, sizeof(path), "/proc/%d/ns/%s", pid, type);

            long long inode = get_inode(path);
            if (inode != -1)
                add_inode(&list, inode);
        }

        closedir(d);

        NSNode *cur = list;
        while (cur) {
            fprintf(f, "%s,%lld,%d\n", type, cur->inode, cur->count);
            cur = cur->next;
        }

        while (list) {
            NSNode *tmp = list;
            list = list->next;
            free(tmp);
        }
    }

    fclose(f);
    printf("Relatório completo gerado em %s\n", output);
}

// -------------------- LISTAR TODOS OS NAMESPACES DO SISTEMA --------------------

static void add_inode(NSNode **list, long long inode) {
    NSNode *cur = *list;
    while (cur) {
        if (cur->inode == inode) {
            cur->count++;
            return;
        }
        cur = cur->next;
    }

    NSNode *new = malloc(sizeof(NSNode));
    new->inode = inode;
    new->count = 1;
    new->next = *list;
    *list = new;
}

void list_all_system_namespaces(void) {
    printf("=== Todos os namespaces ativos no sistema ===\n");

    for (int i = 0; i < namespace_count; i++) {
        const char *type = namespace_types[i];
        NSNode *list = NULL;

        DIR *d = opendir("/proc");
        if (!d) continue;

        struct dirent *ent;
        while ((ent = readdir(d))) {
            if (!isdigit(ent->d_name[0])) continue;

            pid_t pid = atoi(ent->d_name);

            char path[256];
            snprintf(path, sizeof(path), "/proc/%d/ns/%s", pid, type);

            long long inode = get_inode(path);
            if (inode != -1)
                add_inode(&list, inode);
        }

        closedir(d);

        printf("\n[%s] Namespaces encontrados:\n", type);

        NSNode *cur = list;
        while (cur) {
            printf("  inode %-12lld  (%d processos)\n", cur->inode, cur->count);
            cur = cur->next;
        }

        // liberar memória
        while (list) {
            NSNode *tmp = list;
            list = list->next;
            free(tmp);
        }
    }
}

