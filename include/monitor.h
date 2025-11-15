#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>     // FILE
#include <sys/types.h> // pid_t
#include <time.h>      // time_t

/* ====================== CPU SAMPLE ====================== */

typedef struct {
    pid_t pid;
    time_t timestamp;  // instante da coleta

    double cpu_percent;
    unsigned long long user_time_ticks;   // user time
    unsigned long long system_time_ticks; // system time
    unsigned long long context_switches;  // trocas de contexto
    unsigned long long threads;           // número de threads
} CpuSample;

typedef struct {
    pid_t pid;
    unsigned long long last_user_time_ticks;
    unsigned long long last_system_time_ticks;
    unsigned long long last_total_ticks;
} CpuMonitorState;

int cpu_monitor_init(CpuMonitorState *state, pid_t pid);
int cpu_monitor_sample(CpuMonitorState *state, CpuSample *sample);
int cpu_sample_write_csv(const CpuSample *sample, FILE *fp);

/* ==================== MEMORY SAMPLE ==================== */

typedef struct {
    pid_t pid;
    time_t timestamp;  // instante da coleta

    unsigned long long rss_bytes;    // RSS (memória RAM que o processo está ocupando naquele momento)
    unsigned long long vsize_bytes;  // VSZ (tamanho total do espaço de memória virtual do processo)
    unsigned long long page_faults;  // page faults
    unsigned long long swap_bytes;   // swap
} MemorySample;

int memory_monitor_sample(pid_t pid, MemorySample *sample);
int memory_sample_write_csv(const MemorySample *sample, FILE *fp);

/* ====================== I/O SAMPLE ====================== */

typedef struct {
    pid_t pid;
    time_t timestamp;  // instante da coleta

    /* I/O de disco */
    unsigned long long read_bytes;   // bytes lidos
    unsigned long long write_bytes;  // bytes escritos
    unsigned long long io_syscalls;  // chamadas de sistema de I/O
    unsigned long long disk_ops;     // operações de disco
    double read_rate_bytes_per_sec;  // taxa de leitura (bytes/s)
    double write_rate_bytes_per_sec; // taxa de escrita (bytes/s)
    double disk_ops_per_sec;         // taxa de operações de disco (ops/s)

    /* Rede */
    unsigned long long rx_bytes;     // bytes recebidos
    unsigned long long tx_bytes;     // bytes transmitidos
    unsigned long long rx_packets;   // pacotes recebidos
    unsigned long long tx_packets;   // pacotes transmitidos
    unsigned long long connections;  // conexões ativas
} IoSample;

typedef struct {
    pid_t pid;
    unsigned long long last_read_bytes;
    unsigned long long last_write_bytes;
    unsigned long long last_disk_ops;
} IoMonitorState;

int io_monitor_init(IoMonitorState *state, pid_t pid);
int io_monitor_sample(IoMonitorState *state, IoSample *sample, double interval_sec);

#endif