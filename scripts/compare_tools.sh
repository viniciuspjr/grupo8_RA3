#!/usr/bin/env bash

# Uso: ./compare_tools.sh <PID>
# Compara as métricas básicas de um processo usando ps e top.
# Este script é complementar e não faz parte do escopo obrigatório.

if [ $# -ne 1 ]; then
    echo "Uso: $0 <PID>"
    exit 1
fi

PID="$1"

echo "---------------------------------------------"
echo "Comparação de métricas para o PID: $PID"
echo "---------------------------------------------"

echo
ps -p "$PID" -o pid,%cpu,%mem,vsz,rss,command

echo
echo "===> Métricas coletadas pelo top (snapshot único):"
top -b -n 1 -p "$PID" | sed -n '7,12p'

echo
echo "Observação: use o resource-monitor/testes para comparar com essas métricas."
