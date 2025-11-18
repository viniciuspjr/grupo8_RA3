 # Arquitetura do Sistema Resource-Monitor

## Visão Geral

Sistema modular em C (std=c17) para monitoramento de recursos Linux através de `/proc` e `/sys/fs/cgroup`.

**Componentes principais:**
- **Resource Profiler**: Monitora CPU, memória e I/O de processos
- **Namespace Analyzer**: Analisa isolamento via namespaces
- **Control Group Manager**: Gerencia limites de recursos via cgroups

## Estrutura do Projeto

```
resource-monitor/
├── README.md              # Documentação principal do projeto
├── Makefile               # Automação de compilação
├── docs/
│   └── ARCHITECTURE.md    # Arquitetura do sistema (este arquivo)
├── include/
│   ├── monitor.h          # Interface do Resource Profiler
│   ├── namespace.h        # Interface do Namespace Analyzer
│   └── cgroup.h           # Interface do Control Group Manager
├── src/
│   ├── cpu_monitor.c      # Coleta de métricas de CPU + CSV export
│   ├── memory_monitor.c   # Coleta de métricas de memória + CSV export
│   ├── io_monitor.c       # Coleta de métricas de I/O e rede + CSV export
│   ├── namespace_analyzer.c  # Análise de namespaces
│   ├── cgroup_manager.c   # Gerenciamento de cgroups
│   └── main.c             # Menu integrado principal
├── tests/
│   ├── test_cpu.c         # Teste do monitor de CPU
│   ├── test_memory.c      # Teste do monitor de memória
│   └── test_io.c          # Teste do monitor de I/O
└── scripts/
    ├── visualize.py       # Visualização de dados em gráficos
    ├── run_tests.sh       # Execução automatizada de testes
    ├── valgrind_test.sh   # Validação de memory leaks
    └── compare_tools.sh   # Comparação com ferramentas do sistema
```

## Fontes de Dados

- **`/proc/<pid>/*`**: Métricas de processos (CPU, memória, I/O, namespaces)
- **`/sys/fs/cgroup/`**: Control groups v2 para limites e estatísticas

## Componentes

### 4.2. Resource Profiler (monitor.h)
* **Responsável:** Felipe Bueno (CPU/Mem) e Vinícius Jordani (I/O/Rede).
* **Função:** Coletar métricas detalhadas de um processo específico (por PID).
* **API Exposta:**
    * `CpuMonitorState`, `MemorySample`, `IoSample` (Structs de dados).
    * `cpu_monitor_init` / `cpu_monitor_sample`: Coleta CPU%, threads, context switches. (Fonte: `/proc/[pid]/stat`, `/proc/stat`).
    * `memory_monitor_sample`: Coleta RSS, VSZ, Page Faults, Swap. (Fonte: `/proc/[pid]/status`, `/proc/[pid]/statm`).
    * `io_monitor_init` / `io_monitor_sample`: Coleta I/O de disco e rede, calcula taxas e operações/s. (Fonte: `/proc/[pid]/io`, `/proc/net/dev`, `/proc/net/tcp`).
    * `cpu_sample_csv_write` / `memory_sample_csv_write` / `io_sample_csv_write`: Exportação automática para CSV com timestamps formatados.
    * `cpu_sample_csv_close` / `memory_sample_csv_close` / `io_sample_csv_close`: Funções de cleanup para evitar memory leaks.

### 4.3. Namespace Analyzer (namespace.h)
* **Responsável:** Kevin Abe.
* **Função:** Analisar o isolamento de processos via namespaces.
* **API Exposta:**
    * `NamespaceInfo` (Struct de dados).
    * `list_process_namespaces(pid_t)`: Lista os namespaces de um processo.
    * `compare_namespaces(pid_t, pid_t)`: Compara os inodes de namespace de dois processos.
    * `list_namespace_members(...)`, `measure_namespace_overhead(...)`, `generate_namespace_report(...)`: Funções para experimentos e relatórios.
