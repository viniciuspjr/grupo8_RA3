#ifndef CGROUP_H
#define CGROUP_H

#include <sys/types.h> // Para pid_t

// --- NOVA STRUCT PARA I/O ---
/**
 * @brief Armazena estatísticas de I/O (BlkIO) lidas do cgroup.
 */
typedef struct {
    long long rbytes; // Bytes lidos
    long long wbytes; // Bytes escritos
} CgroupIOStats;


// --- Funções Principais ---

int cgroup_create(const char *controller, const char *group_name);
int cgroup_move_pid(const char *controller, const char *group_name, pid_t pid);

// Funções de Limite
int cgroup_set_memory_limit(const char *group_name, long long limit_bytes);
int cgroup_set_cpu_limit(const char *group_name, double cores, long period_us);

// Funções de Leitura de Métricas
long long cgroup_get_memory_usage(const char *group_name);
long long cgroup_get_cpu_usage(const char *group_name);

/**
 * @brief Lê as estatísticas de I/O (rbytes/wbytes) de um cgroup (cgroup v2).
 * @param group_name O nome do cgroup.
 * @return Uma struct CgroupIOStats. rbytes será -1 em caso de falha.
 */
CgroupIOStats cgroup_get_io_stats(const char *group_name);


#endif // CGROUP_Hs