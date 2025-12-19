import struct
from dataclasses import dataclass
from typing import List, BinaryIO

import tqdm
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

import time

@dataclass
class IndexItem:
    x: int
    y: int
    z: int
    size: int
    offset: int

@dataclass
class StatsItem:
    x: int
    y: int
    z: int
    visits: int

def parse_index_file_generator(filepath: str):
    """Generator that yields IndexItems one at a time"""
    struct_format = "<4IQ"
    struct_size = struct.calcsize(struct_format)
    
    with open(filepath, 'rb') as f:
        while True:
            data = f.read(struct_size)
            if not data or len(data) < struct_size:
                break
            
            unpacked = struct.unpack(struct_format, data)
            yield IndexItem(
                x=unpacked[0],
                y=unpacked[1],
                z=unpacked[2],
                size=unpacked[3],
                offset=unpacked[4]
            )

def parse_stats_file_generator(filepath: str):
    struct_format = "<4Q"
    struct_size = struct.calcsize(struct_format) 
    
    with open(filepath, 'rb') as f:
        while True:
            data = f.read(struct_size)
            if not data or len(data) < struct_size:
                break
            x, y, z, visits = struct.unpack(struct_format, data)
            yield StatsItem(x=x, y=y, z=z, visits=visits)


if __name__ == "__main__":
    index_path = "/home/yc-user/tiles/tilesets/data/2025-08-02-planet.index"
    stats_path = "/home/yc-user/tiles/tilesets/log/tiles-2025-09-01.bin"

    only_visits = 1

    stats_info = []
    bar = tqdm.tqdm(total=31703414, desc="Processing stats")
    for item in parse_stats_file_generator(stats_path):
        stats_info.append(item)
        bar.update(1)
    bar.close()

    if not only_visits:
    
    # Read index data
        cnt = 0
        tile_info = []
        bar = tqdm.tqdm(total=69875051, desc="Processing tiles")
        for item in parse_index_file_generator(index_path):
            tile_info.append(item)
            bar.update(1)
            cnt += 1
            if cnt == 69875051:
                break
        bar.close()

        """
        посчитать статистики
        1) распределение тайлов по размеру в зависимости от зума
        2) сколько в среднем на тайл визитов в зависимости от зума
        """

        # 1) Распределение тайлов по размеру в зависимости от зума
        print("\n=== Распределение тайлов по размеру в зависимости от зума ===")

        # Группируем тайлы по зумам
        tiles_by_zoom = {}
        for zoom in range(14):
            if zoom not in tiles_by_zoom:
                tiles_by_zoom[zoom] = []

        for tile in tile_info:
            zoom = tile.z
            
            for zoom_id in range(zoom, 14):
                tiles_by_zoom[zoom_id].append(tile)

        # Выводим статистику по зумам
        print(f"Всего зумов: {len(tiles_by_zoom)}")
        for zoom in sorted(tiles_by_zoom.keys()):
            tiles = tiles_by_zoom[zoom]
            sizes = [tile.size for tile in tiles]
            print(f"Zoom {zoom}: {len(tiles)} тайлов, размеры: "
                    f"min={min(sizes)}B, max={max(sizes)}B, "
                    f"avg={np.mean(sizes):.0f}B, med={np.median(sizes):.0f}B")

        # Создаем графики
        # Вариант 1: Общий график для всех зумов (в логарифмическом масштабе)
        print("\nСоздание общего графика распределения размеров тайлов...")
        fig1, ax1 = plt.subplots(figsize=(14, 8))

        all_sizes = [tile.size for tile in tile_info]

        # Логарифмическая гистограмма
        ax1.hist(all_sizes, bins=100, log=True, alpha=0.7, color='blue', edgecolor='black')
        ax1.set_xlabel('Размер тайла (байты)', fontsize=12)
        ax1.set_ylabel('Количество тайлов (log scale)', fontsize=12)
        ax1.set_title('Распределение размеров тайлов (все зумы)', fontsize=14, fontweight='bold')
        ax1.grid(True, alpha=0.3)

        # Добавляем вертикальные линии для квартилей
        q25, q50, q75 = np.percentile(all_sizes, [25, 50, 75])
        ax1.axvline(q50, color='red', linestyle='--', alpha=0.7, label=f'Медиана: {int(q50)} B')
        ax1.axvline(q25, color='orange', linestyle='--', alpha=0.5, label=f'25-й перцентиль: {int(q25)} B')
        ax1.axvline(q75, color='green', linestyle='--', alpha=0.5, label=f'75-й перцентиль: {int(q75)} B')

        ax1.legend()
        plt.tight_layout()
        plt.savefig('tile_sizes_all_zooms_2.png', dpi=150, bbox_inches='tight')

        # Вариант 2: Отдельные графики для каждого зума
        print("\nСоздание отдельных графиков для каждого зума...")

        # Определяем оптимальную сетку для subplots
        zoom_levels = sorted(tiles_by_zoom.keys())
        n_zooms = len(zoom_levels)

        # Вычисляем размеры сетки
        cols = 4
        rows = (n_zooms + cols - 1) // cols

        fig2, axes = plt.subplots(rows, cols, figsize=(20, 4*rows))
        axes = axes.flatten() if n_zooms > 1 else [axes]

        for idx, zoom in enumerate(zoom_levels):
            ax = axes[idx]
            sizes = [tile.size for tile in tiles_by_zoom[zoom]]
            
            # Гистограмма в логарифмическом масштабе
            ax.hist(sizes, bins=50, log=True, alpha=0.7, color='steelblue', edgecolor='black')
            ax.set_title(f'Zoom {zoom} (n={len(sizes):,})', fontsize=11, fontweight='bold')
            ax.set_xlabel('Размер (B)')
            ax.set_ylabel('Количество (log)')
            ax.grid(True, alpha=0.3)
            
            # Добавляем медиану
            median_size = np.median(sizes)
            ax.axvline(median_size, color='red', linestyle='--', alpha=0.7, 
                        label=f'Медиана: {int(median_size)} B')
            ax.legend(fontsize=9)
            
            # Форматируем большие числа на оси X
            if max(sizes) > 1000000:  # Если размеры в мегабайтах
                ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=True))
                ax.ticklabel_format(style='sci', axis='x', scilimits=(0,0))

        # Скрываем пустые subplots
        for idx in range(len(zoom_levels), len(axes)):
            axes[idx].set_visible(False)

        plt.suptitle('Распределение размеров тайлов по зумам', fontsize=16, fontweight='bold', y=1.02)
        plt.tight_layout()
        plt.savefig('tile_sizes_by_zoom_2.png', dpi=150, bbox_inches='tight')

        # Вариант 3: Box plot для сравнения распределений по зумам
        print("\nСоздание Box plot для сравнения распределений...")
        fig3, ax3 = plt.subplots(figsize=(14, 8))

        # Собираем данные для box plot
        box_data = []
        box_labels = []
        for zoom in sorted(tiles_by_zoom.keys()):
            sizes = [tile.size for tile in tiles_by_zoom[zoom]]
            if len(sizes) > 10:  # Только зумы с достаточным количеством тайлов
                box_data.append(sizes)
                box_labels.append(f'Z{zoom}\n(n={len(sizes):,})')

        # Создаем box plot с логарифмической шкалой
        box = ax3.boxplot(box_data, labels=box_labels, showfliers=False, patch_artist=True)

        # Раскрашиваем box'ы
        colors = plt.cm.viridis(np.linspace(0, 1, len(box_data)))
        for patch, color in zip(box['boxes'], colors):
            patch.set_facecolor(color)
            patch.set_alpha(0.7)

        ax3.set_yscale('log')
        ax3.set_ylabel('Размер тайла (байты, log scale)', fontsize=12)
        ax3.set_xlabel('Уровень зума', fontsize=12)
        ax3.set_title('Сравнение распределений размеров тайлов по зумам (без выбросов)', 
                        fontsize=14, fontweight='bold')
        ax3.grid(True, alpha=0.3, axis='y')
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig('tile_sizes_boxplot_2.png', dpi=150, bbox_inches='tight')

    # 2) Сколько в среднем на тайл визитов в зависимости от зума
    print("\n=== Среднее количество визитов на тайл в зависимости от зума ===")
    
    # Группируем статистику по зумам

    visits_by_zoom = {}
    for stat in stats_info:
        zoom_id = stat.z

        for zoom in range(zoom_id, 14):
            if zoom not in visits_by_zoom:
                visits_by_zoom[zoom] = []
            visits_by_zoom[zoom].append(stat.visits)
    
    # Вычисляем средние значения
    avg_visits_by_zoom = {}
    for zoom in sorted(visits_by_zoom.keys()):
        visits = visits_by_zoom[zoom]
        avg_visits_by_zoom[zoom] = np.mean(visits)
        print(f"Zoom {zoom}: {len(visits)} тайлов, "
              f"среднее визитов: {avg_visits_by_zoom[zoom]:.2f}, "
              f"медиана: {np.median(visits):.2f}")
    
    # Создаем график средних визитов по зумам
    print("\nСоздание графика средних визитов по зумам...")
    fig4, (ax4a, ax4b) = plt.subplots(1, 2, figsize=(16, 6))
    
    # График 1: Среднее количество визитов
    zooms = sorted(avg_visits_by_zoom.keys())
    avg_visits = [avg_visits_by_zoom[z] for z in zooms]
    
    bars = ax4a.bar(zooms, avg_visits, color='coral', edgecolor='darkred', alpha=0.7)
    ax4a.set_xlabel('Уровень зума', fontsize=12)
    ax4a.set_ylabel('Среднее количество визитов', fontsize=12)
    ax4a.set_title('Среднее количество визитов на тайл по зумам', fontsize=14, fontweight='bold')
    ax4a.grid(True, alpha=0.3, axis='y')
    
    # Добавляем значения на столбцы
    for bar, value in zip(bars, avg_visits):
        height = bar.get_height()
        ax4a.text(bar.get_x() + bar.get_width()/2., height + 0.1*max(avg_visits),
                 f'{value:.1f}', ha='center', va='bottom', fontsize=9)
    
    # График 2: Общее количество визитов по зумам
    total_visits_by_zoom = {}
    for zoom in sorted(visits_by_zoom.keys()):
        total_visits_by_zoom[zoom] = np.sum(visits_by_zoom[zoom])
    
    total_visits = [total_visits_by_zoom[z] for z in zooms]
    
    ax4b.bar(zooms, total_visits, color='lightgreen', edgecolor='darkgreen', alpha=0.7)
    ax4b.set_xlabel('Уровень зума', fontsize=12)
    ax4b.set_ylabel('Общее количество визитов', fontsize=12)
    ax4b.set_title('Общее количество визитов по зумам', fontsize=14, fontweight='bold')
    ax4b.set_yscale('log')  # Логарифмическая шкала для наглядности
    ax4b.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig('visits_by_zoom_2.png', dpi=150, bbox_inches='tight')
    
    # Выводим сводную статистику
    print("\n=== СВОДНАЯ СТАТИСТИКА ===")
    print(f"Всего тайлов в индексе: {len(tile_info):,}")
    print(f"Всего тайлов в статистике: {len(stats_info):,}")
    
    total_size_gb = sum(tile.size for tile in tile_info) / (1024**3)
    print(f"Общий размер всех тайлов: {total_size_gb:.2f} GB")
    
    print(f"\nГрафики сохранены как:")
    print("1. tile_sizes_all_zooms.png - общее распределение размеров")
    print("2. tile_sizes_by_zoom.png - распределение по зумам")
    print("3. tile_sizes_boxplot.png - сравнение распределений (box plot)")
    print("4. visits_by_zoom.png - статистика визитов по зумам")
    
    # Показываем все графики
    plt.show()