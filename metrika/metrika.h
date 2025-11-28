#pragma once

#include "statistics.h"
#include "page_handle.h"

// \sum stats[i] * (\sum !is_cached[page]) * page_sz) -> min (E)
// сколько в среднем не будет лежать в кэше при запросе
class Metrika {
public:
    Metrika() = delete;
    Metrika(const Metrika&) = delete;
    Metrika(Metrika&&) = delete;
    Metrika(stats::Statistics* stats, stats::TileInfo* tile_info, PageHandle* handler);

    double compute() const;
private:
    stats::Statistics* stats_;
    stats::TileInfo* tile_info_;
    PageHandle* handler_;
};