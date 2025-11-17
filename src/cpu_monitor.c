#include "monitor.h"

#include <stdio.h>
#include <string.h>

static int read_total_ticks(unsigned long long *total_out) {
    
    FILE *fp = fopen("/proc/stat", "r");
    
    if (!fp) {
        fprintf(stderr, "Erro: nao foi possivel abrir /proc/stat\n");
        return -1;
    }

    char buf[512];
    if (!fgets(buf, sizeof(buf), fp)) {
        fprintf(stderr, "Erro: nao foi possivel ler a primeira linha de /proc/stat\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    unsigned long long fields[10] = {0};
    
    // Lê de buf a string "cpu" e até 10 números inteiros sem sinal (unsigned long long)
    // Formato esperado de buf:
    // cpu  user nice system idle iowait irq softirq steal guest guest_nice
    int n = sscanf(
        buf,
        "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",  // descarta primeira string "cpu"
        &fields[0],     // user
        &fields[1],     // nice
        &fields[2],     // system
        &fields[3],     // idle
        &fields[4],     // iowait
        &fields[5],     // irq
        &fields[6],     // softirq
        &fields[7],     // steal
        &fields[8],     // guest
        &fields[9]      // guest_nice
    );
    
    if (n < 5) {
        fprintf(stderr, "Erro: formato inesperado em /proc/stat: %s\n", buf);
        return -1;
    }

    unsigned long long total = 0;
    // soma todos os n campos lidos
    for (int i = 0; i < n; i++) {
        total += fields[i];
    }

    *total_out = total;  // escreve o total lido na variável original fornecida

    return 0;
}

static int read_process_times_and_threads(pid_t pid,
                                          unsigned long long *utime_out,
                                          unsigned long long *stime_out,
                                          unsigned long long *threads_out) {
    
    char path[64];

    // Monta o caminho do arquivo /proc/<pid>/stat
    snprintf(path, sizeof(path), "/proc/%d/stat", (int)pid);
    
    // Abre o arquivo /proc/<pid>/stat para leitura
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", path);
        return -1;
    }

    char buf[4096];

    // Lê a linha do arquivo /proc/<pid>/stat
    if (!fgets(buf, sizeof(buf), fp)) {
        fprintf(stderr, "Erro: nao foi possivel ler %s\n", path);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    // /proc/<pid>/stat tem o nome do processo entre parênteses: pid (nome) ...
    // Procura o último ')' na linha para achar o fim do nome do processo
    char *p = strrchr(buf, ')');
    if (!p) {
        fprintf(stderr, "Erro: formato inesperado em %s: %s\n", path, buf);
        return -1;
    }
    // Avança um caractere para ficar depois do ')'
    p++; 

    unsigned long long utime = 0;        // tempo em modo usuário em ticks
    unsigned long long stime = 0;        // tempo em modo sistema em ticks
    long threads = 0;                    // número de threads     

    // Formato de /proc/[pid]/stat após o ')':
    // 1) state (char)
    // 2) ppid (int), 3) pgrp (int), 4) session (int), 5) tty_nr (int)
    // 6) tpgid (int), 7) flags (uint), 8) minflt (ulong), 9) cminflt (ulong)
    // 10) majflt (ulong), 11) cmajflt (ulong)
    // 12) utime (ulong), 13) stime (ulong)  <-- queremos esses
    // 14) cutime (long), 15) cstime (long), 16) priority (long), 17) nice (long)
    // 18) num_threads (long)  <-- e esse
    int scanned = sscanf(
        p,
        " %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u "  // campos 1-11
        "%llu %llu "                                      // campos 12-13: utime, stime
        "%*d %*d %*d %*d "                               // campos 14-17
        "%ld",                                            // campo 18: num_threads
        &utime, &stime, &threads);


    if (scanned != 3) {
        fprintf(stderr, "Erro: nao foi possivel extrair utime/stime/threads de %s\n", path);
        return -1;
    }

    // escreve nas variáveis originais fornecidas
    *utime_out = utime;
    *stime_out = stime;
    *threads_out = (unsigned long long)threads;
    return 0;
}

static int read_context_switches(pid_t pid, unsigned long long *ctx_out) {
    
    char path[64];

    // Monta o caminho do arquivo /proc/<pid>/status
    snprintf(path, sizeof(path), "/proc/%d/status", (int)pid);

    // Abre o arquivo /proc/<pid>/status para leitura
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, "Aviso: nao foi possivel abrir %s para ler context switches\n", path);
        return -1;
    }

    // Buffer para armazenar cada linha lida do arquivo
    char line[256];
    unsigned long long voluntary = 0;
    unsigned long long nonvoluntary = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Se a linha tiver "voluntary_ctxt_switches: <valor>", guarda em voluntary
        if (sscanf(line, "voluntary_ctxt_switches: %llu", &voluntary) == 1) {
            continue; // vai para a próxima linha
        }
        // Se a linha tiver "nonvoluntary_ctxt_switches: <valor>", guarda em nonvoluntary
        if (sscanf(line, "nonvoluntary_ctxt_switches: %llu", &nonvoluntary) == 1) {
            continue; // vai para a próxima linha
        }
    }

    // Fecha o arquivo depois de ler tudo
    fclose(fp);

    // Soma trocas voluntárias + não voluntárias e escreve na variável original fornecida
    *ctx_out = voluntary + nonvoluntary;

    return 0;
}

