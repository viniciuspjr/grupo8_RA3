#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "monitor.h"
#include "namespace.h"
#include "cgroup.h"

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_main_menu(void) {
    printf("\n========================================\n");
    printf("   RESOURCE MONITOR - MENU PRINCIPAL   \n");
    printf("========================================\n");
    printf("  1. Resource Profiler (CPU, Memoria, I/O)\n");
    printf("  2. Namespace Analyzer\n");
    printf("  3. Control Group Manager\n");
    printf("  0. Sair\n");
    printf("\nEscolha uma opcao: ");
}

void print_profiler_menu(void) {
    printf("\n========================================\n");
    printf("        RESOURCE PROFILER - MENU        \n");
    printf("========================================\n");
    printf("  1. Monitorar CPU de um processo\n");
    printf("  2. Monitorar Memoria de um processo\n");
    printf("  3. Monitorar I/O de um processo\n");
    printf("  4. Monitorar TUDO (CPU + Memoria + I/O)\n");
    printf("  0. Voltar\n");
    printf("\nEscolha uma opcao: ");
}

void print_namespace_menu(void) {
    printf("\n========================================\n");
    printf("      NAMESPACE ANALYZER - MENU        \n");
    printf("========================================\n");
    printf("  1. Listar namespaces de um processo\n");
    printf("  2. Comparar namespaces entre dois processos\n");
    printf("  3. Listar processos em um namespace\n");
    printf("  4. Medir overhead de criacao\n");
    printf("  5. Gerar relatorio completo\n");
    printf("  0. Voltar\n");
    printf("\nEscolha uma opcao: ");
}

void print_cgroup_menu(void) {
    printf("\n========================================\n");
    printf("    CONTROL GROUP MANAGER - MENU       \n");
    printf("========================================\n");
    printf("  1. Criar cgroup\n");
    printf("  2. Mover processo para cgroup\n");
    printf("  3. Definir limite de memoria\n");
    printf("  4. Definir limite de CPU\n");
    printf("  5. Ler uso de memoria\n");
    printf("  6. Ler uso de CPU\n");
    printf("  7. Ler estatisticas de I/O\n");
    printf("  8. Teste de estresse\n");
    printf("  0. Voltar\n");
    printf("\nEscolha uma opcao: ");
}

int run_stress_test(const char *group_name) {
    pid_t self_pid = getpid();
    printf("Teste de estresse no grupo %s (PID: %d)\n", group_name, self_pid);

    cgroup_create("cpu", group_name);
    cgroup_create("memory", group_name);
    cgroup_move_pid("cpu", group_name, self_pid);
    cgroup_move_pid("memory", group_name, self_pid);
    cgroup_set_memory_limit(group_name, 100 * 1024 * 1024);
    cgroup_set_cpu_limit(group_name, 0.5, 100000);

    printf("Limites aplicados. Pressione Enter...\n");
    getchar();

    printf("Memoria: %lld bytes\n", cgroup_get_memory_usage(group_name));
    printf("CPU: %lld us\n", cgroup_get_cpu_usage(group_name));

    return 0;
}

void handle_profiler_menu(void) {
    int opt, pid, dur;
    
    while (1) {
        print_profiler_menu();
        if (scanf("%d", &opt) != 1) { clear_input_buffer(); continue; }
        clear_input_buffer();
        if (opt == 0) break;
        
        switch (opt) {
            case 1: // CPU
                printf("\nPID: "); scanf("%d", &pid);
                printf("Duracao (s): "); scanf("%d", &dur);
                clear_input_buffer();
                
                CpuMonitorState cs;
                if (cpu_monitor_init(&cs, pid) == 0) {
                    printf("\nMonitorando CPU...\n");
                    for (int i = 0; i < dur; i++) {
                        CpuSample smp;
                        sleep(1);
                        if (cpu_monitor_sample(&cs, &smp) == 0) {
                            printf("[%d/%d] CPU: %.2f%% | Threads: %llu\n", 
                                   i+1, dur, smp.cpu_percent, smp.threads);
                        }
                    }
                }
                break;
                
            case 2: // Memoria
                printf("\nPID: "); scanf("%d", &pid);
                printf("Duracao (s): "); scanf("%d", &dur);
                clear_input_buffer();
                
                printf("\nMonitorando Memoria...\n");
                for (int i = 0; i < dur; i++) {
                    MemorySample ms;
                    sleep(1);
                    if (memory_monitor_sample(pid, &ms) == 0) {
                        printf("[%d/%d] RSS: %.2f MB | VSZ: %.2f MB\n",
                               i+1, dur, 
                               ms.rss_bytes/(1024.0*1024.0),
                               ms.vsize_bytes/(1024.0*1024.0));
                    }
                }
                break;
                
            case 3: // I/O
                if (geteuid() != 0) printf("\nAVISO: Requer sudo\n");
                printf("\nPID: "); scanf("%d", &pid);
                printf("Duracao (s): "); scanf("%d", &dur);
                clear_input_buffer();
                
                IoMonitorState is;
                if (io_monitor_init(&is, pid) == 0) {
                    printf("\nMonitorando I/O...\n");
                    for (int i = 0; i < dur; i++) {
                        IoSample ios;
                        sleep(1);
                        if (io_monitor_sample(&is, &ios, 1.0) == 0) {
                            printf("[%d/%d] R: %.2f KB/s | W: %.2f KB/s\n",
                                   i+1, dur,
                                   ios.read_rate_bytes_per_sec/1024.0,
                                   ios.write_rate_bytes_per_sec/1024.0);
                        }
                    }
                }
                break;
                
            case 4: // Tudo
                if (geteuid() != 0) printf("\nAVISO: I/O requer sudo\n");
                printf("\nPID: "); scanf("%d", &pid);
                printf("Duracao (s): "); scanf("%d", &dur);
                clear_input_buffer();
                
                CpuMonitorState csa;
                IoMonitorState isa;
                cpu_monitor_init(&csa, pid);
                int io_ok = (io_monitor_init(&isa, pid) == 0);
                
                printf("\nMonitorando tudo...\n");
                for (int i = 0; i < dur; i++) {
                    CpuSample c; MemorySample m; IoSample io;
                    sleep(1);
                    cpu_monitor_sample(&csa, &c);
                    memory_monitor_sample(pid, &m);
                    if (io_ok) io_monitor_sample(&isa, &io, 1.0);
                    
                    printf("[%d/%d] CPU: %.1f%% | Mem: %.1f MB",
                           i+1, dur, c.cpu_percent, m.rss_bytes/(1024.0*1024.0));
                    if (io_ok) printf(" | I/O: %.1f KB/s", 
                                      (io.read_rate_bytes_per_sec+io.write_rate_bytes_per_sec)/1024.0);
                    printf("\n");
                }
                break;
        }
    }
}

