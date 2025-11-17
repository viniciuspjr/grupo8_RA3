# Arquitetura do Sistema Resource-Monitor

## 1. Visão Geral

O `resource-monitor` é uma ferramenta de linha de comando (CLI) interativa, baseada em menus, escrita em C (padrão C17). Ela é projetada para analisar, monitorar e controlar os recursos de processos no sistema operacional Linux.

A arquitetura é modular, com cada componente principal (Profiler, Analyzer, Manager) expondo uma API pública clara através de seus arquivos de cabeçalho (`.h`) na pasta `include/`.

O ponto de entrada `src/main.c` atua como o "orquestrador" (Aluno 1). Ele apresenta uma interface de menu ao usuário e chama as funções dos módulos correspondentes com base na entrada do usuário.

## 2. Decisão de Design Crítica: cgroup v2 (Ubuntu 24.04)

Este projeto foi desenvolvido e testado no **Ubuntu 24.04 LTS**, que utiliza a hierarquia **cgroup v2** por padrão. Esta decisão dita o design do módulo `cgroup_manager`:

1.  **Hierarquia Unificada:** Não existem mais montagens separadas para `cpu`, `memory`, `blkio`. Todos os controladores residem em uma única árvore em `/sys/fs/cgroup`.
2.  **Ativação de Controlador:** Os controladores (`cpu`, `memory`, `io`) devem ser ativados **antes** da criação do grupo, escrevendo (ex: `+cpu +memory +io`) no arquivo `cgroup.subtree_control` do diretório pai.
3.  **Nomes de Arquivo V2:** O código usa os nomes de arquivo modernos do v2 (ex: `cpu.max`, `memory.max`, `cpu.stat`, `io.stat`).

## 3. Fontes de Dados do Kernel

O sistema interage diretamente com duas interfaces principais do kernel, sem bibliotecas externas:

* **/proc (Procfs):** Usado para *leitura* e *profiling*. É a fonte de dados para os módulos **Resource Profiler** (Aluno 1/2) e **Namespace Analyzer** (Aluno 3).
* **/sys/fs/cgroup (Cgroupfs):** Usado para *leitura* e *escrita*. É a fonte de dados para o **Control Group Manager** (Aluno 4) para aplicar limites e coletar métricas de *grupos*.

## 4. Componentes Principais e APIs

### 4.1. CLI (main.c)
* **Responsável:** Aluno 1 (Integração).
* **Função:** Ponto de entrada e interface do usuário (UI). Apresenta menus, coleta a entrada do usuário (`scanf`) e chama as funções de back-end apropriadas.
* **Estrutura:**
    * `main()`: Loop principal do menu.
    * `handle_profiler_menu()`: Lógica para o submenu do Profiler.
    * `handle_namespace_menu()`: Lógica para o submenu do Namespace.
    * `handle_cgroup_menu()`: Lógica para o submenu do Cgroup.

### 4.2. Resource Profiler (monitor.h)
* **Responsável:** Aluno 1 (CPU/Mem) e Aluno 2 (I/O/Rede).
* **Função:** Coletar métricas detalhadas de um processo específico (por PID).
* **API Exposta:**
    * `CpuMonitorState`, `MemorySample`, `IoSample` (Structs de dados).
    * `cpu_monitor_init` / `cpu_monitor_sample`: Coleta CPU%, threads, etc. (Fonte: `/proc/[pid]/stat`, `/proc/stat`).
    * `memory_monitor_sample`: Coleta RSS, VSZ, Page Faults. (Fonte: `/proc/[pid]/status`, `/proc/[pid]/statm`).
    * `io_monitor_init` / `io_monitor_sample`: Coleta I/O de disco e rede, e calcula taxas. (Fonte: `/proc/[pid]/io`, `/proc/[pid]/net/dev`).
    * `..._csv_write`: Funções auxiliares para exportar dados (conforme `main.c`).

### 4.3. Namespace Analyzer (namespace.h)
* **Responsável:** Aluno 3.
* **Função:** Analisar o isolamento de processos via namespaces.
* **API Exposta:**
    * `NamespaceInfo` (Struct de dados).
    * `list_process_namespaces(pid_t)`: Lista os namespaces de um processo.
    * `compare_namespaces(pid_t, pid_t)`: Compara os inodes de namespace de dois processos.
    * `list_namespace_members(...)`, `measure_namespace_overhead(...)`, `generate_namespace_report(...)`: Funções para experimentos e relatórios.
* **Lógica Principal:** A implementação deve usar `readlink` para ler os links simbólicos em `/proc/[pid]/ns/` (ex: `ns/pid`, `ns/net`) e comparar seus valores de inode.

### 4.4. Control Group Manager (cgroup.h)
* **Responsável:** Aluno 4.
* **Função:** Criar grupos, mover processos para grupos, aplicar limites e ler métricas de *grupos* inteiros.
* **API Exposta (`cgroup.h`):**
    * `cgroup_create(...)`: Cria o diretório do grupo (ex: `mkdir /sys/fs/cgroup/MEU_GRUPO`).
    * `cgroup_move_pid(...)`: Escreve o PID no arquivo `cgroup.procs`.
    * `cgroup_set_memory_limit(...)`: Escreve o limite em bytes no arquivo `memory.max`.
    * `cgroup_set_cpu_limit(...)`: Escreve a quota e o período no arquivo `cpu.max`.
    * `cgroup_get_memory_usage(...)`: Lê o uso atual de `memory.current`.
    * `cgroup_get_cpu_usage(...)`: Lê e "parseia" `usage_usec` de `cpu.stat`.
    * `cgroup_get_io_stats(...)`: Lê e "parseia" `rbytes` e `wbytes` de `io.stat`.

## 5. Fluxo de Dados (Exemplo de Integração)

O `main.c` demonstra perfeitamente a arquitetura.

**Exemplo: Monitorar a CPU de um processo (Alunos 1 e 2)**
1.  **[main.c]** O usuário vê `print_main_menu()` e digita `1`.
2.  **[main.c]** A `switch(opt)` chama `handle_profiler_menu()`.
3.  **[main.c]** O usuário vê `print_profiler_menu()` e digita `1`.
4.  **[main.c]** `handle_profiler_menu()` pede ao usuário um `PID` e `Duracao`.
5.  **[main.c]** Chama `cpu_monitor_init()` (API do Aluno 1/2).
6.  **[main.c]** Entra em um loop `for` (baseado na `Duracao`).
7.  Dentro do loop, `main.c` chama `cpu_monitor_sample()` (API do Aluno 1/2) e imprime os resultados da struct `CpuSample`.

**Exemplo: Limitar Memória (Alunos 1 e 4)**
1.  **[main.c]** O usuário vê `print_main_menu()` e digita `3`.
2.  **[main.c]** A `switch(opt)` chama `handle_cgroup_menu()`.
3.  **[main.c]** O usuário vê `print_cgroup_menu()` e digita `3`.
4.  **[main.c]** `handle_cgroup_menu()` pede ao usuário um `Grupo` e `Bytes`.
5.  **[main.c]** Chama `cgroup_set_memory_limit(g, b)` (API do Aluno 4).
6.  **[cgroup_manager.c]** (Aluno 4) Abre o arquivo `/sys/fs/cgroup/NOME_DO_GRUPO/memory.max` e escreve o valor em bytes.
7.  **[main.c]** Imprime "Limite definido!".