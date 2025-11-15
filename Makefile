# Nome do executável final
TARGET = resource-monitor

# Compilador
CC = gcc

# Flags de Compilação
# Nota: O gcc do Ubuntu 20.04 não suporta -std=c23. 
# Usaremos -std=c17, que é moderno e compatível.
CFLAGS = -Wall -Wextra -std=c17 -Iinclude -g

# Flags de Linkagem (adiciona math lib para workloads)
LDFLAGS = -lm

# Encontrar todos os arquivos .c na pasta src/
SRCS = $(wildcard src/*.c)

# Gerar nomes dos arquivos objeto (.o) a partir dos .c
OBJS = $(SRCS:.c=.o)

# Arquivos de teste
TEST_PROGS = test_cpu test_memory test_io

# Regra principal: compilar o executável e todos os testes
all: $(TARGET) tests

# Regra para linkar o executável final
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Regra para compilar arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===== TESTES =====

# Compila todos os testes
tests: $(TEST_PROGS)

# test_cpu: monitora CPU de um processo
test_cpu: tests/test_cpu.c $(filter-out src/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# test_memory: monitora memória de um processo
test_memory: tests/test_memory.c $(filter-out src/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# test_io: monitora I/O de um processo
test_io: tests/test_io.c $(filter-out src/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ===== LIMPEZA =====

# Regra para limpar os arquivos compilados
clean:
	rm -f $(TARGET) $(OBJS) $(TEST_PROGS)

# Phony garante que as regras executem mesmo se existir arquivo com o mesmo nome
.PHONY: all tests clean