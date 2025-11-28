#include "metrika.h"

Metrika::Metrika(stats::Statistics* stats, stats::TileInfo* tile_info, PageHandle* handler)
    : stats_(stats), tile_info_(tile_info), handler_(handler) {}

double Metrika::compute() const {
    std::size_t expected_fair_lookup_bytes = 0;
    for (auto [x, y, z, size, offset] : tile_info_->get_items()) {
        std::size_t last_page_addr = offset + size;
        

        std::size_t uncached_bytes = 0;
        for (std::size_t page_addr = handler_->align(offset);page_addr < last_page_addr; page_addr = handler_->get_next_page(page_addr)) {
            uncached_bytes += !handler_->is_prioritized(page_addr) * handler_->get_page_size();
        }
        expected_fair_lookup_bytes += stats_->get_visits_for(x, y, z) * uncached_bytes;
    }

    std::size_t total_visits = stats_->get_total_visits();

    return expected_fair_lookup_bytes * 1. / total_visits;
}