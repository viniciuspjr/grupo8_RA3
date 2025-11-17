# Resource Monitor — Sistema de Profiling e Análise de Recursos

## Descrição do Projeto

Este projeto implementa um sistema completo de monitoramento, análise e limitação de recursos no Linux, explorando as primitivas do kernel que tornam a containerização possível. O sistema é composto por três componentes principais integrados em um menu interativo.

### Objetivos

- Monitorar recursos de processos em tempo real (CPU, Memória, I/O e Rede)
- Analisar isolamento via namespaces do Linux
- Gerenciar e limitar recursos através de control groups (cgroups)
- Compreender os mecanismos fundamentais utilizados por containers

### Componentes Principais

#### 1. Resource Profiler
Coleta métricas detalhadas de processos através de `/proc`:
- **CPU**: tempo de usuário/sistema, context switches, threads, percentual de uso
- **Memória**: RSS, VSZ, page faults, swap
- **I/O**: bytes lidos/escritos, syscalls de I/O, operações de disco
- **Rede**: bytes rx/tx, pacotes, conexões TCP ativas
- **Exportação CSV**: Todas as métricas são salvas em arquivos CSV com timestamp formatado
- **Visualização**: Gráficos interativos de todas as métricas coletadas
- **Validação**: Sem memory leaks (validado com valgrind)

#### 2. Namespace Analyzer
Analisa isolamento de processos via namespaces:
- Lista todos os namespaces de um processo
- Compara namespaces entre processos
- Identifica processos membros de um namespace
- Mede overhead de criação de namespaces
- Gera relatórios em formato CSV

#### 3. Control Group Manager
Gerencia limitação de recursos via cgroups:
- Lê métricas de controladores (CPU, Memory, BlkIO)
- Cria e configura cgroups experimentais
- Move processos entre cgroups
- Aplica limites de CPU e memória
- Gera relatórios de utilização vs limites

## Estrutura do Projeto

```
resource-monitor/
├── README.md              # Este arquivo
├── Makefile               # Automação de compilação
├── docs/
│   └── ARCHITECTURE.md    # Arquitetura do sistema
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

## Requisitos e Dependências

### Sistema Operacional
- **Linux Kernel 5.x ou superior** (testado em Ubuntu 20.04/22.04)
- **WSL2** (Windows Subsystem for Linux) para desenvolvimento em Windows
- Suporte a **cgroup v1** habilitado no sistema

### Ferramentas de Desenvolvimento
- **GCC** (GNU Compiler Collection) versão 9.0 ou superior
- **Make** (GNU Make) para automação da compilação
- **Git** para controle de versão

### Permissões Necessárias
O programa requer privilégios de **root (sudo)** para:
- Leitura de `/proc/<pid>/io` (monitoramento de I/O)
- Operações em `/sys/fs/cgroup` (gerenciamento de cgroups)
- Criação e manipulação de namespaces

### Bibliotecas
- **libc** (bibliotecas padrão C) - incluída no sistema
- **libm** (biblioteca matemática) - incluída no sistema

### Ferramentas Opcionais (para visualização e testes)
- **Python 3.x** com pandas e matplotlib para visualização de gráficos
- **Valgrind** para validação de memory leaks

```bash
# Instalar dependências Python (opcional)
sudo apt install python3-pandas python3-matplotlib

# Instalar valgrind (opcional)
sudo apt install valgrind
```

## Instruções de Compilação

### Passo 1: Acessar o Ambiente Linux

Se estiver no Windows, abra o terminal WSL:

```bash
wsl
```

### Passo 2: Navegar até o Diretório do Projeto

```bash
cd /mnt/c/Users/[seu_usuario]/caminho/para/grupo8_RA3
```

### Passo 3: Compilar o Projeto

```bash
# Compilar tudo (executável principal + testes)
make all

# OU compilar apenas o executável principal
make resource-monitor

# OU compilar apenas os testes
make tests
```

### Passo 4: Verificar a Compilação

O projeto deve compilar **sem warnings**. Se aparecerem erros:

```bash
# Verificar versão do GCC (deve ser >= 9.0)
gcc --version

