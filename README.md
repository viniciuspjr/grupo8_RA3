# Resource Monitor — Sistema de Profiling e Análise de Recursos

## Descrição do Projeto

Este projeto implementa um sistema completo de monitoramento, análise e limitação de recursos no Linux, explorando as primitivas do kernel que tornam a containerização possível. O sistema é composto por três componentes principais que permitem profiling detalhado de processos, análise de namespaces e gerenciamento de control groups (cgroups).

### Objetivos

- Monitorar recursos de processos (CPU, Memória, I/O e Rede)
- Analisar isolamento via namespaces
- Manipular e monitorar control groups
- Compreender os mecanismos fundamentais de containers

### Componentes

#### 1. Resource Profiler
Ferramenta que coleta e reporta métricas detalhadas de processos:
- **CPU**: user time, system time, context switches, threads
- **Memória**: RSS, VSZ, page faults, swap
- **I/O**: bytes read/write, syscalls de I/O
- **Rede**: bytes rx/tx, packets, conexões TCP

#### 2. Namespace Analyzer
Ferramenta que analisa e reporta isolamento via namespaces:
- Identificar todos os namespaces de um processo
- Mapear processos por namespace
- Comparar namespaces entre processos
- Medir overhead de criação de namespaces
- Gerar relatórios de isolamento em CSV

#### 3. Control Group Manager
Ferramenta que analisa e manipula control groups:
- Ler métricas de controladores (CPU, Memory, BlkIO)
- Criar cgroups experimentais
- Aplicar limites de recursos
- Mover processos entre cgroups
- Gerar relatórios de utilização

## Requisitos e Dependências

### Sistema Operacional
- **Linux Kernel 5.x ou superior** (testado em Ubuntu 20.04/22.04 via WSL2)
- Suporte a **cgroup v1** habilitado

### Ferramentas de Desenvolvimento
- **GCC** (GNU Compiler Collection)
- **Make** (GNU Make)
- **Git** (controle de versão)

### Permissões
- **Root (sudo)** necessário para:
  - Leitura de `/proc/<pid>/io` (monitoramento de I/O)
  - Operações com `/sys/fs/cgroup` (cgroups)
  - Criação de namespaces

### Bibliotecas
- **libc** (bibliotecas padrão C)
- **libm** (biblioteca matemática) - já incluída

## Instruções de Compilação

### Ambiente Windows com WSL

```bash
# 1. Abrir terminal WSL
wsl

# 2. Navegar até o diretório do projeto
cd /mnt/c/Users/[seu_usuario]/caminho/para/grupo8_RA3

# 3. Compilar todos os componentes
make all

# 4. Limpar arquivos compilados (se necessário)
make clean
```

### Targets do Makefile

```bash
# Compilar tudo (executável principal + testes)
make all

# Compilar apenas o executável principal
make resource-monitor

# Compilar apenas os testes
make tests

# Limpar arquivos objeto e executáveis
make clean
```

### Verificação de Compilação

O projeto deve compilar sem warnings. Caso apareçam erros, verifique:
- Versão do GCC: `gcc --version` (recomendado >= 9.0)
- Sistema Linux: `uname -r` (kernel >= 5.0)

## Instruções de Uso

### Execução do Programa Principal

O sistema possui um **menu interativo integrado** que centraliza todos os componentes:

```bash
# Executar com privilégios de root (necessário para cgroups e I/O)
sudo ./resource-monitor
```

#### Menu Principal

```
========================================
   RESOURCE MONITOR - MENU PRINCIPAL
========================================
  1. Resource Profiler (CPU, Memoria, I/O)
  2. Namespace Analyzer
  3. Control Group Manager
  0. Sair

Escolha uma opcao:
```

### Exemplos de Uso

#### Exemplo 1: Monitorar CPU de um Processo

**Pré-requisito**: Ter um processo rodando para monitorar.

```bash
# Terminal 1: Criar um processo (exemplo: stress test)
stress --cpu 1 --timeout 300s
# Anote o PID exibido (ex: 1234)

# Terminal 2: Executar o monitor
sudo ./resource-monitor

# No menu:
Escolha: 1 (Resource Profiler)
Escolha: 1 (Monitorar CPU)
PID: 1234
Duracao (s): 10

# Saída esperada:
[1/10] CPU: 99.5% | Threads: 1
[2/10] CPU: 99.8% | Threads: 1
...
```

#### Exemplo 2: Listar Namespaces de um Processo

```bash
sudo ./resource-monitor

# No menu:
Escolha: 2 (Namespace Analyzer)
Escolha: 1 (Listar namespaces)
PID: 1234

# Saída esperada:
PID 1234 namespaces:
  pid  -> 4026531836
  net  -> 4026531905
  mnt  -> 4026531841
  uts  -> 4026531838
  ipc  -> 4026531839
  user -> 4026531837
```

#### Exemplo 3: Criar e Configurar Cgroup

```bash
sudo ./resource-monitor

# No menu:
Escolha: 3 (Control Group Manager)

# 1. Criar cgroup
Escolha: 1 (Criar cgroup)
Controlador: cpu
Nome do grupo: teste_cpu

# 2. Mover processo para cgroup
Escolha: 2 (Mover PID)
Controlador: cpu
Grupo: teste_cpu
PID: 1234

# 3. Definir limite de CPU (50%)
Escolha: 4 (Definir limite de CPU)
Grupo: teste_cpu
Cores: 0.5

# 4. Verificar uso
Escolha: 6 (Ver uso de CPU)
Grupo: teste_cpu
```

### Programas de Teste Individuais

Além do menu integrado, você pode executar testes individuais:

