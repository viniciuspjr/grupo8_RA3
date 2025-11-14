#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <sys/types.h>

typedef struct {
    char name[32];
    long long inode;
} NamespaceInfo;

void list_process_namespaces(pid_t pid);
void compare_namespaces(pid_t pid1, pid_t pid2);
void list_namespace_members(const char *type, long long inode);
void measure_namespace_overhead(void);
void generate_namespace_report(const char *output_file);

#endif
