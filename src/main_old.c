#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // getpid() e geteuid()

// Módulos existentes
#include "monitor.h"
#include "namespace.h"
#include "cgroup.h"

/**
 * Limpa o buffer de entrada
 */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * Exibe o menu principal do sistema
 */
void print_main_menu(void) {
    printf("\n");
    printf("========================================\n");
    printf("   RESOURCE MONITOR - MENU PRINCIPAL   \n");
    printf("========================================\n");
    printf("\n");
    printf("  1. Resource Profiler (CPU, Memoria, I/O)\n");
    printf("  2. Namespace Analyzer\n");
    printf("  3. Control Group Manager\n");
    printf("  0. Sair\n");
    printf("\n");
    printf("Escolha uma opcao: ");
}

/**
 * Exibe o menu do Resource Profiler
 */
void print_profiler_menu(void) {
    printf("\n");
    printf("========================================\n");
    printf("        RESOURCE PROFILER - MENU        \n");
    printf("========================================\n");
    printf("\n");
    printf("  1. Monitorar CPU de um processo\n");
    printf("  2. Monitorar Memoria de um processo\n");
    printf("  3. Monitorar I/O de um processo\n");
    printf("  4. Monitorar TUDO (CPU + Memoria + I/O)\n");
    printf("  0. Voltar ao menu principal\n");
    printf("\n");
    printf("Escolha uma opcao: ");
}

/**
 * Exibe o menu do Namespace Analyzer
 */
void print_namespace_menu(void) {
    printf("\n");
    printf("========================================\n");
    printf("      NAMESPACE ANALYZER - MENU        \n");
    printf("========================================\n");
    printf("\n");
    printf("  1. Listar namespaces de um processo\n");
    printf("  2. Comparar namespaces entre dois processos\n");
    printf("  3. Listar processos em um namespace\n");
    printf("  4. Medir overhead de criacao de namespaces\n");
    printf("  5. Gerar relatorio completo do sistema\n");
    printf("  0. Voltar ao menu principal\n");
    printf("\n");
    printf("Escolha uma opcao: ");
}

/**
 * Exibe o menu do Control Group Manager
 */
void print_cgroup_menu(void) {
    printf("\n");
    printf("========================================\n");
    printf("    CONTROL GROUP MANAGER - MENU       \n");
    printf("========================================\n");
    printf("\n");
    printf("  1. Criar cgroup\n");
    printf("  2. Mover processo para cgroup\n");
    printf("  3. Definir limite de memoria\n");
    printf("  4. Definir limite de CPU\n");
    printf("  5. Ler uso de memoria de um cgroup\n");
    printf("  6. Ler uso de CPU de um cgroup\n");
    printf("  7. Ler estatisticas de I/O de um cgroup\n");
    printf("  8. Executar teste de estresse\n");
    printf("  0. Voltar ao menu principal\n");
    printf("\n");
    printf("Escolha uma opcao: ");
}

/**
 * Submenu: Resource Profiler
 */
