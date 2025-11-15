#include <stdio.h>    
#include <time.h>     
#include <unistd.h>   // sleep
#include "monitor.h"  // CpuMonitorState, CpuSample, cpu_monitor_init, cpu_monitor_sample

int main(void) {
    pid_t pid;          // PID do processo a ser monitorado
    int duration_sec;   // tempo total de monitoramento, em segundos

    printf("===== TESTE CPU MONITOR =====\n\n");

    printf("Digite o PID do processo: ");
    if (scanf("%d", &pid) != 1) {
        fprintf(stderr, "Erro ao ler PID.\n");
        return 1;
    }

    printf("Digite o tempo de monitoramento (segundos): ");
    if (scanf("%d", &duration_sec) != 1 || duration_sec <= 0) {
        fprintf(stderr, "Tempo invalido.\n");
        return 1;
    }

    // Estrutura de estado usada pelo monitor de CPU
    CpuMonitorState state;

    // Inicializa o monitor de CPU para o PID informado
    if (cpu_monitor_init(&state, pid) != 0) {
        perror("cpu_monitor_init");
        return 1;
    }

    // Mensagem informando início do monitoramento
    printf("\nMonitorando PID %d por %d segundo(s)...\n", (int)pid, duration_sec);

    // Loop principal de monitoramento: uma amostra por segundo
    for (int i = 0; i < duration_sec; i++) {
        CpuSample sample;  // struct que vai receber os dados desta amostra

        // Coleta os dados de CPU para o processo monitorado
        if (cpu_monitor_sample(&state, &sample) != 0) {
            perror("cpu_monitor_sample");
            return 1;
        }

        char buf[32];     // buffer para string de data/hora formatada
        struct tm *tm_info = localtime(&sample.timestamp);
        
        // Formata como "YYYY-MM-DD HH:MM:SS"
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);

        // Impressão formatada da amostra na tela
        printf("\n---------------------------------------------\n");
        printf(" PID          : %d\n", (int)sample.pid);
        printf(" Timestamp    : %s\n", buf);
        printf(" CPU Usage    : %6.2f %%\n", sample.cpu_percent);
        printf(" User time    : %10llu ticks\n",
               (unsigned long long)sample.user_time_ticks);
        printf(" System time  : %10llu ticks\n",
               (unsigned long long)sample.system_time_ticks);
        printf(" Ctx switches : %10llu\n",
               (unsigned long long)sample.context_switches);
        printf(" Threads      : %10llu\n",
               (unsigned long long)sample.threads);
        printf("---------------------------------------------\n");

        // Espera 1 segundo antes da próxima leitura
        sleep(1);
    }

    return 0;
}