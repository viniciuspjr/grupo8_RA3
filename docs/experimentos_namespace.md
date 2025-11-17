## Experimento 2 – Isolamento via Namespaces (Aluno 3 – Kevin Mitsuo Lohmann Abe)

### 2.1. Objetivo

Avaliar o impacto do uso de namespaces no isolamento de recursos em Linux
e medir o overhead de criação de diferentes tipos de namespaces
(PID, NET, MNT, UTS, IPC, USER). Além disso, mapear a distribuição de
processos pelos namespaces ativos no sistema.

### 2.2. Ambiente de Teste

- Sistema operacional: Linux (WSL / distribuição utilizada)
- Kernel: (saída do comando `uname -r`)
- Execução em ambiente de usuário (sem privilégios de root, exceto onde necessário)
- Programa desenvolvido neste trabalho: **Resource Monitor**
  - Módulo utilizado: **Namespace Analyzer** (Aluno 3)

### 2.3. Metodologia

1. **Cenários de processos**

   Foram definidos os seguintes cenários:

   - **Cenário 0** – Processo normal do host, sem isolamento extra.
   - **Cenário 1** – Processo criado com `unshare --pid --fork --mount-proc bash`.
   - **Cenário 2** – Processo criado com `unshare --net --fork bash`.
   - **Cenário 3** – Processo criado com `unshare --pid --net --mount --uts --fork bash`.

   Em cada cenário, foi obtido o PID do processo de teste
   (via `echo $$` e `ps aux`) e, em seguida,
   o Namespace Analyzer foi utilizado para listar e comparar
   os namespaces entre o processo de teste e um processo do host.

2. **Ferramentas do Namespace Analyzer**

   As seguintes funções/opções foram utilizadas:

   - `Listar namespaces de um processo` (menu 2, opção 1)
   - `Comparar namespaces entre dois processos` (menu 2, opção 2)
   - `Listar processos em um namespace` (menu 2, opção 3)
   - `Medir overhead de criacao` (menu 2, opção 4)
   - `Gerar relatorio completo` (menu 2, opção 5)

3. **Medição de overhead**

   Para cada tipo de namespace, a função `measure_namespace_overhead()`
   cria processos via `clone()` com as flags apropriadas
   (por exemplo, `CLONE_NEWPID`, `CLONE_NEWNET`, etc.) e mede o tempo
   de criação com `clock_gettime(CLOCK_MONOTONIC)`. São realizadas
   várias iterações e o programa imprime o tempo médio em microssegundos (µs).

4. **Relatório de processos por namespace**

   A opção 5 gera um arquivo CSV (`namespace_report.csv`) com as colunas:

   - `namespace` – tipo de namespace (pid, mnt, net, uts, ipc, user, etc.)
   - `inode` – identificador do namespace
   - `pid_count` – número de processos associados àquele namespace

   Esse arquivo foi utilizado para identificar quais namespaces são mais
   compartilhados no sistema.

### 2.4. Resultados

#### 2.4.1. Isolamento por tipo de namespace

A Tabela 1 resume se houve isolamento de cada tipo de namespace
ao comparar processos do host com processos criados com `unshare`.

**Tabela 1 – Isolamento efetivo por tipo de namespace**

| Cenário                       | PID namespace | NET namespace | MNT namespace | UTS namespace | IPC namespace | USER namespace |
|-------------------------------|---------------|---------------|---------------|---------------|---------------|----------------|
| 0 – Processo host             | Igual ao host | Igual ao host | Igual ao host | Igual ao host | Igual ao host | Igual ao host  |
| 1 – `unshare --pid`           | Diferente     | Compartilham  | Compartilham  | Diferente     | Diferente     | Diferente      |
| 2 – `unshare --net`           | Compartilham  | Compartilham  | Compartilham  | Igual         | Compartilham  | Diferente      |
| 3 – `unshare --pid --net...`  | Diferente     | Compartilham  | Compartilham  | Diferente     | Diferente     | Diferente      |
   
#### 2.4.2. Overhead de criação de namespaces

A Tabela 2 apresenta o tempo médio de criação de cada tipo de namespace,
medido pela função `measure_namespace_overhead()`.

**Tabela 2 – Overhead médio de criação de namespaces**

| Tipo de namespace | Tempo médio de criação (µs) |
|-------------------|-----------------------------|
| pid               | 79 us                       |
| net               | 80 us                       |
| mnt               | 76 us                       |
| uts               | 73 us                       |
| ipc               | 82 us                       |
| user              | 94 us                       |

#### 2.4.3. Número de processos por namespace

A partir do arquivo `namespace_report.csv`, foi possível observar
a distribuição de processos por namespace. A Tabela 3 resume alguns
dos namespaces mais representativos.

**Tabela 3 – Exemplos de namespaces e número de processos**

| Tipo   | Inode      |  Qtde. de processos (`pid_count`) |
| ------ | ---------- | ----------------------------------|
| pid    | 4026532290 | 1                                 |
| pid    | 4026532226 | 4                                 |
| pid    | 4026532220 | 6                                 |
| net    | 4026532291 | 2                                 |
| net    | 4026532227 | 2                                 |
| net    | 4026531840 | 7                                 |
| mnt    | 4026532288 | 2                                 |
| mnt    | 4026532218 | 9                                 |
| uts    | 4026532289 | 2                                 |
| uts    | 4026532219 | 9                                 |
| ipc    | 4026532206 | 11                                |
| user   | 4026532225 | 6                                 |
| user   | 4026531837 | 5                                 |
| cgroup | 4026531835 | 11                                |

### 2.5. Discussão

Os resultados mostram que:

- O uso de `unshare --pid` cria um novo namespace de PID, isolando
  a visão dos processos internos em relação ao host.
- O uso de `unshare --net` cria uma pilha de rede separada, o que pode
  ser observado pela diferença de interfaces de rede e pelo namespace `net`.
- Combinações como `unshare --pid --net --mount --uts` resultam em
  um isolamento mais completo, afetando PIDs, rede, pontos de montagem
  e nome da máquina (`hostname`).

Em relação ao overhead, os tempos médios de criação mostraram-se
(baixo/moderado/alto – ajustar conforme os valores medidos), indicando
que a criação de namespaces é (relativamente barata / aceitável)
para o uso em containers e ambientes isolados.

O relatório `namespace_report.csv` também evidencia que alguns
namespaces (como determinados `mnt` e `pid`) são amplamente
compartilhados entre processos, enquanto outros podem ser mais
específicos de serviços ou sessões.
