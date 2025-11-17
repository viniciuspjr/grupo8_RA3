#include "monitor.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int read_rss_vsz(pid_t pid,
                        unsigned long long *rss_bytes_out,
                        unsigned long long *vsize_bytes_out) {
    
    char path[64];

    // Monta o caminho do arquivo /proc/<pid>/statm
    snprintf(path, sizeof(path), "/proc/%d/statm", (int)pid);

    // Abre o arquivo /proc/<pid>/statm para leitura
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", path);
        return -1;
    }

    // Buffer para armazenar a linha lida de statm
    char buf[256];
    if (!fgets(buf, sizeof(buf), fp)) {
        fprintf(stderr, "Erro: nao foi possivel ler %s\n", path);
        fclose(fp);
        return -1;
    }
    // Já leu o que precisa, fecha o arquivo
    fclose(fp);

    unsigned long long size = 0;     // total de páginas alocadas
    unsigned long long resident = 0; // páginas residentes (RSS)

    // Lê size e resident, e descarta o resto
    int n = sscanf(
        buf, 
        "%llu %llu %*u %*u %*u %*u %*u",  // elementos ignorados
        &size, &resident);

    if (n < 2) {
        fprintf(stderr, "Erro: formato inesperado em %s: %s\n", path, buf);
        return -1;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        // fallback se sysconf falhar
        page_size = 4096; // Em quase todos os sistemas x86 é 4096 bytes (4 KB), mas a função pega o valor real da máquina
    }

    // escreve os valores em bytes nas variáveis originais fornecidas
    *vsize_bytes_out = size * (unsigned long long)page_size;
    *rss_bytes_out   = resident * (unsigned long long)page_size;

    return 0;
}

static int read_page_faults(pid_t pid, unsigned long long *faults_out) {
    
    char path[64];
    
    // Monta o caminho do arquivo /proc/<pid>/stat
    snprintf(path, sizeof(path), "/proc/%d/stat", (int)pid);

    // Abre o arquivo /proc/<pid>/stat para leitura
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", path);
        return -1;
    }

    // Buffer para receber a linha inteira de /proc/<pid>/stat
    char buf[4096];
    if (!fgets(buf, sizeof(buf), fp)) {
        fprintf(stderr, "Erro: nao foi possivel ler %s\n", path);
        fclose(fp);
        return -1;
    }
    // Já leu o que precisa, fecha o arquivo
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

    unsigned long long minflt = 0;  // minor page faults - page faults leves (sem I/O de disco)
    unsigned long long majflt = 0;  // major page faults - page faults "grandes" (precisam de I/O de disco)

    // Formato de /proc/[pid]/stat após o ')':
    // 1) state (char), 2) ppid, 3) pgrp, 4) session, 5) tty_nr, 6) tpgid
    // 7) flags, 8) minflt, 9) cminflt, 10) majflt
    int scanned = sscanf(
        p,
        " %*c %*d %*d %*d %*d %*d %*u "  // campos 1-7
        "%llu "                           // campo 8: minflt
        "%*u "                            // campo 9: cminflt
        "%llu",                           // campo 10: majflt
        &minflt, &majflt);

    if (scanned != 2) {
        fprintf(stderr, "Erro: nao foi possivel extrair page faults de %s\n", path);
        return -1;
    }

    // Soma minflt + majflt e escreve na variável original fornecida
    *faults_out = minflt + majflt;

    return 0;
}

static int read_swap_bytes(pid_t pid, unsigned long long *swap_bytes_out) {
    
    char path[64];
    
    // Monta o caminho do arquivo /proc/<pid>/status
    snprintf(path, sizeof(path), "/proc/%d/status", (int)pid);

    // Abre o arquivo /proc/<pid>/status para leitura
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, "Aviso: nao foi possivel abrir %s para ler VmSwap\n", path);
        *swap_bytes_out = 0;
        return 0; // trata como zero se não der pra ler
    }

    // Buffer para ler o arquivo linha por linha
    char line[256];
    unsigned long long swap_kb = 0;

    // Percorre todas as linhas procurando pela linha que começa com "VmSwap:"
    while (fgets(line, sizeof(line), fp)) {
        // Se encontrar "VmSwap: <valor> kB", lê o valor em kilobytes
        if (sscanf(line, "VmSwap: %llu kB", &swap_kb) == 1) {
            break; // achou, pode parar de ler
        }
    }
    // Fecha o arquivo depois de ler tudo
    fclose(fp);

    // Converte de kB para bytes (1 kB = 1024 bytes)
    // Escreve na variável original fornecida
    *swap_bytes_out = swap_kb * 1024;

    return 0;
}