* **Lógica Principal:** A implementação deve usar `readlink` para ler os links simbólicos em `/proc/[pid]/ns/` (ex: `ns/pid`, `ns/net`) e comparar seus valores de inode.

### 4.4. Control Group Manager (cgroup.h)
* **Responsável:** João Guilherme.
* **Função:** Criar grupos, mover processos para grupos, aplicar limites e ler métricas de *grupos* inteiros.
* **API Exposta (`cgroup.h`):**
    * `cgroup_create(...)`: Cria o diretório do grupo (ex: `mkdir /sys/fs/cgroup/MEU_GRUPO`).
    * `cgroup_move_pid(...)`: Escreve o PID no arquivo `cgroup.procs`.
    * `cgroup_set_memory_limit(...)`: Escreve o limite em bytes no arquivo `memory.max`.
    * `cgroup_set_cpu_limit(...)`: Escreve a quota e o período no arquivo `cpu.max`.
    * `cgroup_get_memory_usage(...)`: Lê o uso atual de `memory.current`.
    * `cgroup_get_cpu_usage(...)`: Lê e "parseia" `usage_usec` de `cpu.stat`.
    * `cgroup_get_io_stats(...)`: Lê e "parseia" `rbytes` e `wbytes` de `io.stat`.

## 5. Fluxo de Dados

### Monitoramento de Recursos

```
main.c → Monitor API → /proc/<pid>/* → Display + CSV
```

1. Usuário informa PID e duração
2. `*_monitor_init()` lê estado inicial
3. Loop a cada 1s: `*_monitor_sample()` → coleta métricas → exibe + `*_csv_write()`
4. `*_csv_close()` finaliza arquivo

**Resultado:** Console com métricas em tempo real + CSV exportado

### Análise de Namespaces

```
main.c → readlink(/proc/<pid>/ns/*) → Comparação de inodes
```

- Lista namespaces de um processo
- Compara inodes entre processos (mesmo inode = namespace compartilhado)

### Cgroups

```
main.c → write(/sys/fs/cgroup/...) → Kernel aplica limite
```

1. `cgroup_create()` → cria diretório
2. `cgroup_move_pid()` → move processo para grupo
3. `cgroup_set_*_limit()` → escreve limite em arquivo de controle
4. `cgroup_get_*_usage()` → lê uso atual

### Visualização de CSV

```
CSV → visualize.py → pandas + matplotlib → Gráficos
```

**Uso:**
```bash
python3 scripts/visualize.py cpu-monitor-20251117_150909.csv
```

O script automaticamente:
- Detecta o tipo de monitor (CPU/Memory/I/O) pelas colunas
- Converte timestamps Unix para formato legível
- Gera gráficos multi-painel:
  - **CPU**: Usage %, User/System Time, Context Switches, Threads
  - **Memory**: RSS/VSZ, Page Faults, Swap, Estatísticas
  - **I/O**: Disk Rate, Ops/sec, Syscalls, Network Connections

### Testes Automatizados

```
run_tests.sh → Compilação → Execução → CSVs → Gráficos
```

**Uso:**
```bash
sudo ./scripts/run_tests.sh 
```

**O que acontece:**
1. Compila o projeto (`make clean && make tests`)
2. Cria processo de teste em background (`sleep 120 &`)
3. Executa sequencialmente: `test_cpu`, `test_memory`, `test_io`
4. Cada teste gera seu CSV com timestamp formatado
5. Abre visualizações automaticamente em paralelo

**Saída:**
```
========================================
  TESTES AUTOMATIZADOS - RESOURCE MONITOR
========================================

Compilando o projeto...
Iniciando processo de teste...
✓ PID de teste: 12345

════════════════════════════════════════
  TESTE 1: CPU Monitor (5 segundos)
════════════════════════════════════════
✅ CSV gerado: cpu-monitor-20251117_150909.csv
   Gerando gráfico...

[... Memory e I/O ...]

Os gráficos serão exibidos em janelas separadas.
```