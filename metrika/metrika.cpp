#include "metrika.h"
#include "constants.h"

#include <ranges>

namespace metrika {

constexpr std::size_t kAlignment = 1024;
constexpr std::size_t kPageSize = 4 * kKilobyte;
// change it for a good lord
constexpr std::size_t kTileSize = 2 * kMegabyte + 561;

std::size_t align_offset_by_page(std::size_t offset) {
    return offset - offset % kAlignment;
}


std::size_t get_size(std::size_t offset) {
    return kTileSize;
}

// \sum stats[i] * (\sum is_cached[page]) * page_sz) -> max (E)
// сколько в среднем не будет лежать в кэше при запросе :)
/*
*  offsets - vector of offsets, just info about where is tile layed
*  statistics - for every tile we know how popular he is
*  is_cached - vector[addr] - is memory from addr to addr + kPageSize is being in cache
*/
double compute_metrics(const std::vector<std::size_t>& offsets,
                       const std::vector<double>& statistics,
                       const std::vector<bool>& is_cached) {
    double expected_fair_lookup_bytes = 0;
    for (auto [offset, statistic] : std::ranges::views::zip(offsets, statistics)) {
        std::size_t page_addr = align_offset_by_page(offset);
        auto tile_size = get_size();
        std::size_t last_page_addr = page_addr + tile_size;

        std::size_t uncached_bytes = 0;
        for (;page_addr < last_page_addr; page_addr += kPageSize) {
            uncached_bytes += !is_cached[page_addr] * kPageSize; 
        }
        expected_fair_lookup_bytes += statistic * uncached_bytes;
    }
    return expected_fair_lookup_bytes;
}

}