int cpu_monitor_init(CpuMonitorState *state, pid_t pid) {
    
    // Verifica se o ponteiro passado é válido
    if (!state) {
        fprintf(stderr, "Erro: state nulo em cpu_monitor_init\n");
        return -1;
    }

    unsigned long long utime = 0, stime = 0, total_ticks = 0, threads = 0;

    // Lê tempos de CPU (user time, system time) e número de threads do processo
    if (read_process_times_and_threads(pid, &utime, &stime, &threads) < 0) {
        fprintf(stderr, "Erro em cpu_monitor_init: nao foi possivel ler tempos e threads do processo %d\n", (int)pid);
        return -1;
    }

    // Lê o total de ticks de CPU do sistema
    if (read_total_ticks(&total_ticks) < 0) {
        fprintf(stderr, "Erro em cpu_monitor_init: nao foi possivel ler ticks totais de CPU\n");
        return -1;
    }

    // Armazena nos atributos da estrutura os valores iniciais que servirão de referência
    state->pid = pid;
    state->last_user_time_ticks = utime;
    state->last_system_time_ticks = stime;
    state->last_total_ticks = total_ticks;

    return 0;
}

int cpu_monitor_sample(CpuMonitorState *state, CpuSample *sample) {
    
    // Verifica se os ponteiros recebidos são válidos
    if (!state || !sample) {
        fprintf(stderr, "Erro: ponteiro nulo em cpu_monitor_sample\n");
        return -1;
    }

    unsigned long long utime = 0, stime = 0, threads = 0;
    unsigned long long total_ticks = 0;
    unsigned long long ctx_switches = 0;

    // Lê os tempos de CPU (utime/stime) e número de threads do processo
    if (read_process_times_and_threads(state->pid, &utime, &stime, &threads) < 0) {
        fprintf(stderr, "Erro em cpu_monitor_sample: nao foi possivel ler tempos do processo %d\n",
                (int)state->pid);
        return -1;
    }

    // Lê o total de ticks de CPU do sistema
    if (read_total_ticks(&total_ticks) < 0) {
        fprintf(stderr, "Erro em cpu_monitor_sample: nao foi possivel ler ticks totais de CPU\n");
        return -1;
    }

    // Lê o total de trocas de contexto do processo
    if (read_context_switches(state->pid, &ctx_switches) < 0) {
        ctx_switches = 0; // só avisa antes, já avisado no helper
    }

    // Soma anterior de utime+stime do processo (guardada no estado)
    unsigned long long prev_proc = state->last_user_time_ticks + state->last_system_time_ticks;
    // Soma atual de utime+stime do processo
    unsigned long long curr_proc = utime + stime;

    unsigned long long delta_proc = 0;
    unsigned long long delta_total = 0;

    // só calcula a diferença se o valor atual for maior ou igual ao anterior
    if (curr_proc >= prev_proc) {
        delta_proc = curr_proc - prev_proc;
    }

    if (total_ticks >= state->last_total_ticks) {
        delta_total = total_ticks - state->last_total_ticks;
    }

    double cpu_percent = 0.0;

    // Calcula a porcentagem de uso de CPU do processo no intervalo
    if (delta_total > 0) {
        cpu_percent = 100.0 * (double)delta_proc / (double)delta_total;
    }

    // Preenche a struct de amostra com os dados coletados
    sample->pid = state->pid;                // PID do processo monitorado
    sample->timestamp = time(NULL);          // horário da coleta
    sample->cpu_percent = cpu_percent;       // uso de CPU em %
    sample->user_time_ticks = utime;         // utime acumulado em ticks
    sample->system_time_ticks = stime;       // stime acumulado em ticks
    sample->context_switches = ctx_switches; // total de trocas de contexto
    sample->threads = threads;               // número de threads atuais

    // Atualiza o estado para servir de referência na próxima amostragem
    state->last_user_time_ticks = utime;
    state->last_system_time_ticks = stime;
    state->last_total_ticks = total_ticks;

    return 0;
}

static FILE *cpu_csv_file = NULL;  // arquivo CSV para CPU

int cpu_sample_csv_write(const CpuSample *sample) {
    if (!sample) {
        fprintf(stderr, "Erro: ponteiro nulo em cpu_sample_csv_write\n");
        return -1;
    }

    // cria o arquivo na primeira chamada
    if (!cpu_csv_file) {

        // Formata o timestamp para o nome do arquivo (YYYYMMDD_HHMMSS)
        struct tm *tm_info = localtime(&sample->timestamp);
        char filename[64];
        snprintf(filename, sizeof(filename),
                 "cpu-monitor-%04d%02d%02d_%02d%02d%02d.csv",
                 tm_info->tm_year + 1900,
                 tm_info->tm_mon + 1,
                 tm_info->tm_mday,
                 tm_info->tm_hour,
                 tm_info->tm_min,
                 tm_info->tm_sec);

        cpu_csv_file = fopen(filename, "w");
        if (!cpu_csv_file) {
            fprintf(stderr, "Erro: nao foi possivel criar %s\n", filename);
            return -1;
        }

        // Escreve o cabeçalho do CSV
        fprintf(cpu_csv_file, "timestamp,pid,cpu_percent,user_time_ticks,system_time_ticks,context_switches,threads\n");
        fflush(cpu_csv_file);
    }

    if (fprintf(cpu_csv_file,
                "%lld,%d,%.2f,%llu,%llu,%llu,%llu\n",
                (long long)sample->timestamp,
                (int)sample->pid,
                sample->cpu_percent,
                (unsigned long long)sample->user_time_ticks,
                (unsigned long long)sample->system_time_ticks,
                (unsigned long long)sample->context_switches,
                (unsigned long long)sample->threads) < 0) {
        fprintf(stderr, "Erro: nao foi possivel escrever linha CSV\n");
        return -1;
    }

    fflush(cpu_csv_file);
    return 0;
}

void cpu_sample_csv_close(void) {
    if (cpu_csv_file) {
        fclose(cpu_csv_file);
        cpu_csv_file = NULL;
    }
}