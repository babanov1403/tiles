#include "strategy.h"
#include "statistics.h"

#include <random>

PageHandle IStrategy::build_handler_from_tiles(const std::vector<IndexItem>& tiles, double ratio) const {
    PageHandle handler;
    handler.set_ratio(ratio);
    std::size_t tiles_counter = 0;
    std::size_t pages_counter = 0;
    for (const auto& item : tiles) {
        auto start = handler.align(item.offset);
        for (auto offset = start; offset <= start + item.size; offset += handler.get_page_size()) {
            if (!handler.include_page(offset)) {
                std::cout << "Total tiles included is " << tiles_counter << '\n';
                std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
                return handler;
            }
            pages_counter++;
        }
        tiles_counter++;
    }
    std::cout << "Total tiles included is " << tiles_counter << '\n';
    std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
    return handler;
}

PageHandle RandomStrategy::build_handler(
    stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const {
    std::vector<IndexItem> tiles;
    std::mt19937 gen(0xdeadbeef);
    std::uniform_int_distribution uid(0, 6);
    
    std::size_t total_size_bytes = 0;

    for (const auto& tile : tile_info->get_items()) {
        if (uid(gen) == 1) {
            tiles.push_back(tile);
            total_size_bytes += tile.size;
        }

        // @babanov1403 TODO: leave it? fix it?
        if (total_size_bytes > 16ull * 1024 * 1024 * 1024) {
            break;
        }
    }

    return build_handler_from_tiles(tiles, ratio);
}

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy live (no)
PageHandle GreedyStrategy::build_handler(
    stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const {
    const auto& top_tiles = tile_info->get_sorted(stats::StatsLessComparator(stats));
    std::cout << "tiles size is: " << top_tiles.size() << '\n';
    return build_handler_from_tiles(top_tiles, ratio);
}

// @brief
// bruteforce until we find the best solution
// based on Metrika
BruteForceStrategy::BruteForceStrategy(Metrika* metrika) : metrika_(metrika) {}

PageHandle BruteForceStrategy::build_handler(
    stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const {
    return PageHandle{};
}