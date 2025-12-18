import struct
from dataclasses import dataclass
from typing import List, BinaryIO

import tqdm
import numpy as np

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
    cnt = 0
    tile_info = []
    bar = tqdm.tqdm(total=69875051, desc="Processing tiles")
    for item in tqdm.tqdm(parse_index_file_generator(index_path)):
        tile_info.append(item)
        bar.update(1)
        cnt += 1
        if cnt == 69875051:
            break
    
    stats_info = []
    bar = tqdm.tqdm(total=31703414, desc="Processing stats")
    for item in tqdm.tqdm(parse_stats_file_generator(stats_path)):
        stats_info.append(item)
        bar.update(1)

    stats_info = np.array([stats_info])
    tile_info = np.array([tile_info])



    """
    посчитать статистики
    1) распределение тайлов по размеру в зависимости от зума
    2) сколько в среднем на тайл визитов в зависимости от зума
    """
    
   