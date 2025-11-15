#include "monitor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Lê as estatísticas de I/O de disco a partir de /proc/<pid>/io
 * 
 * @param pid PID do processo a ser monitorado
 * @param read_bytes_out Ponteiro para armazenar bytes lidos
 * @param write_bytes_out Ponteiro para armazenar bytes escritos
 * @param io_syscalls_out Ponteiro para armazenar número de syscalls de I/O
 * @return 0 em sucesso, -1 em erro
 */
static int read_io_stats(pid_t pid,
                         unsigned long long *read_bytes_out,
                         unsigned long long *write_bytes_out,
                         unsigned long long *io_syscalls_out) {
    
    char path[64];
    
    // Monta o caminho do arquivo /proc/<pid>/io
    snprintf(path, sizeof(path), "/proc/%d/io", (int)pid);
    
    // Abre o arquivo /proc/<pid>/io para leitura
    FILE *fp = fopen(path, "r");
    
    if (!fp) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s (pode requerer permissoes root)\n", path);
        return -1;
    }
    
    // Buffer para ler linha por linha
    char line[256];
    unsigned long long read_bytes = 0;
    unsigned long long write_bytes = 0;
    unsigned long long syscr = 0;  // read syscalls
    unsigned long long syscw = 0;  // write syscalls
    
    // Percorre todas as linhas do arquivo
    while (fgets(line, sizeof(line), fp)) {
        // read_bytes: bytes lidos de dispositivos de armazenamento
        if (sscanf(line, "read_bytes: %llu", &read_bytes) == 1) {
            continue;
        }
        // write_bytes: bytes escritos em dispositivos de armazenamento
        if (sscanf(line, "write_bytes: %llu", &write_bytes) == 1) {
            continue;
        }
        // syscr: número de syscalls de leitura (read, pread, etc)
        if (sscanf(line, "syscr: %llu", &syscr) == 1) {
            continue;
        }
        // syscw: número de syscalls de escrita (write, pwrite, etc)
        if (sscanf(line, "syscw: %llu", &syscw) == 1) {
            continue;
        }
    }
    
    // Fecha o arquivo
    fclose(fp);
    
    // Escreve os valores coletados nas variáveis de saída
    *read_bytes_out = read_bytes;
    *write_bytes_out = write_bytes;
    *io_syscalls_out = syscr + syscw;  // total de syscalls de I/O
    
    return 0;
}

/**
 * Lê as estatísticas de rede a partir de /proc/net/dev
 * 
 * @param pid PID do processo (não usado, mas mantido para consistência)
 * @param rx_bytes_out Ponteiro para armazenar bytes recebidos
 * @param tx_bytes_out Ponteiro para armazenar bytes transmitidos
 * @param rx_packets_out Ponteiro para armazenar pacotes recebidos
 * @param tx_packets_out Ponteiro para armazenar pacotes transmitidos
 * @return 0 em sucesso, -1 em erro
 * 
 * Nota: /proc/net/dev mostra estatísticas globais do sistema, não por processo
 * Para monitoramento por processo seria necessário usar netlink ou eBPF
 */