# Verificar kernel Linux (deve ser >= 5.0)
uname -r
```

### Limpar Arquivos Compilados

```bash
make clean
```

## Instruções de Uso

### Execução do Programa Principal

O sistema possui um **menu interativo integrado**:

```bash
# Executar com privilégios de root
sudo ./resource-monitor
```

### Menu Principal

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

**Pré-requisito**: Ter um processo em execução para monitorar.

```bash
# Terminal 1: Iniciar um processo qualquer (ex: stress test)
stress --cpu 1 --timeout 300s
# Anote o PID exibido pelo comando: ps aux | grep stress

# Terminal 2: Executar o monitor
sudo ./resource-monitor

# No menu:
Escolha: 1 (Resource Profiler)
Escolha: 1 (Monitorar CPU)
PID: [PID_do_processo]
Duracao (s): 10

# Saída esperada:
[1/10] CPU: 99.5% | Threads: 1
[2/10] CPU: 99.8% | Threads: 1
...
```

#### Exemplo 2: Analisar Namespaces de um Processo

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

#### Exemplo 3: Criar e Limitar Recurso com Cgroup

```bash
sudo ./resource-monitor

# No menu:
Escolha: 3 (Control Group Manager)

# Passo 1: Criar cgroup
Escolha: 1 (Criar cgroup)
Controlador: cpu
Nome do grupo: teste_limite

# Passo 2: Mover processo para o cgroup
Escolha: 2 (Mover PID)
Controlador: cpu
Grupo: teste_limite
PID: 1234

# Passo 3: Definir limite de CPU (50% de 1 núcleo)
Escolha: 4 (Definir limite de CPU)
Grupo: teste_limite
Cores: 0.5

# Passo 4: Verificar uso atual
Escolha: 6 (Ver uso de CPU)
Grupo: teste_limite
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

### Testes Automatizados com Visualização

Execute todos os testes e gere gráficos automaticamente:

```bash
# Teste básico (5 segundos, sem I/O)
./scripts/run_tests.sh 5

# Teste completo com I/O (requer sudo)
sudo ./scripts/run_tests.sh 10
```

**O que acontece:**
1. Compila o projeto e testes
2. Cria um processo de teste em background
3. Executa test_cpu, test_memory e test_io
4. Gera arquivos CSV com timestamp formatado
5. Abre gráficos interativos para cada tipo de métrica

### Visualização de Dados CSV

Visualize qualquer arquivo CSV gerado:

```bash
# Visualizar métricas de CPU
python3 scripts/visualize.py cpu-monitor-*.csv

# Visualizar métricas de memória
python3 scripts/visualize.py memory-monitor-*.csv

# Visualizar métricas de I/O
python3 scripts/visualize.py io-monitor-*.csv
```

**Gráficos gerados:**
- **CPU**: Usage %, User/System Time, Context Switches, Threads
- **Memória**: RSS/VSZ, Page Faults, Swap, Estatísticas
- **I/O**: Disk Rate, Ops/sec, Syscalls, Network Connections

### Validação de Memory Leaks

Valide que não há memory leaks no Resource Profiler:

```bash
# Validação automática de todos os monitores
./scripts/valgrind_test.sh

# Validação manual com valgrind
valgrind --leak-check=full ./resource-monitor
# (escolha opção 1 -> 1 -> <PID> -> 3 -> 0 -> 0)
```

**Resultado esperado:**
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors from 0 contexts
```

### Como Obter PID de Processos

```bash
# Listar todos os processos
ps aux

# Encontrar processo específico
ps aux | grep [nome_processo]

# Ver PIDs por uso de CPU (em tempo real)
top

# Ver PID do próprio terminal
echo $$
```

## Metodologia de Testes

### Objetivo dos Testes

Os programas de teste (`test_cpu`, `test_memory`, `test_io`) validam a precisão e funcionalidade dos monitores através de:
1. Coleta de métricas em intervalos regulares
2. Exibição em tempo real dos dados coletados
3. Validação de leitura correta de `/proc` e `/sys`

### Procedimento de Teste Padrão

Para cada monitor, siga este procedimento:

#### 1. Preparação
```bash
# Compilar o projeto
make tests

