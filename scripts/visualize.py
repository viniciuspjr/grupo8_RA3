#!/usr/bin/env python3
"""
visualize.py - Visualização simples de arquivos CSV gerados pelo resource-monitor.

Uso:
    python3 visualize.py namespace_report.csv
"""

import sys
import pandas as pd

try:
    import matplotlib.pyplot as plt
    HAS_MPL = True
except ImportError:
    HAS_MPL = False

def main():
    if len(sys.argv) < 2:
        print("Uso: python3 visualize.py <arquivo.csv>")
        sys.exit(1)

    path = sys.argv[1]
    df = pd.read_csv(path)
    
    # Converte timestamp Unix para formato legível se existir
    if 'timestamp' in df.columns:
        df['timestamp'] = pd.to_datetime(df['timestamp'], unit='s')
    
    print("\nPrimeiras linhas do CSV:\n")
    print(df.head())

    if not HAS_MPL:
        print("\nmatplotlib não encontrado. Mostrando apenas a tabela no terminal.")
        return

    # CPU Monitor
    if 'cpu_percent' in df.columns:
        fig, axes = plt.subplots(2, 2, figsize=(12, 8))
        fig.suptitle('CPU Monitor - Métricas', fontsize=16)
        
        df.plot(x='timestamp', y='cpu_percent', ax=axes[0, 0], legend=False, color='red')
        axes[0, 0].set_title('CPU Usage (%)')
        axes[0, 0].set_ylabel('CPU %')
        axes[0, 0].grid(True)
        
        df.plot(x='timestamp', y=['user_time_ticks', 'system_time_ticks'], ax=axes[0, 1])
        axes[0, 1].set_title('CPU Time (ticks)')
        axes[0, 1].set_ylabel('Ticks')
        axes[0, 1].grid(True)
        
        df.plot(x='timestamp', y='context_switches', ax=axes[1, 0], legend=False, color='orange')
        axes[1, 0].set_title('Context Switches')
        axes[1, 0].set_ylabel('Count')
        axes[1, 0].grid(True)
        
        df.plot(x='timestamp', y='threads', ax=axes[1, 1], legend=False, color='green')
        axes[1, 1].set_title('Threads')
        axes[1, 1].set_ylabel('Count')
        axes[1, 1].grid(True)
        
        plt.tight_layout()
        plt.show()
        return

    # Memory Monitor
    if 'rss_bytes' in df.columns:
        fig, axes = plt.subplots(2, 2, figsize=(12, 8))
        fig.suptitle('Memory Monitor - Métricas', fontsize=16)
        
        df['rss_mb'] = df['rss_bytes'] / (1024 * 1024)
        df['vsize_mb'] = df['vsize_bytes'] / (1024 * 1024)
        df['swap_mb'] = df['swap_bytes'] / (1024 * 1024)
        
        df.plot(x='timestamp', y=['rss_mb', 'vsize_mb'], ax=axes[0, 0])
        axes[0, 0].set_title('Memory Usage')
        axes[0, 0].set_ylabel('MB')
        axes[0, 0].grid(True)
        
        df.plot(x='timestamp', y='page_faults', ax=axes[0, 1], legend=False, color='red')
        axes[0, 1].set_title('Page Faults')
        axes[0, 1].set_ylabel('Count')
        axes[0, 1].grid(True)
        
        df.plot(x='timestamp', y='swap_mb', ax=axes[1, 0], legend=False, color='purple')
        axes[1, 0].set_title('Swap Usage')
        axes[1, 0].set_ylabel('MB')
        axes[1, 0].grid(True)
        
        axes[1, 1].axis('off')
        stats_text = f"RSS Médio: {df['rss_mb'].mean():.2f} MB\n"
        stats_text += f"VSZ Médio: {df['vsize_mb'].mean():.2f} MB\n"
        stats_text += f"Page Faults Total: {df['page_faults'].iloc[-1]}\n"
        stats_text += f"Swap Máximo: {df['swap_mb'].max():.2f} MB"
        axes[1, 1].text(0.1, 0.5, stats_text, fontsize=12, verticalalignment='center')
        
        plt.tight_layout()
        plt.show()
        return

    # I/O Monitor
    if 'read_bytes' in df.columns:
        fig, axes = plt.subplots(2, 2, figsize=(12, 8))
        fig.suptitle('I/O Monitor - Métricas', fontsize=16)
        
        df['read_kb_s'] = df['read_rate_bytes_per_sec'] / 1024
        df['write_kb_s'] = df['write_rate_bytes_per_sec'] / 1024
        
        df.plot(x='timestamp', y=['read_kb_s', 'write_kb_s'], ax=axes[0, 0])
        axes[0, 0].set_title('Disk I/O Rate')
        axes[0, 0].set_ylabel('KB/s')
        axes[0, 0].grid(True)
        
        df.plot(x='timestamp', y='disk_ops_per_sec', ax=axes[0, 1], legend=False, color='orange')
        axes[0, 1].set_title('Disk Operations per Second')
        axes[0, 1].set_ylabel('Ops/s')
        axes[0, 1].grid(True)
        
        df.plot(x='timestamp', y='io_syscalls', ax=axes[1, 0], legend=False, color='green')
        axes[1, 0].set_title('I/O System Calls')
        axes[1, 0].set_ylabel('Count')
        axes[1, 0].grid(True)
        
        df.plot(x='timestamp', y='connections', ax=axes[1, 1], legend=False, color='blue')
        axes[1, 1].set_title('Network Connections')
        axes[1, 1].set_ylabel('Count')
        axes[1, 1].grid(True)
        
        plt.tight_layout()
        plt.show()
        return

    # Namespace Report (original)
    if {"inode", "pid_count"}.issubset(df.columns):
        plt.figure()
        df.plot(kind="bar", x="inode", y="pid_count", legend=False)
        plt.title("Processos por namespace (inode)")
        plt.xlabel("inode")
        plt.ylabel("pid_count")
        plt.tight_layout()
        plt.show()
        return
    
    print("\nNenhum formato de gráfico reconhecido para este CSV.")

if __name__ == "__main__":
    main()