static int read_net_stats(pid_t pid,
                          unsigned long long *rx_bytes_out,
                          unsigned long long *tx_bytes_out,
                          unsigned long long *rx_packets_out,
                          unsigned long long *tx_packets_out) {
    
    (void)pid;  // Marca o parâmetro como não utilizado
    
    // Abre o arquivo /proc/net/dev para leitura
    FILE *fp = fopen("/proc/net/dev", "r");
    
    if (!fp) {
        fprintf(stderr, "Aviso: nao foi possivel abrir /proc/net/dev\n");
        // Define valores como 0 se não conseguir ler
        *rx_bytes_out = 0;
        *tx_bytes_out = 0;
        *rx_packets_out = 0;
        *tx_packets_out = 0;
        return 0;  // Não é um erro crítico
    }
    
    // Buffer para ler linha por linha
    char line[512];
    unsigned long long total_rx_bytes = 0;
    unsigned long long total_tx_bytes = 0;
    unsigned long long total_rx_packets = 0;
    unsigned long long total_tx_packets = 0;
    
    // Pula as duas primeiras linhas (cabeçalho)
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    
    // Lê cada interface de rede
    while (fgets(line, sizeof(line), fp)) {
        char iface[32];
        unsigned long long rx_bytes, rx_packets, tx_bytes, tx_packets;
        
        // Formato: interface: rx_bytes rx_packets ... tx_bytes tx_packets ...
        int n = sscanf(line, "%31[^:]:%llu %llu %*u %*u %*u %*u %*u %*u %llu %llu",
                       iface, &rx_bytes, &rx_packets, &tx_bytes, &tx_packets);
        
        if (n == 5) {
            // Ignora a interface loopback
            if (strcmp(iface, "lo") != 0 && strncmp(iface, " lo", 3) != 0) {
                total_rx_bytes += rx_bytes;
                total_tx_bytes += tx_bytes;
                total_rx_packets += rx_packets;
                total_tx_packets += tx_packets;
            }
        }
    }
    
    // Fecha o arquivo
    fclose(fp);
    
    // Escreve os valores totais nas variáveis de saída
    *rx_bytes_out = total_rx_bytes;
    *tx_bytes_out = total_tx_bytes;
    *rx_packets_out = total_rx_packets;
    *tx_packets_out = total_tx_packets;
    
    return 0;
}

/**
 * Conta o número de conexões TCP ativas
 * 
 * @return Número de conexões TCP estabelecidas, ou 0 em caso de erro
 * 
 * Lê /proc/net/tcp e conta linhas com estado 01 (ESTABLISHED)
 */
static unsigned long long count_tcp_connections(void) {
    
    // Abre o arquivo /proc/net/tcp para leitura
    FILE *fp = fopen("/proc/net/tcp", "r");
    
    if (!fp) {
        fprintf(stderr, "Aviso: nao foi possivel abrir /proc/net/tcp\n");
        return 0;
    }
    
    // Buffer para ler linha por linha
    char line[512];
    unsigned long long count = 0;
    
    // Pula a primeira linha (cabeçalho)
    fgets(line, sizeof(line), fp);
    
    // Lê cada conexão
    while (fgets(line, sizeof(line), fp)) {
        unsigned int state;
        
        // Formato: sl local_address rem_address st tx_queue rx_queue ...
        // Estado 01 = ESTABLISHED
        if (sscanf(line, "%*d: %*x:%*x %*x:%*x %x", &state) == 1) {
            if (state == 0x01) {  // TCP_ESTABLISHED
                count++;
            }
        }
    }
    
    // Fecha o arquivo
    fclose(fp);
    
    return count;
}

/**
 * Inicializa o estado do monitor de I/O
 * 
 * @param state Ponteiro para estrutura de estado a ser inicializada
 * @param pid PID do processo a ser monitorado
 * @return 0 em sucesso, -1 em erro
 * 
 * Lê os valores iniciais de I/O que servirão de referência
 * para calcular taxas nas próximas amostras
 */
int io_monitor_init(IoMonitorState *state, pid_t pid) {
    
    // Verifica se o ponteiro passado é válido
    if (!state) {
        fprintf(stderr, "Erro: state nulo em io_monitor_init\n");
        return -1;
    }
    
    unsigned long long read_bytes = 0;
    unsigned long long write_bytes = 0;
    unsigned long long io_syscalls = 0;
    
    // Lê as estatísticas iniciais de I/O de disco
    if (read_io_stats(pid, &read_bytes, &write_bytes, &io_syscalls) < 0) {
        fprintf(stderr, "Erro em io_monitor_init: nao foi possivel ler I/O do processo %d\n", (int)pid);
        return -1;
    }
    
    // Armazena os valores iniciais na estrutura de estado
    state->pid = pid;
    state->last_read_bytes = read_bytes;
    state->last_write_bytes = write_bytes;
    state->last_disk_ops = io_syscalls;
    
    return 0;
}

