# Nome do executável final
TARGET = resource-monitor

# Compilador
CC = gcc

# Flags de Compilação
# Nota: O gcc do Ubuntu 20.04 não suporta -std=c23. 
# Usaremos -std=c17, que é moderno e compatível.
CFLAGS = -Wall -Wextra -std=c17 -Iinclude -g

# Flags de Linkagem
LDFLAGS =

# Encontrar todos os arquivos .c na pasta src/
SRCS = $(wildcard src/*.c)

# Gerar nomes dos arquivos objeto (.o) a partir dos .c
OBJS = $(SRCS:.c=.o)

# Regra principal: compilar o executável
all: $(TARGET)

# Regra para linkar o executável final
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Regra para compilar arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos compilados
clean:
	rm -f $(TARGET) $(OBJS)

# Phony garante que 'all' e 'clean' executem
.PHONY: all clean