void handle_namespace_menu(void) {
    int opt; pid_t p1, p2; char t[32], f[256]; long long in;
    
    while (1) {
        print_namespace_menu();
        if (scanf("%d", &opt) != 1) { clear_input_buffer(); continue; }
        clear_input_buffer();
        if (opt == 0) break;
        
        switch (opt) {
            case 1:
                printf("\nPID: "); scanf("%d", &p1); clear_input_buffer();
                list_process_namespaces(p1);
                break;
            case 2:
                printf("\nPID 1: "); scanf("%d", &p1);
                printf("PID 2: "); scanf("%d", &p2); clear_input_buffer();
                compare_namespaces(p1, p2);
                break;
            case 3:
                printf("\nTipo (pid/net/mnt): "); scanf("%31s", t);
                printf("Inode: "); scanf("%lld", &in); clear_input_buffer();
                list_namespace_members(t, in);
                break;
            case 4:
                measure_namespace_overhead();
                break;
            case 5:
                printf("\nArquivo: "); scanf("%255s", f); clear_input_buffer();
                generate_namespace_report(f);
                break;
        }
    }
}

void handle_cgroup_menu(void) {
    int opt; char c[64], g[256]; pid_t p; long long b; double co;
    
    while (1) {
        print_cgroup_menu();
        if (scanf("%d", &opt) != 1) { clear_input_buffer(); continue; }
        clear_input_buffer();
        if (opt == 0) break;
        
        if (geteuid() != 0 && opt <= 4) printf("\nAVISO: Requer sudo\n");
        
        switch (opt) {
            case 1:
                printf("\nControlador (cpu/memory): "); scanf("%63s", c);
                printf("Nome do grupo: "); scanf("%255s", g); clear_input_buffer();
                if (cgroup_create(c, g) == 0) printf("Criado!\n");
                break;
            case 2:
                printf("\nControlador: "); scanf("%63s", c);
                printf("Grupo: "); scanf("%255s", g);
                printf("PID: "); scanf("%d", &p); clear_input_buffer();
                if (cgroup_move_pid(c, g, p) == 0) printf("Movido!\n");
                break;
            case 3:
                printf("\nGrupo: "); scanf("%255s", g);
                printf("Bytes: "); scanf("%lld", &b); clear_input_buffer();
                if (cgroup_set_memory_limit(g, b) == 0) printf("Limite definido!\n");
                break;
            case 4:
                printf("\nGrupo: "); scanf("%255s", g);
                printf("Cores: "); scanf("%lf", &co); clear_input_buffer();
                if (cgroup_set_cpu_limit(g, co, 100000) == 0) printf("Limite definido!\n");
                break;
            case 5:
                printf("\nGrupo: "); scanf("%255s", g); clear_input_buffer();
                printf("Memoria: %lld bytes\n", cgroup_get_memory_usage(g));
                break;
            case 6:
                printf("\nGrupo: "); scanf("%255s", g); clear_input_buffer();
                printf("CPU: %lld us\n", cgroup_get_cpu_usage(g));
                break;
            case 7:
                printf("\nGrupo: "); scanf("%255s", g); clear_input_buffer();
                CgroupIOStats io = cgroup_get_io_stats(g);
                printf("I/O - R: %lld | W: %lld\n", io.rbytes, io.wbytes);
                break;
            case 8:
                printf("\nGrupo: "); scanf("%255s", g); clear_input_buffer();
                run_stress_test(g);
                break;
        }
    }
}

int main(void) {
    int opt;
    
    printf("\n================================================\n");
    printf("  RESOURCE MONITOR - SISTEMA INTEGRADO\n");
    printf("================================================\n");
    printf("Grupo 8: Felipe, Vinicius, Kevin, Joao\n");
    
    while (1) {
        print_main_menu();
        if (scanf("%d", &opt) != 1) { clear_input_buffer(); continue; }
        clear_input_buffer();
        
        switch (opt) {
            case 1: handle_profiler_menu(); break;
            case 2: handle_namespace_menu(); break;
            case 3: handle_cgroup_menu(); break;
            case 0:
                printf("\nSaindo... Ate logo!\n\n");
                return 0;
            default:
                printf("\nOpcao invalida!\n");
        }
    }
    
    return 0;
}
