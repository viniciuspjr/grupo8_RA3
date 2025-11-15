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

## Compilação:

**Observação:** Este projeto deve ser compilado e executado em ambiente Linux (use WSL no Windows).

```bash
# Entrar no WSL (no Windows)
wsl

# Navegar até a pasta do projeto
cd /mnt/c/Users/[seu_usuario]/VSCodeProjects/grupo8_RA3

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

## Como Usar o Programa:

### **Método Recomendado: Menu Interativo com 2 Terminais**

O programa principal requer **dois terminais WSL** para funcionar corretamente:

#### **Terminal 1 - Criar processo para monitorar:**

```bash
wsl
cd /mnt/c/Users/[seu_usuario]/VSCodeProjects/grupo8_RA3

# Escolha um workload para criar um processo de teste:

# Opção A: Workload de CPU (uso intensivo de processador)
./workload_cpu 300

# Opção B: Workload de Memória (aloca memória progressivamente)
./workload_memory 500

# Opção C: Workload de I/O (escreve/lê arquivos)
./workload_io 100
```

**⚠️ IMPORTANTE:** Anote o **PID** que aparece na tela (ex: `PID do processo: 1234`)

#### **Terminal 2 - Executar o menu de monitoramento:**

```bash
wsl
cd /mnt/c/Users/[seu_usuario]/VSCodeProjects/grupo8_RA3

# Executar com privilégios de root (necessário para I/O e cgroups)
sudo ./resource-monitor
```

O menu principal oferece 3 componentes:
- **1. Resource Profiler** - Monitorar CPU, Memória e I/O de processos
- **2. Namespace Analyzer** - Analisar e comparar namespaces
- **3. Control Group Manager** - Gerenciar cgroups e limites de recursos

**Exemplo de uso no menu:**
```
Escolha uma opcao: 1         (Resource Profiler)
Escolha uma opcao: 1         (Monitorar CPU)
PID: 1234                    (PID anotado do Terminal 1)
Duracao (s): 10              (Monitorar por 10 segundos)
```

### **Método Alternativo: Testes Individuais**

Você também pode usar os programas de teste individuais sem o menu:

### **Método Alternativo: Testes Individuais**

Você também pode usar os programas de teste individuais sem o menu:

```bash
# Testar monitor de CPU (requer PID de um processo em execução)
./test_cpu

# Testar monitor de memória
./test_memory

# Testar monitor de I/O (requer sudo para acesso a /proc/<pid>/io)
sudo ./test_io
```

## Programas de Workload (para testes):

Os workloads criam processos que podem ser monitorados:

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

## Documentação Adicional:

- `docs/ARCHITECTURE.md` - Arquitetura do sistema
- `docs/TESTES.md` - Documentação detalhada dos testes e workloads

## Requisitos do Sistema:

- **GCC** (compilador C)
- **Linux Kernel 5.x ou superior**
- **Cgroup v1** habilitado
- **Permissões de root** (sudo) para:
  - Monitoramento de I/O (`/proc/<pid>/io`)
  - Operações com cgroups (`/sys/fs/cgroup`)

## Contribuição dos Alunos:

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
