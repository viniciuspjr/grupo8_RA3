#include <stdio.h>    // printf, scanf, fprintf
#include <time.h>     // time_t, struct tm, localtime, strftime
#include <unistd.h>   // sleep
#include "monitor.h"  // MemorySample, memory_monitor_sample

int main(void) {
    pid_t pid;          // PID do processo a ser monitorado
    int duration_sec;   // tempo total de monitoramento, em segundos

    printf("===== TESTE MEMORY MONITOR =====\n\n");

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

    printf("\nMonitorando MEMORIA do PID %d por %d segundo(s)...\n", (int)pid, duration_sec);

    // Loop principal de monitoramento: uma amostra por segundo
    for (int i = 0; i < duration_sec; i++) {
        MemorySample sample;  // struct que vai receber os dados desta amostra

        // Coleta os dados de memoria para o processo monitorado
        if (memory_monitor_sample(pid, &sample) != 0) {
            fprintf(stderr, "Erro ao coletar memoria do processo %d.\n", (int)pid);
            return 1;
        }

        
        char buf[32];    // buffer para string de data/hora formatada
        struct tm *tm_info = localtime(&sample.timestamp);
        
        // Formata como "YYYY-MM-DD HH:MM:SS"
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);

        // Impressão formatada da amostra de memória na tela
        printf("\n---------------------------------------------\n");
        printf(" PID           : %d\n", (int)sample.pid);
        printf(" Timestamp     : %s\n", buf);
        printf(" RSS           : %10llu bytes\n",
               (unsigned long long)sample.rss_bytes);
        printf(" VSZ           : %10llu bytes\n",
               (unsigned long long)sample.vsize_bytes);
        printf(" Page faults   : %10llu\n",
               (unsigned long long)sample.page_faults);
        printf(" Swap          : %10llu bytes\n",
               (unsigned long long)sample.swap_bytes);
        printf("---------------------------------------------\n");

        // Espera 1 segundo antes da próxima leitura
        sleep(1);
    }

    return 0;
}