```bash
# Testar monitor de CPU
./test_cpu

# Testar monitor de memória
./test_memory

# Testar monitor de I/O (requer sudo)
sudo ./test_io
```

### Obtendo PID de Processos

```bash
# Listar todos os processos
ps aux

# Encontrar processo específico
ps aux | grep [nome_processo]

# Ver processos por uso de CPU
top
```

## Estrutura do Projeto

```
resource-monitor/
├── README.md              # Este arquivo
├── Makefile               # Automação de compilação
├── docs/
│   ├── ARCHITECTURE.md    # Arquitetura do sistema
│   └── TESTES.md          # Documentação dos testes
├── include/
│   ├── monitor.h          # Interface do Resource Profiler
│   ├── namespace.h        # Interface do Namespace Analyzer
│   └── cgroup.h           # Interface do Control Group Manager
├── src/
│   ├── cpu_monitor.c      # Coleta de métricas de CPU
│   ├── memory_monitor.c   # Coleta de métricas de memória
│   ├── io_monitor.c       # Coleta de métricas de I/O e rede
│   ├── namespace_analyzer.c  # Análise de namespaces
│   ├── cgroup_manager.c   # Gerenciamento de cgroups
│   └── main.c             # Menu integrado
├── tests/
│   ├── test_cpu.c         # Teste do monitor de CPU
│   ├── test_memory.c      # Teste do monitor de memória
│   └── test_io.c          # Teste do monitor de I/O
└── scripts/
    ├── visualize.py       # Visualização de dados (futuro)
    └── compare_tools.sh   # Comparação com ferramentas existentes (futuro)
```

## Autores e Contribuições

### Grupo 8 - Turma 04N

#### Aluno 1: Felipe Simionato Bueno
**Responsabilidade**: Resource Profiler + Integração

**Contribuições**:
- Implementação de `cpu_monitor.c` (coleta de CPU e threads)
- Implementação de `memory_monitor.c` (coleta de memória e page faults)
- Integração dos três componentes no menu principal (`main.c`)
- Criação do `Makefile` base
- Estruturação inicial do projeto

#### Aluno 2: Vinicius Pelissari Jordani
**Responsabilidade**: Resource Profiler + Testes

**Contribuições**:
- Implementação de `io_monitor.c` (coleta de I/O e rede)
  - Função `io_monitor_init()`: inicialização do monitoramento
  - Função `io_monitor_sample()`: coleta de métricas de I/O e rede
  - Contagem de conexões TCP ativas
- Criação de `test_io.c` para validação do monitor
- Atualização do Makefile com targets de testes
- Correção de bugs em `sscanf` (parsing de `/proc`)
- Documentação de uso e testes

#### Aluno 3: Kevin Mitsuo Lohmann Abe
**Responsabilidade**: Namespace Analyzer + Experimentos

**Contribuições**:
- Implementação completa de `namespace_analyzer.c`
  - Listagem de namespaces de processos
  - Comparação de namespaces entre processos
  - Identificação de membros por namespace
  - Medição de overhead de criação
- Geração de relatórios CSV
- Execução e documentação de experimentos de isolamento
- Análise de tipos de namespace (pid, net, mnt, uts, ipc, user)

#### Aluno 4: João Barowski
**Responsabilidade**: Control Group Manager + Análise

**Contribuições**:
- Implementação de `cgroup_manager.c`
  - Criação e configuração de cgroups
  - Leitura de métricas (CPU, Memory, BlkIO)
  - Aplicação de limites de recursos
  - Movimentação de processos entre cgroups
- Condução de experimentos de throttling
- Geração de relatórios de utilização vs limites
- Testes de precisão de limitação

## Documentação Adicional

- **docs/ARCHITECTURE.md**: Descrição detalhada da arquitetura do sistema
- **docs/TESTES.md**: Metodologia de testes e validação
- **include/*.h**: Documentação inline das interfaces (comentários doxygen)

## Interfaces do Kernel Utilizadas

- **`/proc/<pid>/stat`**: Estatísticas de processos (CPU, threads, etc.)
- **`/proc/<pid>/statm`**: Uso de memória (RSS, VSZ)
- **`/proc/<pid>/status`**: Informações detalhadas (swap, context switches)
- **`/proc/<pid>/io`**: Estatísticas de I/O (requer root)
- **`/proc/<pid>/ns/*`**: Namespaces de processos
- **`/proc/net/dev`**: Estatísticas de interfaces de rede
- **`/proc/net/tcp`**: Conexões TCP ativas
- **`/sys/fs/cgroup/*`**: Interfaces de control groups

## Troubleshooting

### Erro: "Permission denied" ao acessar /proc/<pid>/io
**Solução**: Execute com `sudo`

### Erro: "No such file or directory" ao acessar cgroup
**Solução**: Verifique se cgroup v1 está montado:
```bash
ls /sys/fs/cgroup/cpu
ls /sys/fs/cgroup/memory
```

### Erro: "Não foi possível ler tempos do processo"
**Solução**: Verifique se o processo ainda existe:
```bash
ps -p [PID]
```

### Programa não compila no Windows
**Solução**: Use WSL (Windows Subsystem for Linux). O programa requer APIs do Linux.

## Licença

Este projeto foi desenvolvido para fins acadêmicos como parte da disciplina de Sistemas Operacionais do Insper - Instituto de Ensino e Pesquisa.

## Referências

- Linux Programmer's Manual: `man proc`, `man cgroups`
- Kernel Documentation: `/Documentation/filesystems/proc.txt`
- Control Groups v1: `/Documentation/cgroup-v1/`
- Namespaces: `man 7 namespaces`