/**
 * Coleta uma amostra de I/O do processo monitorado
 * 
 * @param state Ponteiro para estrutura de estado (mantém valores anteriores)
 * @param sample Ponteiro para estrutura que receberá os dados coletados
 * @param interval_sec Intervalo de tempo desde a última amostra (para calcular taxas)
 * @return 0 em sucesso, -1 em erro
 * 
 * Coleta métricas de I/O de disco e rede, calcula taxas por segundo
 */
int io_monitor_sample(IoMonitorState *state, IoSample *sample, double interval_sec) {
    
    // Verifica se os ponteiros recebidos são válidos
    if (!state || !sample) {
        fprintf(stderr, "Erro: ponteiro nulo em io_monitor_sample\n");
        return -1;
    }
    
    // Verifica se o intervalo é válido
    if (interval_sec <= 0.0) {
        fprintf(stderr, "Erro: intervalo invalido em io_monitor_sample\n");
        return -1;
    }
    
    unsigned long long read_bytes = 0;
    unsigned long long write_bytes = 0;
    unsigned long long io_syscalls = 0;
    
    unsigned long long rx_bytes = 0;
    unsigned long long tx_bytes = 0;
    unsigned long long rx_packets = 0;
    unsigned long long tx_packets = 0;
    
    // Lê as estatísticas atuais de I/O de disco
    if (read_io_stats(state->pid, &read_bytes, &write_bytes, &io_syscalls) < 0) {
        fprintf(stderr, "Erro em io_monitor_sample: nao foi possivel ler I/O do processo %d\n", 
                (int)state->pid);
        return -1;
    }
    
    // Lê as estatísticas de rede
    read_net_stats(state->pid, &rx_bytes, &tx_bytes, &rx_packets, &tx_packets);
    
    // Conta conexões TCP ativas
    unsigned long long connections = count_tcp_connections();
    
    // Calcula as diferenças desde a última amostra
    unsigned long long delta_read = 0;
    unsigned long long delta_write = 0;
    unsigned long long delta_ops = 0;
    
    // Só calcula deltas se os valores atuais forem maiores ou iguais aos anteriores
    if (read_bytes >= state->last_read_bytes) {
        delta_read = read_bytes - state->last_read_bytes;
    }
    
    if (write_bytes >= state->last_write_bytes) {
        delta_write = write_bytes - state->last_write_bytes;
    }
    
    if (io_syscalls >= state->last_disk_ops) {
        delta_ops = io_syscalls - state->last_disk_ops;
    }
    
    // Calcula as taxas por segundo
    double read_rate = (double)delta_read / interval_sec;
    double write_rate = (double)delta_write / interval_sec;
    double ops_rate = (double)delta_ops / interval_sec;
    
    // Preenche a estrutura de amostra com os dados coletados
    sample->pid = state->pid;
    sample->timestamp = time(NULL);
    
    // I/O de disco
    sample->read_bytes = read_bytes;
    sample->write_bytes = write_bytes;
    sample->io_syscalls = io_syscalls;
    sample->disk_ops = io_syscalls;  // Aproximação: usamos syscalls como operações
    sample->read_rate_bytes_per_sec = read_rate;
    sample->write_rate_bytes_per_sec = write_rate;
    sample->disk_ops_per_sec = ops_rate;
    
    // Rede
    sample->rx_bytes = rx_bytes;
    sample->tx_bytes = tx_bytes;
    sample->rx_packets = rx_packets;
    sample->tx_packets = tx_packets;
    sample->connections = connections;
    
    // Atualiza o estado para servir de referência na próxima amostragem
    state->last_read_bytes = read_bytes;
    state->last_write_bytes = write_bytes;
    state->last_disk_ops = io_syscalls;
    
    return 0;
}
