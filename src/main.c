#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // para getpid() e geteuid()

// Headers dos outros módulos
#include "monitor.h"
#include "namespace.h"

// Header do Aluno 4 
#include "cgroup.h"

void print_usage(const char *prog_name) {
    fprintf(stderr, "Uso: %s <comando> [argumentos...]\n", prog_name);
    fprintf(stderr, "Comandos do Aluno 4 (requer sudo):\n");
    fprintf(stderr, "  create <controller> <group_name>         - Cria um cgroup\n");
    fprintf(stderr, "  move <controller> <group_name> <pid>   - Move um PID para um cgroup\n");
    fprintf(stderr, "  set_mem <group_name> <bytes>             - Define limite de memória (ex: 100M)\n");
    fprintf(stderr, "  set_cpu <group_name> <cores>             - Define limite de CPU (ex: 0.5)\n");
    fprintf(stderr, "  get_mem <group_name>                     - Lê uso de memória\n");
    fprintf(stderr, "  get_cpu <group_name>                     - Lê uso de CPU (us)\n");
    fprintf(stderr, "  get_io <group_name>                      - Lê uso de I/O (bytes)\n"); // NOVO
    fprintf(stderr, "  stress_test <group_name>                 - Cria e move o PID atual para teste\n");
}

// Função de teste simples (ATUALIZADA)
int run_stress_test(const char *group_name) {
    pid_t self_pid = getpid();
    printf("Iniciando teste de estresse (PID: %d) no grupo: %s\n", self_pid, group_name);

    // 1. Criar cgroups (no v2, as duas chamadas funcionam no mesmo grupo)
    if (cgroup_create("cpu", group_name) != 0) return -1;
    if (cgroup_create("memory", group_name) != 0) return -1;
    // O I/O é ativado no mesmo grupo

    // 2. Mover a si mesmo para os cgroups
    if (cgroup_move_pid("cpu", group_name, self_pid) != 0) return -1;
    if (cgroup_move_pid("memory", group_name, self_pid) != 0) return -1;

    // 3. Definir limites (100MB e 0.5 core)
    long long mem_limit = 100 * 1024 * 1024; // 100MB
    if (cgroup_set_memory_limit(group_name, mem_limit) != 0) return -1;
    
    double cpu_limit = 0.5; // 50% de 1 core
    if (cgroup_set_cpu_limit(group_name, cpu_limit, 100000) != 0) return -1;

    printf("Limites aplicados. Pressione Enter para verificar métricas e sair...\n");
    printf("(Em outro terminal, verifique: cat /sys/fs/cgroup/%s/cpu.stat)\n", group_name);
    getchar(); // Pausa para o usuário poder verificar

    // 4. Ler métricas antes de sair
    long long mem_usage = cgroup_get_memory_usage(group_name);
    long long cpu_usage = cgroup_get_cpu_usage(group_name);
    CgroupIOStats io_stats = cgroup_get_io_stats(group_name); // NOVO

    printf("--- Métricas Finais ---\n");
    printf("Uso de Memória: %lld bytes (de %lld)\n", mem_usage, mem_limit);
    printf("Uso de CPU:     %lld us\n", cpu_usage);
    
    // NOVO Bloco de I/O
    if (io_stats.rbytes != -1) {
        printf("I/O Lidos:      %lld bytes\n", io_stats.rbytes);
        printf("I/O Escritos:   %lld bytes\n", io_stats.wbytes);
    } else {
        printf("I/O Stats:      Falha ao ler (controlador 'io' pode estar desativado)\n");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    if (geteuid() != 0) {
        fprintf(stderr, "AVISO: Este programa deve ser executado como root (sudo) para gerenciar cgroups.\n");
    }

    if (strcmp(command, "create") == 0 && argc == 4) {
        if (cgroup_create(argv[2], argv[3]) != 0) {
            fprintf(stderr, "Falha ao criar cgroup.\n");
        }
    } else if (strcmp(command, "move") == 0 && argc == 5) {
        pid_t pid = atoi(argv[4]);
        if (cgroup_move_pid(argv[2], argv[3], pid) != 0) {
            fprintf(stderr, "Falha ao mover PID.\n");
        }
    } else if (strcmp(command, "set_mem") == 0 && argc == 4) {
        long long limit = atoll(argv[3]);
        if (cgroup_set_memory_limit(argv[2], limit) != 0) {
            fprintf(stderr, "Falha ao definir limite de memória.\n");
        }
    } else if (strcmp(command, "set_cpu") == 0 && argc == 4) {
        double cores = atof(argv[3]);
        if (cgroup_set_cpu_limit(argv[2], cores, 100000) != 0) {
            fprintf(stderr, "Falha ao definir limite de CPU.\n");
        }
    } else if (strcmp(command, "get_mem") == 0 && argc == 3) {
        long long usage = cgroup_get_memory_usage(argv[2]);
        if (usage != -1) printf("Uso de Memória: %lld bytes\n", usage);
    } else if (strcmp(command, "get_cpu") == 0 && argc == 3) {
        long long usage = cgroup_get_cpu_usage(argv[2]);
        if (usage != -1) printf("Uso de CPU: %lld us\n", usage);
    } 
    // NOVO Bloco
    else if (strcmp(command, "get_io") == 0 && argc == 3) {
        CgroupIOStats io_stats = cgroup_get_io_stats(argv[2]);
        if (io_stats.rbytes != -1) {
            printf("I/O Stats para '%s':\n", argv[2]);
            printf("  Lidos:    %lld bytes\n", io_stats.rbytes);
            printf("  Escritos: %lld bytes\n", io_stats.wbytes);
        } else {
            fprintf(stderr, "Falha ao ler I/O stats.\n");
        }
    } 
    else if (strcmp(command, "stress_test") == 0 && argc == 3) {
        run_stress_test(argv[2]);
    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}