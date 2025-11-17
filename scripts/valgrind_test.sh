#!/bin/bash

# Script para validar ausência de memory leaks no Resource Profiler
# Uso: sudo ./scripts/valgrind_test.sh
# Ou se estiver na pasta /scripts: sudo ./valgrind_test.sh

# Verifica se o Valgrind está instalado
if ! command -v valgrind &> /dev/null; then
    echo "Valgrind não está instalado. Instale com: sudo apt install valgrind"
    exit 1
fi

# Compila o projeto
echo "Compilando o projeto..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

# Verifica se a compilação foi bem-sucedida
if [ ! -f "./resource-monitor" ]; then
    echo "Erro: Falha na compilação"
    exit 1
fi

# Inicia um processo de teste
echo "Iniciando processo de teste..."
sleep 60 &
PID=$!
echo "PID de teste: $PID"
echo ""

# teste CPU
echo "=== TESTE 1: CPU Monitor ==="
echo -e "1\n1\n$PID\n3\n0\n0" | valgrind --leak-check=full ./resource-monitor
echo ""

# teste memória
echo "=== TESTE 2: Memory Monitor ==="
echo -e "1\n2\n$PID\n3\n0\n0" | valgrind --leak-check=full ./resource-monitor
echo ""

# teste I/O
echo "=== TESTE 3: I/O Monitor (requer sudo) ==="
echo -e "1\n3\n$PID\n3\n0\n0" | sudo valgrind --leak-check=full ./resource-monitor
echo ""

# teste completo
echo "=== TESTE 4: Monitor Completo (requer sudo) ==="
echo -e "1\n4\n$PID\n3\n0\n0" | sudo valgrind --leak-check=full ./resource-monitor
echo ""

# Remove os csvs gerados
kill $PID 2>/dev/null
rm -f *-monitor-*.csv
echo "Testes concluídos!"