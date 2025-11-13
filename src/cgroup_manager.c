#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define CGROUP_BASE_PATH "/sys/fs/cgroup"
#define BUFFER_SIZE 256

// --- Funções Auxiliares (Helpers) ---

static int write_to_cgroup_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Falha ao abrir arquivo cgroup");
        fprintf(stderr, "Caminho: %s\n", path);
        return -1;
    }

    if (write(fd, value, strlen(value)) == -1) {
        perror("Falha ao escrever no arquivo cgroup");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

// Helper para ler arquivos. Usaremos um mais robusto para cpu.stat
static long long read_from_cgroup_file(const char *path) {
    char buffer[BUFFER_SIZE];
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Falha ao ler arquivo cgroup");
        fprintf(stderr, "Caminho: %s\n", path);
        return -1;
    }

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("Falha na leitura (read)");
        close(fd);
        return -1;
    }

    buffer[bytes_read] = '\0';
    close(fd);
    
    return atoll(buffer); 
}

// --- Implementação das Funções Públicas (cgroup v2) ---

/**
 * No cgroup v2, só existe UM diretório por grupo.
 * Ignoramos o 'controller' e apenas criamos o grupo na raiz.
 * O cgroup_create pode ser chamado várias vezes (para 'cpu', 'memory'),
 * mas ele só cria o diretório na primeira vez.
 */
int cgroup_create(const char *controller, const char *group_name) {
    (void)controller; // <-- CORREÇÃO: Informa ao compilador que 'controller' é intencionalmente não usado.
    
    char path[BUFFER_SIZE];
    // Caminho v2: /sys/fs/cgroup/GROUP_NAME
    snprintf(path, sizeof(path), "%s/%s", CGROUP_BASE_PATH, group_name);

    if (mkdir(path, 0755) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Cgroup (v2) '%s' já existe. Ignorando a criação.\n", path);
            return 0; // Já existe, não é um erro
        }
        perror("Falha ao criar diretório cgroup (v2)");
        fprintf(stderr, "Caminho: %s\n", path);
        return -1;
    }
    printf("Cgroup (v2) criado em: %s\n", path);
    return 0;
}
/**
 * No cgroup v2, o arquivo é sempre 'cgroup.procs',
 * independente do 'controller'.
 */

 int cgroup_move_pid(const char *controller, const char *group_name, pid_t pid) {
    (void)controller; // <-- CORREÇÃO: Informa ao compilador que 'controller' é intencionalmente não usado.

    char path[BUFFER_SIZE];
    char pid_str[BUFFER_SIZE];

    // Caminho v2: /sys/fs/cgroup/GROUP_NAME/cgroup.procs
    snprintf(path, sizeof(path), "%s/%s/cgroup.procs", CGROUP_BASE_PATH, group_name);
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    return write_to_cgroup_file(path, pid_str);
}

 int cgroup_set_memory_limit(const char *group_name, long long limit_bytes) {
    char path[BUFFER_SIZE];
    char limit_str[BUFFER_SIZE];

    // Caminho v2: /sys/fs/cgroup/GROUP_NAME/memory.max
    snprintf(path, sizeof(path), "%s/%s/memory.max", CGROUP_BASE_PATH, group_name);
    
    if (limit_bytes <= 0) {
        snprintf(limit_str, sizeof(limit_str), "max"); // "max" significa sem limite
    } else {
        snprintf(limit_str, sizeof(limit_str), "%lld", limit_bytes);
    }

    return write_to_cgroup_file(path, limit_str);
}

int cgroup_set_cpu_limit(const char *group_name, double cores, long period_us) {
    char path[BUFFER_SIZE];
    char value_str[BUFFER_SIZE];

    long quota_us = (long)(cores * (double)period_us);

    // Caminho v2: /sys/fs/cgroup/GROUP_NAME/cpu.max
    // O formato é: $QUOTA $PERIOD
    snprintf(path, sizeof(path), "%s/%s/cpu.max", CGROUP_BASE_PATH, group_name);
    snprintf(value_str, sizeof(value_str), "%ld %ld", quota_us, period_us);
    
    printf("Limite de CPU (v2) definido: %ld us (quota) / %ld us (period)\n", quota_us, period_us);
    return write_to_cgroup_file(path, value_str);
}

long long cgroup_get_memory_usage(const char *group_name) {
    char path[BUFFER_SIZE];
    // Caminho v2: /sys/fs/cgroup/GROUP_NAME/memory.current
    snprintf(path, sizeof(path), "%s/%s/memory.current", CGROUP_BASE_PATH, group_name);
    
    return read_from_cgroup_file(path);
}

/**
 * Lê o uso da CPU (v2)
 * Precisamos ler o arquivo 'cpu.stat' e extrair o valor 'usage_usec'.
 */
long long cgroup_get_cpu_usage(const char *group_name) {
    char path[BUFFER_SIZE];
    snprintf(path, sizeof(path), "%s/%s/cpu.stat", CGROUP_BASE_PATH, group_name);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Falha ao ler cpu.stat");
        fprintf(stderr, "Caminho: %s\n", path);
        return -1;
    }

    char line_buffer[BUFFER_SIZE];
    long long usage_usec = -1;

    // Lê linha por linha (ex: "usage_usec 12345")
    while (fgets(line_buffer, sizeof(line_buffer), fp)) {
        if (sscanf(line_buffer, "usage_usec %lld", &usage_usec) == 1) {
            break; // Encontramos!
        }
    }

    fclose(fp);
    return usage_usec; // Retorna em microssegundos
}


/**
 * Lê as estatísticas de I/O (v2) - (BlkIO)
 * Precisamos ler o arquivo 'io.stat' e extrair 'rbytes' e 'wbytes'.
 */
CgroupIOStats cgroup_get_io_stats(const char *group_name) {
    char path[BUFFER_SIZE];
    snprintf(path, sizeof(path), "%s/%s/io.stat", CGROUP_BASE_PATH, group_name);

    CgroupIOStats stats = {-1, -1}; // Inicializa com valores de erro

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Falha ao ler io.stat");
        fprintf(stderr, "Caminho: %s\n", path);
        return stats; // Retorna a struct de erro
    }

    char line_buffer[BUFFER_SIZE];
    long long rbytes_val = -1;
    long long wbytes_val = -1;

    // Lê linha por linha (ex: "rbytes 12345" ou "wbytes 67890")
    while (fgets(line_buffer, sizeof(line_buffer), fp)) {
        // Tenta encontrar rbytes
        if (sscanf(line_buffer, "rbytes %lld", &rbytes_val) == 1) {
            stats.rbytes = rbytes_val;
        }
        // Tenta encontrar wbytes
        if (sscanf(line_buffer, "wbytes %lld", &wbytes_val) == 1) {
            stats.wbytes = wbytes_val;
        }
    }

    // Caso um dos valores não seja encontrado no arquivo
    if (stats.rbytes == -1) stats.rbytes = 0;
    if (stats.wbytes == -1) stats.wbytes = 0;

    fclose(fp);
    return stats;
}