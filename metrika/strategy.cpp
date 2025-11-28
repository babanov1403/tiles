#include "strategy.h"
#include "statistics.h"

#include <random>

PageHandle IStrategy::build_handler_from_tiles(const std::vector<IndexItem>& tiles) const {
    PageHandle handler;
    for (const auto& item : tiles) {
        auto start = handler.align(item.offset);
        // and then include all pages [align(offset); align(offset + size)]
        for (auto offset = start; offset <= start + item.size; offset += handler.get_page_size()) {
            handler.include_page(offset);
        }
    }

    return handler;
}

PageHandle RandomStrategy::build_handler(
    stats::Statistics* stats, stats::TileInfo* tile_info) const {
    std::vector<IndexItem> tiles;
    tiles.reserve(kMaxTilesInMemory);

    std::mt19937 gen(0xdeadbeef);
    std::uniform_int_distribution uid(0, 5);
    
    for (const auto& tile : tile_info->get_items()) {
        if (uid(gen) == 1) {
            tiles.push_back(tile);
        }

        if (tiles.size() == kMaxTilesInMemory) {
            break;
        }
    }

    assert(tiles.size() == kMaxTilesInMemory);

    return build_handler_from_tiles(tiles);
}

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy live (no)
PageHandle GreedyStrategy::build_handler(
    stats::Statistics* stats, stats::TileInfo* tile_info) const {
    auto top_tiles = tile_info->get_topk_by(kMaxTilesInMemory, stats::StatsLessComparator(stats));
    return build_handler_from_tiles(top_tiles);
}

// @brief
// bruteforce until we find the best solution
// based on Metrika impl
BruteForceStrategy::BruteForceStrategy(Metrika* metrika) : metrika_(metrika) {}

PageHandle BruteForceStrategy::build_handler(
    stats::Statistics* stats, stats::TileInfo* tile_info) const {
    return PageHandle{};
}