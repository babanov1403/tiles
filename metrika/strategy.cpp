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
            bool was = handler.is_prioritized(offset);
            if (!handler.include_page(offset)) {
                std::cout << "Total tiles included is " << tiles_counter << '\n';
                std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
                return handler;
            }
            if (!was) {
                pages_counter++;
            }
        }
        tiles_counter++;
    }
    std::cout << "Total tiles included is " << tiles_counter << '\n';
    std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
    return handler;
}

PageHandle RandomStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    std::vector<IndexItem> tiles;
    std::mt19937 gen(0xdeadbeef);
    std::uniform_int_distribution uid(0, 6);
    
    std::size_t total_size_bytes = 0;
    std::size_t tota_visits = 0;

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

    PageHandle handler;
    handler.set_ratio(ratio);
    std::size_t tiles_counter = 0;
    std::size_t pages_counter = 0;
    std::size_t visits_counter = 0;
    for (const auto& item : tiles) {
        auto start = handler.align(item.offset);
        for (auto offset = start; offset <= start + item.size; offset += handler.get_page_size()) {
            bool was = handler.is_prioritized(offset);
            if (!handler.include_page(offset)) {
                std::cout << "Total tiles included is " << tiles_counter << '\n';
                std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
                std::cout << "Total visits among tiles is " << visits_counter << '\n';
                return handler;
            }
            if (!was) {
                pages_counter++;
            }
        }
        tiles_counter++;
        visits_counter += stats->get_visits_for(item.x, item.y, item.z);
    }
    std::cout << "Total tiles included is " << tiles_counter << '\n';
    std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
    std::cout << "Total visits among tiles is " << visits_counter << '\n';
    return handler;

    // return build_handler_from_tiles(tiles, ratio);
}

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy live (no)
PageHandle GreedyStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    const auto& top_tiles = tile_info->get_sorted(stats::StatsLessComparator(stats));
    PageHandle handler;
    handler.set_ratio(ratio);
    std::size_t tiles_counter = 0;
    std::size_t pages_counter = 0;
    std::size_t visits_counter = 0;
    for (const auto& item : top_tiles) {
        auto start = handler.align(item.offset);
        for (auto offset = start; offset <= start + item.size; offset += handler.get_page_size()) {
            bool was = handler.is_prioritized(offset);
            if (!handler.include_page(offset)) {
                std::cout << "Total tiles included is " << tiles_counter << '\n';
                std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
                std::cout << "Total visits among tiles is " << visits_counter << '\n';
                return handler;
            }
            if (!was) {
                pages_counter++;
            }
        }
        tiles_counter++;
        visits_counter += stats->get_visits_for(item.x, item.y, item.z);
    }
    std::cout << "Total tiles included is " << tiles_counter << '\n';
    std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
    std::cout << "Total visits among tiles is " << visits_counter << '\n';
    return handler;
    // return build_handler_from_tiles(top_tiles, ratio);
}

PageHandle KnapsackStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;
    const std::size_t kTilesCnt = tile_info->get_items().size();
    const auto& tiles = tile_info->get_items();
    std::cout << kRAMBound << " " << kTilesCnt << '\n';
    
    std::vector<std::vector<std::size_t>> dp(kTilesCnt + 1, std::vector<std::size_t>(kRAMBound + 1, 0));

    for (std::size_t k = 1; k <= kTilesCnt; k++) {
        for (std::size_t s = 1; s <= kRAMBound; s++) {
            if (s >= tiles[k].size) {
                dp[k][s] = std::max(dp[k - 1][s], dp[k - 1][s - tiles[k].size] + stats->get_visits_for(tiles[k].x, tiles[k].y, tiles[k].z));     
            } else {
                dp[k][s] = dp[k - 1][s];
            }
        }
    }


    std::cout << "Using dp we packed " << dp.back().back() << '\n';
    std::cout << "Building what tiles we need to include...\n";

    std::vector<IndexItem> top_tiles;

    std::size_t k = kTilesCnt;
    std::size_t s = kRAMBound;

    while (true) {
        if (dp[k][s] == 0) {
            break;
        }

        if (dp[k][s] == dp[k - 1][s]) {
            k -= 1;
        } else {
            top_tiles.push_back(tiles[k]);
            k -= 1;
            s -= tiles[k].size;
        }
    }
    
    PageHandle handler;
    handler.set_ratio(ratio);
    std::size_t tiles_counter = 0;
    std::size_t pages_counter = 0;
    std::size_t visits_counter = 0;
    for (const auto& item : top_tiles) {
        auto start = handler.align(item.offset);
        for (auto offset = start; offset <= start + item.size; offset += handler.get_page_size()) {
            bool was = handler.is_prioritized(offset);
            if (!handler.include_page(offset)) {
                std::cout << "Total tiles included is " << tiles_counter << '\n';
                std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
                std::cout << "Total visits among tiles is " << visits_counter << '\n';
                return handler;
            }
            if (!was) {
                pages_counter++;
            }
        }
        tiles_counter++;
        visits_counter += stats->get_visits_for(item.x, item.y, item.z);
    }
    std::cout << "Total tiles included is " << tiles_counter << '\n';
    std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
    std::cout << "Total visits among tiles is " << visits_counter << '\n';
    return handler;
    // return build_handler_from_tiles(top_tiles, ratio);
}

PageHandle GreedyScaledStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    const auto& top_tiles = tile_info->get_sorted(stats::StatsLessScaledComparator(stats));
    PageHandle handler;
    handler.set_ratio(ratio);
    std::size_t tiles_counter = 0;
    std::size_t pages_counter = 0;
    std::size_t visits_counter = 0;
    for (const auto& item : top_tiles) {
        auto start = handler.align(item.offset);
        for (auto offset = start; offset <= start + item.size; offset += handler.get_page_size()) {
            bool was = handler.is_prioritized(offset);
            if (!handler.include_page(offset)) {
                std::cout << "Total tiles included is " << tiles_counter << '\n';
                std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
                std::cout << "Total visits among tiles is " << visits_counter << '\n';
                return handler;
            }
            if (!was) {
                pages_counter++;
            }
        }
        tiles_counter++;
        visits_counter += stats->get_visits_for(item.x, item.y, item.z);
    }
    std::cout << "Total tiles included is " << tiles_counter << '\n';
    std::cout << "Total bytes included is " << pages_counter * 4 * 1024 << '\n';
    std::cout << "Total visits among tiles is " << visits_counter << '\n';
    return handler;
    // return build_handler_from_tiles(top_tiles, ratio);
}