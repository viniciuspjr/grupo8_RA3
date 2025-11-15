/**
 * workload_cpu.c - Programa intensivo em CPU
 * 
 * Executa operações matemáticas em loop para gerar carga de CPU
 * Útil para testar monitores de CPU e limitação via cgroups
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

/**
 * Função que realiza operações matemáticas intensivas
 * 
 * @param iterations Número de iterações a executar
 * @return Soma acumulada (para evitar otimização do compilador)
 */
double cpu_intensive_work(unsigned long long iterations) {
    double result = 0.0;
    
    for (unsigned long long i = 1; i <= iterations; i++) {
        // Operações matemáticas que consomem CPU
        result += sqrt((double)i);
        result += sin((double)i / 1000.0);
        result += cos((double)i / 1000.0);
        result += log((double)i + 1.0);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    printf("===== WORKLOAD CPU INTENSIVO =====\n");
    printf("PID: %d\n\n", (int)getpid());
    
    // Parâmetros configuráveis
    unsigned long long iterations_per_round = 10000000;  // 10 milhões por rodada
    int duration_sec = 60;  // duração padrão de 60 segundos
    
    // Permite configurar a duração por linha de comando
    if (argc > 1) {
        duration_sec = atoi(argv[1]);
        if (duration_sec <= 0) {
            fprintf(stderr, "Uso: %s [duracao_segundos]\n", argv[0]);
            return 1;
        }
    }
    
    printf("Duracao: %d segundos\n", duration_sec);
    printf("Iteracoes por rodada: %llu\n", iterations_per_round);
    printf("Pressione Ctrl+C para interromper.\n\n");
    
    time_t start_time = time(NULL);
    time_t current_time;
    unsigned long long total_iterations = 0;
    int round = 0;
    
    // Loop principal: executa trabalho intensivo de CPU
    do {
        round++;
        printf("[Rodada %d] Executando calculos intensivos...\n", round);
        
        // Executa trabalho intensivo de CPU
        double result = cpu_intensive_work(iterations_per_round);
        total_iterations += iterations_per_round;
        
        // Imprime resultado para evitar que o compilador otimize fora o cálculo
        printf("[Rodada %d] Resultado: %.6f | Total iteracoes: %llu\n", 
               round, result, total_iterations);
        
        current_time = time(NULL);
        
    } while (difftime(current_time, start_time) < duration_sec);
    
    printf("\n===== WORKLOAD FINALIZADO =====\n");
    printf("Tempo decorrido: %.0f segundos\n", difftime(current_time, start_time));
    printf("Total de iteracoes: %llu\n", total_iterations);
    printf("Iteracoes/segundo: %.0f\n", 
           (double)total_iterations / difftime(current_time, start_time));
    
    return 0;
}