# Identificar um processo para monitorar
ps aux | grep [processo]
```

#### 2. Execução do Teste
```bash
# Para CPU e Memória
./test_cpu
# ou
./test_memory

# Para I/O (requer sudo)
sudo ./test_io
```

#### 3. Entrada de Dados
```
Digite o PID: [número_do_processo]
Duração (segundos): [tempo_monitoramento]
```

#### 4. Análise dos Resultados

Quando executados individualmente, os testes exibem métricas detalhadas a cada segundo:

**test_cpu:**
```
[2025-11-17 14:32:01] CPU: 45.2% | User: 1234 ticks | System: 567 ticks | Threads: 4 | Context Switches: 1523
[2025-11-17 14:32:02] CPU: 48.1% | User: 1289 ticks | System: 612 ticks | Threads: 4 | Context Switches: 1687
```

**test_memory:**
```
[2025-11-17 14:32:01] RSS: 125.4 MB | VSZ: 512.8 MB | Page Faults: 3421 | Swap: 0.0 MB
[2025-11-17 14:32:02] RSS: 128.2 MB | VSZ: 512.8 MB | Page Faults: 3456 | Swap: 0.0 MB
```

**test_io:**
```
[2025-11-17 14:32:01] Disk Read: 2.4 MB/s | Disk Write: 1.8 MB/s | Read Ops: 123/s | Write Ops: 89/s | Syscalls: 245 | TCP Conns: 4
[2025-11-17 14:32:02] Disk Read: 3.1 MB/s | Disk Write: 2.2 MB/s | Read Ops: 145/s | Write Ops: 102/s | Syscalls: 312 | TCP Conns: 4
```

#### 5. Testes Automatizados (run_tests.sh)

Quando executado via script automatizado, a saída é resumida:

```bash
sudo ./scripts/run_tests.sh 5
```

**Saída esperada:**
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

════════════════════════════════════════
  TESTE 2: Memory Monitor (5 segundos)
════════════════════════════════════════
✅ CSV gerado: memory-monitor-20251117_150912.csv
   Gerando gráfico...

════════════════════════════════════════
  TESTE 3: I/O Monitor (5 segundos)
════════════════════════════════════════
✅ CSV gerado: io-monitor-20251117_150915.csv
   Gerando gráfico...

Os gráficos serão exibidos em janelas separadas.
```

### Validação de Precisão

Compare os resultados com ferramentas do sistema:

```bash
# Comparar CPU com top
top -p [PID]

# Comparar memória com ps
ps -p [PID] -o pid,rss,vsz

# Comparar I/O com iotop (requer sudo)
sudo iotop -p [PID]
```

### Tratamento de Erros Comuns

| Erro | Causa | Solução |
|------|-------|---------|
| "Permission denied" | Falta de privilégios | Execute com `sudo` |
| "No such process" | PID inválido ou processo morto | Verifique o PID com `ps -p [PID]` |
| "Cannot read /proc/[PID]/io" | I/O requer root | Execute `sudo ./test_io` |

### Casos de Teste Recomendados

1. **Processo de baixa carga**: `sleep 300` (PID estável, baixo uso)
2. **Processo de alta CPU**: `stress --cpu 1` (uso próximo a 100%)
3. **Processo com I/O**: `dd if=/dev/zero of=/tmp/test bs=1M count=100` (escrita intensiva)

## Autores e Contribuições

### Grupo 8 - Turma 04N - Sistemas Operacionais

#### Aluno 1: Felipe Simionato Bueno
**Responsabilidade:** Resource Profiler + Integração + Exportação CSV

**Contribuições:**
- Implementação de `cpu_monitor.c`
  - Coleta de tempo de CPU (user/system time)
  - Cálculo de percentual de uso de CPU
  - Contagem de threads e context switches
  - Exportação automática para CSV com timestamps formatados
  - Funções de cleanup para evitar memory leaks
