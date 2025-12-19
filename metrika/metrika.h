#pragma once

#include "statistics.h"
#include "page_handle.h"

/*  
    \sum stats[i] * (\sum !is_cached[page]) * page_sz) -> min (E)
    Computes how many MB/s we need to process kRPSBound requests per second on average.
    Same as how many bytes we need to read from disk on average.
*/
class Metrika {
    static constexpr std::size_t kRPSBound = 50'000;
public:
    Metrika() = delete;
    Metrika(const Metrika&) = delete;
    Metrika(Metrika&&) = delete;
    Metrika(stats::Statistics* stats, stats::TileHandle* tile_info, PageHandle* handler);

    double compute() const;
    double compute_sum_among_pages() const;
    double compute_unpaged() const;
    double compute_ideal() const;
private:
    stats::Statistics* stats_;
    stats::TileHandle* tile_info_;
    PageHandle* handler_;
};