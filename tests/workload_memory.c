/**
 * workload_memory.c - Programa intensivo em memória
 * 
 * Aloca e manipula grandes quantidades de memória
 * Útil para testar monitores de memória e limitação via cgroups
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MB (1024 * 1024)  // 1 Megabyte em bytes

/**
 * Função que aloca e escreve em memória
 * 
 * @param size_mb Tamanho em megabytes a alocar
 * @return Ponteiro para memória alocada, ou NULL em caso de erro
 */
void* allocate_and_use_memory(size_t size_mb) {
    size_t size_bytes = size_mb * MB;
    
    printf("Alocando %zu MB de memoria...\n", size_mb);
    
    // Aloca memória
    char *buffer = (char *)malloc(size_bytes);
    
    if (!buffer) {
        fprintf(stderr, "ERRO: Falha ao alocar %zu MB\n", size_mb);
        return NULL;
    }
    
    printf("Memoria alocada. Escrevendo dados...\n");
    
    // Escreve em toda a memória para forçar alocação física (RSS)
    // Sem isso, a memória seria apenas virtual (VSZ)
    memset(buffer, 'A', size_bytes);
    
    // Padrão de escrita adicional para garantir uso
    for (size_t i = 0; i < size_bytes; i += 4096) {
        buffer[i] = (char)(i % 256);
    }
    
    printf("Memoria em uso: %zu MB\n", size_mb);
    
    return buffer;
}

int main(int argc, char *argv[]) {
    printf("===== WORKLOAD MEMORIA INTENSIVO =====\n");
    printf("PID: %d\n\n", (int)getpid());
    
    // Parâmetros configuráveis
    size_t increment_mb = 50;     // Incremento de memória por rodada (50 MB)
    size_t max_memory_mb = 500;   // Máximo de memória a alocar (500 MB)
    int sleep_seconds = 2;        // Tempo de espera entre alocações
    
    // Permite configurar o máximo de memória por linha de comando
    if (argc > 1) {
        max_memory_mb = (size_t)atoi(argv[1]);
        if (max_memory_mb <= 0) {
            fprintf(stderr, "Uso: %s [max_memoria_mb]\n", argv[0]);
            return 1;
        }
    }
    
    printf("Incremento por rodada: %zu MB\n", increment_mb);
    printf("Maximo de memoria: %zu MB\n", max_memory_mb);
    printf("Pressione Ctrl+C para interromper.\n\n");
    
    // Array para armazenar ponteiros de memória alocada
    void **allocated_blocks = NULL;
    size_t num_blocks = 0;
    size_t current_total_mb = 0;
    
    // Calcula número máximo de blocos
    size_t max_blocks = (max_memory_mb + increment_mb - 1) / increment_mb;
    allocated_blocks = (void **)malloc(max_blocks * sizeof(void *));
    
    if (!allocated_blocks) {
        fprintf(stderr, "ERRO: Falha ao alocar array de ponteiros\n");
        return 1;
    }
    
    // Loop de alocação incremental
    while (current_total_mb < max_memory_mb) {
        size_t to_allocate = increment_mb;
        
        // Ajusta para não ultrapassar o máximo
        if (current_total_mb + to_allocate > max_memory_mb) {
            to_allocate = max_memory_mb - current_total_mb;
        }
        
        printf("\n----- Rodada %zu -----\n", num_blocks + 1);
        printf("Memoria total atual: %zu MB\n", current_total_mb);
        
        // Aloca e usa memória
        void *block = allocate_and_use_memory(to_allocate);
        
        if (!block) {
            fprintf(stderr, "AVISO: Nao foi possivel alocar mais memoria.\n");
            break;
        }
        
        // Armazena o ponteiro
        allocated_blocks[num_blocks] = block;
        num_blocks++;
        current_total_mb += to_allocate;
        
        printf("Memoria total alocada: %zu MB (%zu blocos)\n", 
               current_total_mb, num_blocks);
        
        // Aguarda antes da próxima alocação
        printf("Aguardando %d segundos...\n", sleep_seconds);
        sleep(sleep_seconds);
    }
    
    printf("\n===== ALOCACAO COMPLETA =====\n");
    printf("Memoria total alocada: %zu MB\n", current_total_mb);
    printf("Numero de blocos: %zu\n", num_blocks);
    printf("\nMantendo memoria alocada. Pressione Enter para liberar e finalizar...\n");
    getchar();
    
    // Libera toda a memória alocada
    printf("\nLiberando memoria...\n");
    for (size_t i = 0; i < num_blocks; i++) {
        free(allocated_blocks[i]);
    }
    free(allocated_blocks);
    
    printf("Memoria liberada. Workload finalizado.\n");
    
    return 0;
}
