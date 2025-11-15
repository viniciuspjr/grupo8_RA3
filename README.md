Resource Monitor — Namespaces, Cgroups e Profiling de Recursos

Este projeto implementa um sistema completo de monitoramento, análise e limitação de recursos no Linux, utilizando as interfaces do kernel:

- /proc
- /sys/fs/cgroup
- Namespaces (CLONE_NEW*)
- Control Groups (cgroup v1)
- Syscalls de baixo nível (stat, clone, sched…)

O objetivo é demonstrar como containers utilizam namespaces para isolamento e cgroups para controle e contabilização de recursos.

Estrutura do Projeto:

resource-monitor/
├── README.md
├── Makefile
├── docs/
│   └── ARCHITECTURE.md
├── include/
│   ├── monitor.h
│   ├── namespace.h
│   └── cgroup.h
├── src/
│   ├── cpu_monitor.c
│   ├── memory_monitor.c
│   ├── io_monitor.c
│   ├── namespace_analyzer.c
│   ├── cgroup_manager.c
│   └── main.c
└── tests/
    ├── test_cpu.c
    ├── test_memory.c
    └── test_io.c

Requisitos: 

- GCC
- Linux Kernel 5.x ou superior
- Cgroup v1 habilitado
- Permissões de root para operações envolvendo cgroups e monitoramento de I/O

Compilação:

```bash
# Compilar tudo (executável principal, testes e workloads)
make all

# Compilar apenas o executável principal
make resource-monitor

# Compilar apenas os testes
make tests

# Compilar apenas os workloads
make workloads

# Limpar arquivos compilados
make clean
```

Uso do Sistema Integrado:

O executável principal `resource-monitor` agora possui um menu interativo que integra
todos os três componentes do sistema:

```bash
# Executar o sistema com menu interativo
./resource-monitor

# O menu principal oferece 3 opções:
# 1. Resource Profiler - Monitorar CPU, Memória e I/O de processos
# 2. Namespace Analyzer - Analisar e comparar namespaces
# 3. Control Group Manager - Gerenciar cgroups e limites de recursos
```

Cada submenu oferece funcionalidades específicas de forma interativa e intuitiva.

1. Monitoramento de Recursos:

```bash
# Testar monitor de CPU (requer PID de um processo em execução)
./test_cpu

# Testar monitor de memória
./test_memory

# Testar monitor de I/O (requer sudo para acesso a /proc/<pid>/io)
sudo ./test_io
```

2. Programas de Workload (para testes):

```bash
# Workload intensivo de CPU (60 segundos padrão)
./workload_cpu [duracao_segundos]
# Exemplo: ./workload_cpu 30

# Workload intensivo de memória (aloca até 500 MB padrão)
./workload_memory [max_memoria_mb]
# Exemplo: ./workload_memory 200

# Workload intensivo de I/O (arquivo de 100 MB padrão)
./workload_io [tamanho_arquivo_mb]
# Exemplo: ./workload_io 50
```

3. Exemplo de uso completo:

```bash
# Terminal 1: Executar um workload
./workload_cpu 60

# Terminal 2: Monitorar o workload (use o PID exibido pelo workload)
./test_cpu
# Digite o PID quando solicitado
# Digite a duração do monitoramento (ex: 30 segundos)
```

Documentação Adicional:

- `docs/ARCHITECTURE.md` - Arquitetura do sistema
- `docs/ALUNO2_TESTES.md` - Documentação detalhada dos testes e workloads (Aluno 2)


Contribuição dos Alunos:

- Aluno 1 – Resource Profiler + Integração (Felipe Simionato Bueno)

Coleta de CPU e memória

Integração dos módulos

Makefile base

- Aluno 2 – Resource Profiler + Testes (Vinicius Pelissari Jordani)

Coleta de I/O (disco) e rede (estatísticas do sistema)

Implementação de `io_monitor.c` com funções:
  - `io_monitor_init()`: inicializa monitoramento de I/O
  - `io_monitor_sample()`: coleta métricas de I/O e rede

Criação de `test_io.c` para validação do monitor de I/O

Criação de workloads de teste:
  - `workload_cpu.c`: stress de CPU com operações matemáticas
  - `workload_memory.c`: alocação incremental de memória até limite configurável
  - `workload_io.c`: ciclos de escrita/leitura de arquivos grandes

Atualização do Makefile com targets para testes e workloads

Documentação de uso dos testes e workloads

- Aluno 3 – Namespace Analyzer (Kevin Mitsuo Lohmann Abe)

Implementação completa do módulo de namespaces

Listagem e comparação de namespaces

Identificação de membros por namespace

Medição de overhead via clone()

Geração de relatórios CSV

Execução e documentação dos experimentos de isolamento

- Aluno 4 – Control Groups (João Barowski)

Implementação do Cgroup Manager

Criação, configuração e leitura de cgroups

Experimentos de limitação de CPU e memória