void handle_profiler_menu(void) {
    int option;
    pid_t pid;
    int duration;
    
    while (1) {
        print_profiler_menu();
        
        if (scanf("%d", &option) != 1) {
            clear_input_buffer();
            printf("Opcao invalida!\n");
            continue;
        }
        clear_input_buffer();
        
        if (option == 0) {
            break; // Volta ao menu principal
        }
        
        switch (option) {
            case 1: // Monitorar CPU
                printf("\nDigite o PID do processo: ");
                if (scanf("%d", &pid) != 1) {
                    clear_input_buffer();
                    printf("PID invalido!\n");
                    break;
                }
                
                printf("Digite o tempo de monitoramento (segundos): ");
                if (scanf("%d", &duration) != 1 || duration <= 0) {
                    clear_input_buffer();
                    printf("Tempo invalido!\n");
                    break;
                }
                clear_input_buffer();
                
                // Inicializa monitor de CPU
                CpuMonitorState cpu_state;
                if (cpu_monitor_init(&cpu_state, pid) != 0) {
                    printf("Erro ao inicializar monitor de CPU.\n");
                    break;
                }
                
                printf("\nMonitorando CPU do PID %d por %d segundo(s)...\n", pid, duration);
                
                // Loop de monitoramento
                for (int i = 0; i < duration; i++) {
                    CpuSample cpu_sample;
                    
                    sleep(1);
                    
                    if (cpu_monitor_sample(&cpu_state, &cpu_sample) != 0) {
                        printf("Erro ao coletar amostra de CPU.\n");
                        break;
                    }
                    
                    printf("\n[Amostra %d/%d]\n", i + 1, duration);
                    printf("  CPU%%         : %.2f%%\n", cpu_sample.cpu_percent);
                    printf("  User time    : %llu ticks\n", cpu_sample.user_time_ticks);
                    printf("  System time  : %llu ticks\n", cpu_sample.system_time_ticks);
                    printf("  Ctx switches : %llu\n", cpu_sample.context_switches);
                    printf("  Threads      : %llu\n", cpu_sample.threads);
                }
                
                printf("\nMonitoramento concluido!\n");
                break;
                
            case 2: // Monitorar Memoria
                printf("\nDigite o PID do processo: ");
                if (scanf("%d", &pid) != 1) {
                    clear_input_buffer();
                    printf("PID invalido!\n");
                    break;
                }
                
                printf("Digite o tempo de monitoramento (segundos): ");
                if (scanf("%d", &duration) != 1 || duration <= 0) {
                    clear_input_buffer();
                    printf("Tempo invalido!\n");
                    break;
                }
                clear_input_buffer();
                
                printf("\nMonitorando MEMORIA do PID %d por %d segundo(s)...\n", pid, duration);
                
                for (int i = 0; i < duration; i++) {
                    MemorySample mem_sample;
                    
                    sleep(1);
                    
                    if (memory_monitor_sample(pid, &mem_sample) != 0) {
                        printf("Erro ao coletar amostra de memoria.\n");
                        break;
                    }
                    
                    printf("\n[Amostra %d/%d]\n", i + 1, duration);
                    printf("  RSS          : %llu bytes (%.2f MB)\n", 
                           mem_sample.rss_bytes, 
                           mem_sample.rss_bytes / (1024.0 * 1024.0));
                    printf("  VSZ          : %llu bytes (%.2f MB)\n", 
                           mem_sample.vsize_bytes,
                           mem_sample.vsize_bytes / (1024.0 * 1024.0));
                    printf("  Page faults  : %llu\n", mem_sample.page_faults);
                    printf("  Swap         : %llu bytes\n", mem_sample.swap_bytes);
                }
                
                printf("\nMonitoramento concluido!\n");
                break;
                
            case 3: // Monitorar I/O
                if (geteuid() != 0) {
                    printf("\nAVISO: Monitoramento de I/O requer permissoes root.\n");
                    printf("Execute o programa com sudo para resultados completos.\n");
                }
                
                printf("\nDigite o PID do processo: ");
                if (scanf("%d", &pid) != 1) {
                    clear_input_buffer();
                    printf("PID invalido!\n");
                    break;
                }
                
                printf("Digite o tempo de monitoramento (segundos): ");
                if (scanf("%d", &duration) != 1 || duration <= 0) {
                    clear_input_buffer();
                    printf("Tempo invalido!\n");
                    break;
                }
                clear_input_buffer();
                
                // Inicializa monitor de I/O
                IoMonitorState io_state;
                if (io_monitor_init(&io_state, pid) != 0) {
                    printf("Erro ao inicializar monitor de I/O.\n");
                    printf("Dica: Execute com sudo.\n");
                    break;
                }
                
                printf("\nMonitorando I/O do PID %d por %d segundo(s)...\n", pid, duration);
                
                for (int i = 0; i < duration; i++) {
                    IoSample io_sample;
                    
                    sleep(1);
                    
                    if (io_monitor_sample(&io_state, &io_sample, 1.0) != 0) {
                        printf("Erro ao coletar amostra de I/O.\n");
                        break;
                    }
                    
                    printf("\n[Amostra %d/%d]\n", i + 1, duration);
                    printf("  === DISCO ===\n");
                    printf("  Read bytes   : %llu\n", io_sample.read_bytes);
                    printf("  Write bytes  : %llu\n", io_sample.write_bytes);
                    printf("  Read rate    : %.2f bytes/s\n", io_sample.read_rate_bytes_per_sec);
                    printf("  Write rate   : %.2f bytes/s\n", io_sample.write_rate_bytes_per_sec);
                    printf("  === REDE ===\n");
                    printf("  RX bytes     : %llu\n", io_sample.rx_bytes);
                    printf("  TX bytes     : %llu\n", io_sample.tx_bytes);
                    printf("  Connections  : %llu\n", io_sample.connections);
                }
                
                printf("\nMonitoramento concluido!\n");
                break;
                
            case 4: // Monitorar TUDO
                if (geteuid() != 0) {
                    printf("\nAVISO: Monitoramento de I/O requer permissoes root.\n");
                }
                
                printf("\nDigite o PID do processo: ");
                if (scanf("%d", &pid) != 1) {
                    clear_input_buffer();
                    printf("PID invalido!\n");
                    break;
                }
                
                printf("Digite o tempo de monitoramento (segundos): ");
                if (scanf("%d", &duration) != 1 || duration <= 0) {
                    clear_input_buffer();
                    printf("Tempo invalido!\n");
                    break;
                }
                clear_input_buffer();
                
                // Inicializa todos os monitores
                CpuMonitorState cpu_state_all;
                IoMonitorState io_state_all;
                
                if (cpu_monitor_init(&cpu_state_all, pid) != 0) {
                    printf("Erro ao inicializar monitor de CPU.\n");
                    break;
                }
                
                int io_ok = (io_monitor_init(&io_state_all, pid) == 0);
                if (!io_ok) {
                    printf("Aviso: Monitor de I/O nao disponivel (requer sudo).\n");
                }
                
                printf("\nMonitorando TODOS recursos do PID %d por %d segundo(s)...\n", pid, duration);
                
                for (int i = 0; i < duration; i++) {
                    CpuSample cpu_s;
                    MemorySample mem_s;
                    IoSample io_s;
                    
                    sleep(1);
                    
                    // Coleta todas as amostras
                    int cpu_err = cpu_monitor_sample(&cpu_state_all, &cpu_s);
                    int mem_err = memory_monitor_sample(pid, &mem_s);
                    int io_err = io_ok ? io_monitor_sample(&io_state_all, &io_s, 1.0) : -1;
                    
                    printf("\n========== Amostra %d/%d ==========\n", i + 1, duration);
                    
                    if (cpu_err == 0) {
                        printf("  [CPU]\n");
                        printf("    CPU%%         : %.2f%%\n", cpu_s.cpu_percent);
                        printf("    Threads      : %llu\n", cpu_s.threads);
                    }
                    
                    if (mem_err == 0) {
                        printf("  [MEMORIA]\n");
                        printf("    RSS          : %.2f MB\n", mem_s.rss_bytes / (1024.0 * 1024.0));
                        printf("    VSZ          : %.2f MB\n", mem_s.vsize_bytes / (1024.0 * 1024.0));
                    }
                    
                    if (io_err == 0) {
                        printf("  [I/O]\n");
                        printf("    Read rate    : %.2f KB/s\n", io_s.read_rate_bytes_per_sec / 1024.0);
                        printf("    Write rate   : %.2f KB/s\n", io_s.write_rate_bytes_per_sec / 1024.0);
                    }
                }
                
                printf("\nMonitoramento concluido!\n");
                break;
                
            default:
                printf("Opcao invalida!\n");
        }
    }
}
    pid_t self_pid = getpid();
    printf("Iniciando teste de estresse (PID: %d) no grupo: %s\n", self_pid, group_name);

    if (cgroup_create("cpu", group_name) != 0) return -1;
    if (cgroup_create("memory", group_name) != 0) return -1;

    if (cgroup_move_pid("cpu", group_name, self_pid) != 0) return -1;
    if (cgroup_move_pid("memory", group_name, self_pid) != 0) return -1;

    long long mem_limit = 100 * 1024 * 1024;
    if (cgroup_set_memory_limit(group_name, mem_limit) != 0) return -1;

    double cpu_limit = 0.5;
    if (cgroup_set_cpu_limit(group_name, cpu_limit, 100000) != 0) return -1;

    printf("Limites aplicados. Pressione Enter para continuar...\n");
    getchar();

    long long mem_usage = cgroup_get_memory_usage(group_name);
    long long cpu_usage = cgroup_get_cpu_usage(group_name);
    CgroupIOStats io_stats = cgroup_get_io_stats(group_name);

    printf("--- Métricas Finais ---\n");
    printf("Memória usada: %lld bytes\n", mem_usage);
    printf("CPU usada: %lld us\n", cpu_usage);
    if (io_stats.rbytes != -1) {
        printf("I/O Lidos: %lld\n", io_stats.rbytes);
        printf("I/O Escritos: %lld\n", io_stats.wbytes);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    // Aviso sobre sudo (somente para cgroups)
    if (geteuid() != 0 &&
        (strcmp(command, "create") == 0 ||
         strcmp(command, "move") == 0 ||
         strcmp(command, "set_mem") == 0 ||
         strcmp(command, "set_cpu") == 0 ||
         strcmp(command, "stress_test") == 0)) {

        fprintf(stderr, "AVISO: Este comando deve ser executado com sudo.\n");
    }

    // ==============================
    // 🔵 COMANDOS DO ALUNO 3 (NAMESPACES)
    // ==============================

    if (strcmp(command, "ns_list") == 0 && argc == 3) {
        list_process_namespaces(atoi(argv[2]));
    }

    else if (strcmp(command, "ns_compare") == 0 && argc == 4) {
        compare_namespaces(atoi(argv[2]), atoi(argv[3]));
    }

    else if (strcmp(command, "ns_members") == 0 && argc == 4) {
        list_namespace_members(argv[2], atoll(argv[3]));
    }

    else if (strcmp(command, "ns_overhead") == 0) {
        measure_namespace_overhead();
    }

    else if (strcmp(command, "ns_report") == 0 && argc == 3) {
        generate_namespace_report(argv[2]);
    }

    // ==============================
    // 🔵 COMANDOS DO ALUNO 4 (CGROUPS)
    // ==============================

    else if (strcmp(command, "create") == 0 && argc == 4) {
        if (cgroup_create(argv[2], argv[3]) != 0)
            fprintf(stderr, "Falha ao criar cgroup.\n");
    }

    else if (strcmp(command, "move") == 0 && argc == 5) {
        pid_t pid = atoi(argv[4]);
        if (cgroup_move_pid(argv[2], argv[3], pid) != 0)
            fprintf(stderr, "Falha ao mover PID.\n");
    }

    else if (strcmp(command, "set_mem") == 0 && argc == 4) {
        if (cgroup_set_memory_limit(argv[2], atoll(argv[3])) != 0)
            fprintf(stderr, "Falha ao definir limite de memória.\n");
    }

    else if (strcmp(command, "set_cpu") == 0 && argc == 4) {
        if (cgroup_set_cpu_limit(argv[2], atof(argv[3]), 100000) != 0)
            fprintf(stderr, "Falha ao definir limite de CPU.\n");
    }

    else if (strcmp(command, "get_mem") == 0 && argc == 3) {
        printf("Memória usada: %lld\n", cgroup_get_memory_usage(argv[2]));
    }

    else if (strcmp(command, "get_cpu") == 0 && argc == 3) {
        printf("CPU usada: %lld us\n", cgroup_get_cpu_usage(argv[2]));
    }

    else if (strcmp(command, "get_io") == 0 && argc == 3) {
        CgroupIOStats io = cgroup_get_io_stats(argv[2]);
        printf("Lidos: %lld bytes\nEscritos: %lld bytes\n", io.rbytes, io.wbytes);
    }

    else if (strcmp(command, "stress_test") == 0 && argc == 3) {
        run_stress_test(argv[2]);
    }

    else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}