#!/bin/bash

# Script para executar testes automatizados e gerar gráficos
# Uso: sudo ./scripts/run_tests.sh [duração_em_segundos]

DURATION=${1:-5}  # Duração padrão: 5 segundos

# Encontra o diretório raiz do projeto
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Muda para o diretório do projeto
cd "$PROJECT_DIR" || exit 1

echo "========================================"
echo "  TESTES AUTOMATIZADOS - RESOURCE MONITOR"
echo "========================================"
echo ""

# Compila o projeto
echo "Compilando o projeto..."
make clean > /dev/null 2>&1
make tests > /dev/null 2>&1

# Verifica se a compilação foi bem-sucedida
if [ ! -f "./test_cpu" ]; then
    echo "❌ Erro na compilação"
    exit 1
fi

# Cria processo de teste em background
echo "Iniciando processo de teste..."
sleep 120 &
TEST_PID=$!
echo "✓ PID de teste: $TEST_PID"
echo ""

# Teste CPU
echo "════════════════════════════════════════"
echo "  TESTE 1: CPU Monitor ($DURATION segundos)"
echo "════════════════════════════════════════"
echo -e "$TEST_PID\n$DURATION" | ./test_cpu > /dev/null 2>&1
CPU_CSV=$(ls -t cpu-monitor-*.csv 2>/dev/null | head -1)

# Verifica se o CSV foi gerado
if [ -n "$CPU_CSV" ]; then
    echo "✅ CSV gerado: $CPU_CSV"
    echo "   Gerando gráfico..."
    python3 scripts/visualize.py "$CPU_CSV" > /dev/null 2>&1 &
else
    echo "❌ Falha ao gerar CSV"
fi
echo ""

# Teste Memory
echo "════════════════════════════════════════"
echo "  TESTE 2: Memory Monitor ($DURATION segundos)"
echo "════════════════════════════════════════"
echo -e "$TEST_PID\n$DURATION" | ./test_memory > /dev/null 2>&1
MEM_CSV=$(ls -t memory-monitor-*.csv 2>/dev/null | head -1)

# Verifica se o CSV foi gerado
if [ -n "$MEM_CSV" ]; then
    echo "✅ CSV gerado: $MEM_CSV"
    echo "   Gerando gráfico..."
    python3 scripts/visualize.py "$MEM_CSV" > /dev/null 2>&1 &
else
    echo "❌ Falha ao gerar CSV"
fi
echo ""

# Teste I/O (requer sudo)
echo "════════════════════════════════════════"
echo "  TESTE 3: I/O Monitor ($DURATION segundos)"
echo "════════════════════════════════════════"

# Verifica se está sendo executado como root
if [ "$EUID" -eq 0 ]; then
    echo -e "$TEST_PID\n$DURATION" | ./test_io > /dev/null 2>&1
    IO_CSV=$(ls -t io-monitor-*.csv 2>/dev/null | head -1)
    
    # Verifica se o CSV foi gerado
    if [ -n "$IO_CSV" ]; then
        echo "✅ CSV gerado: $IO_CSV"
        echo "   Gerando gráfico..."
        python3 scripts/visualize.py "$IO_CSV" > /dev/null 2>&1 &
    else
        echo "❌ Falha ao gerar CSV"
    fi
else
    echo "⚠️  I/O Monitor requer sudo. Execute: sudo ./scripts/run_tests.sh"
fi
echo ""

# Limpa processo de teste
kill $TEST_PID 2>/dev/null
wait $TEST_PID 2>/dev/null

echo ""
echo "Os gráficos serão exibidos em janelas separadas."
echo ""
echo "Para visualizar um CSV específico:"
echo "  python3 scripts/visualize.py <arquivo.csv>"
echo ""