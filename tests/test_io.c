#include <stdio.h>    // printf, scanf, fprintf
#include <time.h>     // time_t, struct tm, localtime, strftime
#include <unistd.h>   // sleep
#include "monitor.h"  // IoMonitorState, IoSample, io_monitor_init, io_monitor_sample

int main(void) {
    pid_t pid;          // PID do processo a ser monitorado
    int duration_sec;   // tempo total de monitoramento, em segundos

    printf("===== TESTE I/O MONITOR =====\n\n");
    printf("NOTA: Monitoramento de I/O requer permissoes de root.\n");
    printf("      Execute com sudo para resultados completos.\n\n");

    // Lê o PID que o usuário deseja monitorar
    printf("Digite o PID do processo: ");
    if (scanf("%d", &pid) != 1) {
        fprintf(stderr, "Erro ao ler PID.\n");
        return 1;
    }

    // Lê por quanto tempo o monitoramento deve ser feito
    printf("Digite o tempo de monitoramento (segundos): ");
    if (scanf("%d", &duration_sec) != 1 || duration_sec <= 0) {
        fprintf(stderr, "Tempo invalido.\n");
        return 1;
    }

    // Estrutura de estado usada pelo monitor de I/O
    IoMonitorState state;

    // Inicializa o monitor de I/O para o PID informado
    if (io_monitor_init(&state, pid) != 0) {
        fprintf(stderr, "Erro ao inicializar monitor de I/O.\n");
        fprintf(stderr, "Dica: Execute com sudo se estiver monitorando outro processo.\n");
        return 1;
    }

    // Mensagem informando início do monitoramento
    printf("\nMonitorando I/O do PID %d por %d segundo(s)...\n", (int)pid, duration_sec);

    // Loop principal de monitoramento: uma amostra por segundo
    for (int i = 0; i < duration_sec; i++) {
        IoSample sample;  // struct que vai receber os dados desta amostra

        // Espera 1 segundo antes da próxima leitura
        sleep(1);

        // Coleta os dados de I/O para o processo monitorado
        // Intervalo de 1.0 segundo para calcular taxas
        if (io_monitor_sample(&state, &sample, 1.0) != 0) {
            fprintf(stderr, "Erro ao coletar I/O do processo %d.\n", (int)pid);
            return 1;
        }

        char buf[32];     // buffer para string de data/hora formatada
        struct tm *tm_info = localtime(&sample.timestamp);
        
        // Formata como "YYYY-MM-DD HH:MM:SS"
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);

        // Impressão formatada da amostra de I/O na tela
        printf("\n=============================================\n");
        printf(" PID           : %d\n", (int)sample.pid);
        printf(" Timestamp     : %s\n", buf);
        printf("---------------------------------------------\n");
        printf(" === DISCO ===\n");
        printf(" Read bytes    : %10llu bytes\n",
               (unsigned long long)sample.read_bytes);
        printf(" Write bytes   : %10llu bytes\n",
               (unsigned long long)sample.write_bytes);
        printf(" I/O syscalls  : %10llu\n",
               (unsigned long long)sample.io_syscalls);
        printf(" Disk ops      : %10llu\n",
               (unsigned long long)sample.disk_ops);
        printf(" Read rate     : %10.2f bytes/s\n",
               sample.read_rate_bytes_per_sec);
        printf(" Write rate    : %10.2f bytes/s\n",
               sample.write_rate_bytes_per_sec);
        printf(" Disk ops rate : %10.2f ops/s\n",
               sample.disk_ops_per_sec);
        printf("---------------------------------------------\n");
        printf(" === REDE ===\n");
        printf(" RX bytes      : %10llu bytes\n",
               (unsigned long long)sample.rx_bytes);
        printf(" TX bytes      : %10llu bytes\n",
               (unsigned long long)sample.tx_bytes);
        printf(" RX packets    : %10llu\n",
               (unsigned long long)sample.rx_packets);
        printf(" TX packets    : %10llu\n",
               (unsigned long long)sample.tx_packets);
        printf(" Connections   : %10llu\n",
               (unsigned long long)sample.connections);
        printf("=============================================\n");
    }

    return 0;
}
