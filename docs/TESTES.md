# Testes e Workloads - Aluno 2

## O que foi feito

Implementei a coleta de I/O e rede, além dos programas de teste para validar os monitores.

## Monitor de I/O

O arquivo `io_monitor.c` coleta:

- **Disco** (lê `/proc/<pid>/io`):
  - Bytes lidos e escritos
  - Syscalls de I/O
  - Taxas de leitura/escrita

- **Rede** (lê `/proc/net/dev` e `/proc/net/tcp`):
  - Bytes e pacotes recebidos/transmitidos
  - Conexões TCP ativas

**Obs:** Precisa de sudo para funcionar: `sudo ./test_io`

## Programas de Teste

Criei 3 workloads para testar os monitores:

### 1. workload_cpu
Faz cálculos matemáticos pesados (sqrt, sin, cos, log).

```bash
./workload_cpu 30    # roda por 30 segundos
```

### 2. workload_memory
Aloca memória em incrementos de 50 MB até o limite.

```bash
./workload_memory 200    # aloca até 200 MB
```

### 3. workload_io
Escreve e lê arquivos grandes em /tmp.

```bash
./workload_io 100    # usa arquivo de 100 MB
```

## Como usar

**Exemplo básico:**

```bash
# Terminal 1
./workload_cpu 60

# Terminal 2 (pega o PID que aparece no terminal 1)
./test_cpu
```

**Para testar I/O:**

```bash
# Terminal 1
./workload_io 50

# Terminal 2 (precisa de sudo)
sudo ./test_io
```

## Limitações conhecidas

- Monitor de rede mostra estatísticas do sistema todo, não por processo (é assim que /proc/net/dev funciona)
- Precisa de root para acessar /proc/<pid>/io

## Compilar

```bash
make all        # compila tudo
make tests      # só os testes
make workloads  # só os workloads
```
