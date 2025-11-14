#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // getpid() e geteuid()

// Módulos existentes
#include "monitor.h"
#include "namespace.h"
#include "cgroup.h"

void print_usage(const char *prog_name) {
    fprintf(stderr, "Uso: %s <comando> [argumentos...]\n\n", prog_name);

    fprintf(stderr, "===== COMANDOS DO ALUNO 3 (NAMESPACES) =====\n");
    fprintf(stderr, "  ns_list <pid>                     - Lista todos os namespaces de um processo\n");
    fprintf(stderr, "  ns_compare <pid1> <pid2>          - Compara namespaces entre dois processos\n");
    fprintf(stderr, "  ns_members <type> <inode>         - Lista processos que compartilham um namespace\n");
    fprintf(stderr, "  ns_overhead                       - Mede o overhead de criação de namespaces\n");
    fprintf(stderr, "  ns_report <arquivo.csv>           - Gera relatório completo dos namespaces do sistema\n\n");

    fprintf(stderr, "===== COMANDOS DO ALUNO 4 (CGROUPS, requer sudo) =====\n");
    fprintf(stderr, "  create <controller> <group_name>  - Cria um cgroup\n");
    fprintf(stderr, "  move <controller> <group_name> <pid> - Move um PID para o cgroup\n");
    fprintf(stderr, "  set_mem <group> <bytes>           - Define limite de memória\n");
    fprintf(stderr, "  set_cpu <group> <cores>           - Define limite de CPU\n");
    fprintf(stderr, "  get_mem <group>                   - Lê uso de memória\n");
    fprintf(stderr, "  get_cpu <group>                   - Lê uso de CPU\n");
    fprintf(stderr, "  get_io <group>                    - Lê estatísticas de I/O\n");
    fprintf(stderr, "  stress_test <group>               - Executa teste de estresse no grupo\n");
}

int run_stress_test(const char *group_name) {
    pid_t self_pid = getpid();
    printf("Iniciando teste de estresse (PID: %d) no grupo: %s\n", self_pid, group_name);

    if (cgroup_create("cpu", group_name) != 0) return -1;
    if (cgroup_create("memory", group_name) != 0) return -1;

    if (cgroup_move_pid("cpu", group_name, self_pid) != 0) return -1;
    if (cgroup_move_pid("memory", group_name, self_pid) != 0) return -1;

    long long mem_limit = 100 * 1024 * 1024;
    if (cgroup_set_memory_limit(group_name, mem_limit) != 0) return -1;

    double cpu_limit = 0.5;
    if (cgroup_set_cpu_limit(group_name, cpu_limit, 100000) != 0) return -1;

    printf("Limites aplicados. Pressione Enter para continuar...\n");
    getchar();

    long long mem_usage = cgroup_get_memory_usage(group_name);
    long long cpu_usage = cgroup_get_cpu_usage(group_name);
    CgroupIOStats io_stats = cgroup_get_io_stats(group_name);

    printf("--- Métricas Finais ---\n");
    printf("Memória usada: %lld bytes\n", mem_usage);
    printf("CPU usada: %lld us\n", cpu_usage);
    if (io_stats.rbytes != -1) {
        printf("I/O Lidos: %lld\n", io_stats.rbytes);
        printf("I/O Escritos: %lld\n", io_stats.wbytes);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    // Aviso sobre sudo (somente para cgroups)
    if (geteuid() != 0 &&
        (strcmp(command, "create") == 0 ||
         strcmp(command, "move") == 0 ||
         strcmp(command, "set_mem") == 0 ||
         strcmp(command, "set_cpu") == 0 ||
         strcmp(command, "stress_test") == 0)) {

        fprintf(stderr, "AVISO: Este comando deve ser executado com sudo.\n");
    }

    // ==============================
    // 🔵 COMANDOS DO ALUNO 3 (NAMESPACES)
    // ==============================

    if (strcmp(command, "ns_list") == 0 && argc == 3) {
        list_process_namespaces(atoi(argv[2]));
    }

    else if (strcmp(command, "ns_compare") == 0 && argc == 4) {
        compare_namespaces(atoi(argv[2]), atoi(argv[3]));
    }

    else if (strcmp(command, "ns_members") == 0 && argc == 4) {
        list_namespace_members(argv[2], atoll(argv[3]));
    }

    else if (strcmp(command, "ns_overhead") == 0) {
        measure_namespace_overhead();
    }

    else if (strcmp(command, "ns_report") == 0 && argc == 3) {
        generate_namespace_report(argv[2]);
    }

    // ==============================
    // 🔵 COMANDOS DO ALUNO 4 (CGROUPS)
    // ==============================

    else if (strcmp(command, "create") == 0 && argc == 4) {
        if (cgroup_create(argv[2], argv[3]) != 0)
            fprintf(stderr, "Falha ao criar cgroup.\n");
    }

    else if (strcmp(command, "move") == 0 && argc == 5) {
        pid_t pid = atoi(argv[4]);
        if (cgroup_move_pid(argv[2], argv[3], pid) != 0)
            fprintf(stderr, "Falha ao mover PID.\n");
    }

    else if (strcmp(command, "set_mem") == 0 && argc == 4) {
        if (cgroup_set_memory_limit(argv[2], atoll(argv[3])) != 0)
            fprintf(stderr, "Falha ao definir limite de memória.\n");
    }

    else if (strcmp(command, "set_cpu") == 0 && argc == 4) {
        if (cgroup_set_cpu_limit(argv[2], atof(argv[3]), 100000) != 0)
            fprintf(stderr, "Falha ao definir limite de CPU.\n");
    }

    else if (strcmp(command, "get_mem") == 0 && argc == 3) {
        printf("Memória usada: %lld\n", cgroup_get_memory_usage(argv[2]));
    }

    else if (strcmp(command, "get_cpu") == 0 && argc == 3) {
        printf("CPU usada: %lld us\n", cgroup_get_cpu_usage(argv[2]));
    }

    else if (strcmp(command, "get_io") == 0 && argc == 3) {
        CgroupIOStats io = cgroup_get_io_stats(argv[2]);
        printf("Lidos: %lld bytes\nEscritos: %lld bytes\n", io.rbytes, io.wbytes);
    }

    else if (strcmp(command, "stress_test") == 0 && argc == 3) {
        run_stress_test(argv[2]);
    }

    else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}