- Implementação de `memory_monitor.c`
  - Coleta de RSS (Resident Set Size) e VSZ (Virtual Size)
  - Monitoramento de page faults e swap
  - Exportação automática para CSV
  - Funções de cleanup para gerenciamento de recursos
- Integração dos três componentes no menu principal (`main.c`)
  - Desenvolvimento do menu hierárquico interativo
  - Integração de Resource Profiler, Namespace Analyzer e Cgroup Manager
  - Exibição formatada com timestamps legíveis
  - Chamadas automáticas de cleanup após monitoramento
- Criação do `Makefile` base
  - Configuração de flags de compilação (`-Wall -Wextra -std=c17`)
  - Definição de targets para compilação modular
- Estruturação inicial do projeto e organização de diretórios
- Correção de memory leaks e validação com valgrind
  - Implementação de funções `*_csv_close()` para todos os monitores
  - Validação completa: 0 bytes leaked, 0 errors
- Desenvolvimento de scripts de automação
  - `run_tests.sh`: Execução automatizada de testes com geração de CSVs
  - `valgrind_test.sh`: Validação automatizada de memory leaks
- Atualização de `visualize.py`
  - Conversão de timestamps Unix para formato legível
  - Geração de gráficos para CPU, Memória e I/O
  - Visualização multi-painel com métricas detalhadas

#### Aluno 2: Vinicius Pelissari Jordani
**Responsabilidade:** Resource Profiler (I/O) + Testes + Exportação CSV

**Contribuições:**
- Implementação de `io_monitor.c`
  - Função `io_monitor_init()`: inicialização do estado de monitoramento
  - Função `io_monitor_sample()`: coleta de métricas de I/O via `/proc/<pid>/io`
  - Coleta de estatísticas de rede via `/proc/net/dev`
  - Contagem de conexões TCP ativas via `/proc/net/tcp`
  - Cálculo de taxas de leitura/escrita (bytes/s)
  - Exportação automática para CSV com todas as métricas de I/O e rede
  - Função de cleanup para gerenciamento de recursos
- Criação de `test_io.c` para validação do monitor de I/O
  - Loop de amostragem configurável
  - Exibição de métricas em tempo real
  - Exportação automática para CSV durante testes
- Atualização do Makefile com targets de testes
  - Adição de regras para `test_cpu`, `test_memory`, `test_io`
  - Configuração de linkagem com `-lm`
- Correção de bugs críticos em `sscanf`
  - Fix no parsing de `/proc/<pid>/stat` (cpu_monitor.c)
  - Fix no parsing de `/proc/<pid>/statm` (memory_monitor.c)
- Documentação de uso, testes e troubleshooting
- Atualização dos programas de teste para incluir exportação CSV
  - Integração de `*_csv_write()` e `*_csv_close()` em todos os testes

#### Aluno 3: Kevin Mitsuo Lohmann Abe
**Responsabilidade:** Namespace Analyzer + Experimentos

**Contribuições:**
- Implementação completa de `namespace_analyzer.c`
  - `list_process_namespaces()`: listagem de namespaces via `/proc/<pid>/ns/`
  - `compare_namespaces()`: comparação de namespaces entre processos
  - `list_namespace_members()`: identificação de processos por inode de namespace
  - `measure_namespace_overhead()`: medição de overhead via `clone()` syscall
  - `generate_namespace_report()`: geração de relatórios em formato CSV
- Análise detalhada de tipos de namespace
  - pid, net, mnt, uts, ipc, user
  - Identificação de inodes únicos
- Execução e documentação de experimentos de isolamento
  - Medição de tempo de criação de namespaces
  - Validação de efetividade de isolamento
- Geração de relatórios científicos em CSV para análise posterior

- Documentação completa dos experimentos disponível em:
  `docs/experimentos_namespace.md`

#### Aluno 4: João Barowski
**Responsabilidade:** Control Group Manager + Análise