int memory_monitor_sample(pid_t pid, MemorySample *sample) {
    
    // Garante que o ponteiro de saída é válido
    if (!sample) {
        fprintf(stderr, "Erro: ponteiro nulo em memory_monitor_sample\n");
        return -1;
    }

    // Variáveis temporárias para guardar os valores coletados
    unsigned long long rss_bytes = 0;
    unsigned long long vsize_bytes = 0;
    unsigned long long page_faults = 0;
    unsigned long long swap_bytes = 0;

    // Lê RSS e VSZ a partir de /proc/<pid>/statm
    if (read_rss_vsz(pid, &rss_bytes, &vsize_bytes) < 0) {
        fprintf(stderr, "Erro em memory_monitor_sample: falha ao ler RSS/VSZ do processo %d\n",
                (int)pid);
        return -1;
    }

    // Lê page faults a partir de /proc/<pid>/stat
    if (read_page_faults(pid, &page_faults) < 0) {
        fprintf(stderr, "Erro em memory_monitor_sample: falha ao ler page faults do processo %d\n",
                (int)pid);
        return -1;
    }

    // Lê uso de swap (VmSwap) a partir de /proc/<pid>/status
    if (read_swap_bytes(pid, &swap_bytes) < 0) {
         // Se der erro, o helper já mostra aviso e aqui usamos 0 como fallback
        swap_bytes = 0;
    }

    // Preenche a struct de amostra com os valores coletados
    sample->pid = pid;
    sample->timestamp = time(NULL);    // instante da coleta
    sample->rss_bytes = rss_bytes;     // memória ram ocupada em bytes
    sample->vsize_bytes = vsize_bytes; // tamanho virtual do processo em bytes
    sample->page_faults = page_faults; // número de page faults
    sample->swap_bytes = swap_bytes;   // uso de swap em bytes

    return 0;
}

static FILE *memory_csv_file = NULL;  // arquivo CSV para memória

int memory_sample_csv_write(const MemorySample *sample) {
    if (!sample) {
        fprintf(stderr, "Erro: ponteiro nulo em memory_sample_csv_write\n");
        return -1;
    }

    // cria o arquivo na primeira chamada
    if (!memory_csv_file) {
        // Formata o timestamp para o nome do arquivo (YYYYMMDD_HHMMSS)
        struct tm *tm_info = localtime(&sample->timestamp);
        char filename[64];
        snprintf(filename, sizeof(filename),
                 "memory-monitor-%04d%02d%02d_%02d%02d%02d.csv",
                 tm_info->tm_year + 1900,
                 tm_info->tm_mon + 1,
                 tm_info->tm_mday,
                 tm_info->tm_hour,
                 tm_info->tm_min,
                 tm_info->tm_sec);

        memory_csv_file = fopen(filename, "w");
        if (!memory_csv_file) {
            fprintf(stderr, "Erro: nao foi possivel criar %s\n", filename);
            return -1;
        }

        // Escreve o cabeçalho do CSV
        fprintf(memory_csv_file, "timestamp,pid,rss_bytes,vsize_bytes,page_faults,swap_bytes\n");
        fflush(memory_csv_file);
    }

    if (fprintf(memory_csv_file,
                "%lld,%d,%llu,%llu,%llu,%llu\n",
                (long long)sample->timestamp,
                (int)sample->pid,
                (unsigned long long)sample->rss_bytes,
                (unsigned long long)sample->vsize_bytes,
                (unsigned long long)sample->page_faults,
                (unsigned long long)sample->swap_bytes) < 0) {
        fprintf(stderr, "Erro: nao foi possivel escrever linha CSV\n");
        return -1;
    }

    fflush(memory_csv_file);
    return 0;
}

void memory_sample_csv_close(void) {
    if (memory_csv_file) {
        fclose(memory_csv_file);
        memory_csv_file = NULL;
    }
}