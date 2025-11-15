/**
 * workload_io.c - Programa intensivo em I/O de disco
 * 
 * Realiza operações intensivas de leitura e escrita em disco
 * Útil para testar monitores de I/O e limitação via cgroups
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define KB (1024)       // 1 Kilobyte
#define MB (1024 * KB)  // 1 Megabyte

/**
 * Escreve dados em arquivo
 * 
 * @param filename Nome do arquivo
 * @param size_mb Tamanho em megabytes a escrever
 * @return 0 em sucesso, -1 em erro
 */
int write_file(const char *filename, size_t size_mb) {
    
    printf("Escrevendo %zu MB em %s...\n", size_mb, filename);
    
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "ERRO: Nao foi possivel criar arquivo %s\n", filename);
        return -1;
    }
    
    // Buffer de 1 MB para escrita
    size_t buffer_size = 1 * MB;
    char *buffer = (char *)malloc(buffer_size);
    
    if (!buffer) {
        fprintf(stderr, "ERRO: Falha ao alocar buffer de escrita\n");
        fclose(fp);
        return -1;
    }
    
    // Preenche buffer com dados
    memset(buffer, 'X', buffer_size);
    
    // Escreve em blocos de 1 MB
    size_t total_written = 0;
    for (size_t i = 0; i < size_mb; i++) {
        size_t written = fwrite(buffer, 1, buffer_size, fp);
        total_written += written;
        
        if (written != buffer_size) {
            fprintf(stderr, "AVISO: Escrita incompleta no bloco %zu\n", i);
        }
        
        // Força gravação no disco a cada 10 MB
        if ((i + 1) % 10 == 0) {
            fflush(fp);
        }
    }
    
    fflush(fp);
    fclose(fp);
    free(buffer);
    
    printf("Escrita completa: %zu MB\n", total_written / MB);
    
    return 0;
}

/**
 * Lê dados de arquivo
 * 
 * @param filename Nome do arquivo
 * @return 0 em sucesso, -1 em erro
 */
int read_file(const char *filename) {
    
    printf("Lendo arquivo %s...\n", filename);
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "ERRO: Nao foi possivel abrir arquivo %s\n", filename);
        return -1;
    }
    
    // Buffer de 1 MB para leitura
    size_t buffer_size = 1 * MB;
    char *buffer = (char *)malloc(buffer_size);
    
    if (!buffer) {
        fprintf(stderr, "ERRO: Falha ao alocar buffer de leitura\n");
        fclose(fp);
        return -1;
    }
    
    // Lê todo o arquivo
    size_t total_read = 0;
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, buffer_size, fp)) > 0) {
        total_read += bytes_read;
    }
    
    fclose(fp);
    free(buffer);
    
    printf("Leitura completa: %zu MB\n", total_read / MB);
    
    return 0;
}

int main(int argc, char *argv[]) {
    printf("===== WORKLOAD I/O INTENSIVO =====\n");
    printf("PID: %d\n\n", (int)getpid());
    
    // Parâmetros configuráveis
    size_t file_size_mb = 100;  // Tamanho do arquivo (100 MB padrão)
    int num_iterations = 5;     // Número de ciclos de escrita/leitura
    const char *test_file = "/tmp/io_workload_test.dat";
    
    // Permite configurar o tamanho do arquivo por linha de comando
    if (argc > 1) {
        file_size_mb = (size_t)atoi(argv[1]);
        if (file_size_mb <= 0) {
            fprintf(stderr, "Uso: %s [tamanho_arquivo_mb]\n", argv[0]);
            return 1;
        }
    }
    
    printf("Arquivo de teste: %s\n", test_file);
    printf("Tamanho do arquivo: %zu MB\n", file_size_mb);
    printf("Numero de iteracoes: %d\n", num_iterations);
    printf("Pressione Ctrl+C para interromper.\n\n");
    
    time_t start_time = time(NULL);
    unsigned long long total_bytes_written = 0;
    unsigned long long total_bytes_read = 0;
    
    // Loop principal: ciclos de escrita e leitura
    for (int i = 0; i < num_iterations; i++) {
        printf("\n===== ITERACAO %d/%d =====\n", i + 1, num_iterations);
        
        // Fase de escrita
        printf("\n--- Fase de Escrita ---\n");
        if (write_file(test_file, file_size_mb) == 0) {
            total_bytes_written += file_size_mb * MB;
        }
        
        // Sincroniza para garantir que dados foram gravados
        sync();
        
        // Pequena pausa
        sleep(1);
        
        // Fase de leitura
        printf("\n--- Fase de Leitura ---\n");
        if (read_file(test_file) == 0) {
            total_bytes_read += file_size_mb * MB;
        }
        
        printf("\nIteracao %d completa.\n", i + 1);
        
        // Pausa entre iterações
        if (i < num_iterations - 1) {
            sleep(2);
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("\n===== WORKLOAD FINALIZADO =====\n");
    printf("Tempo decorrido: %.0f segundos\n", elapsed);
    printf("Total escrito: %llu MB\n", total_bytes_written / MB);
    printf("Total lido: %llu MB\n", total_bytes_read / MB);
    printf("Taxa de escrita: %.2f MB/s\n", 
           (double)(total_bytes_written / MB) / elapsed);
    printf("Taxa de leitura: %.2f MB/s\n", 
           (double)(total_bytes_read / MB) / elapsed);
    
    // Remove arquivo de teste
    printf("\nRemovendo arquivo de teste...\n");
    if (remove(test_file) == 0) {
        printf("Arquivo removido.\n");
    } else {
        fprintf(stderr, "AVISO: Nao foi possivel remover %s\n", test_file);
    }
    
    return 0;
}