**Contribuições:**
- Implementação de `cgroup_manager.c`
  - `cgroup_create()`: criação de cgroups em `/sys/fs/cgroup`
  - `cgroup_move_pid()`: movimentação de processos entre cgroups
  - `cgroup_set_cpu_limit()`: aplicação de limites de CPU
  - `cgroup_set_memory_limit()`: aplicação de limites de memória
  - `cgroup_get_cpu_usage()`: leitura de uso de CPU do cgroup
  - `cgroup_get_memory_usage()`: leitura de uso de memória
  - `cgroup_get_io_stats()`: leitura de estatísticas de I/O
- Condução de experimentos de throttling
  - Testes de precisão de limitação de CPU (0.25, 0.5, 1.0, 2.0 cores)
  - Testes de limitação de memória e comportamento de OOM killer
- Geração de relatórios comparativos (utilização vs limites configurados)
- Testes de precisão e validação de funcionalidade dos cgroups

## Interfaces do Kernel Utilizadas

O projeto interage diretamente com as seguintes interfaces do kernel Linux:

- **`/proc/<pid>/stat`**: Estatísticas de processos (CPU, threads, state)
- **`/proc/<pid>/statm`**: Uso de memória (RSS, VSZ)
- **`/proc/<pid>/status`**: Informações detalhadas (swap, context switches)
- **`/proc/<pid>/io`**: Estatísticas de I/O (requer privilégios de root)
- **`/proc/<pid>/ns/*`**: Namespaces de processos (pid, net, mnt, uts, ipc, user)
- **`/proc/net/dev`**: Estatísticas de interfaces de rede
- **`/proc/net/tcp`**: Conexões TCP ativas
- **`/sys/fs/cgroup/*`**: Interfaces de control groups (cgroup v1)

## Troubleshooting

### Problema: "Permission denied" ao acessar /proc/<pid>/io
**Solução:** Execute o programa com privilégios de root:
```bash
sudo ./resource-monitor
# ou
sudo ./test_io
```

### Problema: "No such file or directory" ao acessar cgroup
**Solução:** Verifique se cgroup v1 está montado no sistema:
```bash
ls /sys/fs/cgroup/cpu
ls /sys/fs/cgroup/memory
```
Se não existir, o sistema pode estar usando cgroup v2 (não suportado nesta versão).

### Problema: "Não foi possível ler tempos do processo"
**Solução:** Verifique se o processo ainda existe:
```bash
ps -p [PID]
```
Se não aparecer, o processo foi finalizado. Tente com outro PID.

### Problema: Programa não compila no Windows
**Solução:** Este programa requer APIs do Linux. Use WSL (Windows Subsystem for Linux):
```bash
# Instalar WSL (PowerShell como admin)
wsl --install

# Após instalação, abrir WSL
wsl

# Navegar para o projeto e compilar
cd /mnt/c/Users/[usuario]/caminho/para/projeto
make all
```

### Problema: Warnings durante compilação
**Solução:** O projeto deve compilar sem warnings. Se aparecerem:
1. Verifique a versão do GCC: `gcc --version` (recomendado >= 9.0)
2. Certifique-se de estar compilando em Linux (não em Windows nativo)
3. Reporte o warning para correção

## Documentação Adicional

- **docs/ARCHITECTURE.md**: Descrição detalhada da arquitetura do sistema
- **include/*.h**: Documentação inline das interfaces (comentários doxygen-style)
- **Código-fonte comentado**: Todas as funções principais possuem comentários explicativos

## Licença

Este projeto foi desenvolvido para fins acadêmicos como parte da disciplina de Sistemas Operacionais do **Insper - Instituto de Ensino e Pesquisa**, sob orientação do corpo docente.

## Referências Técnicas

- Linux Programmer's Manual: `man proc`, `man cgroups`, `man namespaces`
- Kernel Documentation: `/Documentation/filesystems/proc.txt`
- Control Groups v1: `/Documentation/cgroup-v1/`
- Namespaces in operation: `man 7 namespaces`
- The Linux Programming Interface (Michael Kerrisk)