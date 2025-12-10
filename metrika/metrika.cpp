#include "metrika.h"

Metrika::Metrika(stats::Statistics* stats, stats::TileHandle* tile_info, PageHandle* handler)
    : stats_(stats), tile_info_(tile_info), handler_(handler) {}

double Metrika::compute() const {
    std::size_t expected_fair_lookup_bytes = 0;
    for (auto [x, y, z, size, offset] : tile_info_->get_items()) {
        std::size_t last_page_addr = offset + size;
        std::size_t uncached_bytes = 0;
        for (std::size_t page_addr = handler_->align(offset);page_addr < last_page_addr; page_addr = handler_->get_next_page(page_addr)) {
            uncached_bytes += !handler_->is_prioritized(page_addr);
        }
        expected_fair_lookup_bytes += stats_->get_visits_for(x, y, z) * uncached_bytes;
    }
    expected_fair_lookup_bytes *= handler_->get_page_size();

    std::size_t total_visits = stats_->get_total_visits();

    // std::cout << expected_fair_lookup_bytes << "<-----\n";

    double raw_metr = expected_fair_lookup_bytes * 1. / total_visits; // average bytes
    // average megabytes, if we have this speed we process each
    // request in one sec on average
    raw_metr /= 1024 * 1024; 
    // we this speed we can process 50'000 requests on average
    raw_metr *= kRPSBound; 
    return raw_metr;
}

double Metrika::compute_sum_among_pages() const {
    std::size_t stats_sum = 0;
    for (auto [x, y, z, size, offset] : tile_info_->get_items()) {
        std::size_t last_page_addr = offset + size;
        std::size_t page_cnt = 0;
        for (std::size_t page_addr = handler_->align(offset);page_addr < last_page_addr; page_addr = handler_->get_next_page(page_addr)) {
            page_cnt += handler_->is_prioritized(page_addr);
        }
        stats_sum += stats_->get_visits_for(x, y, z) * page_cnt;
    }

    stats_sum /= handler_->get_page_size();
    
    return stats_sum;
}