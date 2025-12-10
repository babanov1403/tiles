#include "strategy.h"
#include "statistics.h"

#include <algorithm>
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

void IStrategy::update_layout(std::vector<IndexItem>& tiles) const {
    auto [small_tiles, big_tiles] = split_by(tiles, [](const IndexItem& item){
        return item.size > PageHandle{}.get_page_size();
    });

    // implement algo... fuck

    // std::vector<IndexItem> new_tiles;
    // new_tiles.reserve(tiles.size());
}


template <class Pred>
std::pair<std::vector<libtiles::tileindex::IndexItem>, std::vector<libtiles::tileindex::IndexItem>>
IStrategy::split_by(const std::vector<IndexItem>& tiles, Pred pred) const {
    std::vector<IndexItem> small_tiles;
    std::vector<IndexItem> big_tiles = tiles;
    // [begin; middle) > 4 * 1024, [middle; end) <= 4 * 1024
    auto middle = std::partition(big_tiles.begin(), big_tiles.end(), pred);
    auto iter = big_tiles.end() - 1;
    while (iter != middle) {
        small_tiles.push_back(*iter);
        big_tiles.pop_back();
        --iter;
    }
    small_tiles.push_back(*iter);
    big_tiles.pop_back();

    return {small_tiles, big_tiles};
}

PageHandle IStrategy::build_handler_from_tiles(const std::vector<IndexItem>& tiles, stats::Statistics* stats, double ratio) const {
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
}

PageHandle RandomStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    std::mt19937 gen(0xdeadbeef);
    auto& tiles = tile_info->get_items_mutable();
    std::ranges::shuffle(tiles, gen);
    update_layout(tiles);
    return build_handler_from_tiles(tiles, stats, ratio);
}

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy life (no)
PageHandle GreedyStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    auto& tiles = tile_info->get_items_mutable();
    std::ranges::sort(tiles, stats::StatsGreaterComparator(stats));
    update_layout(tiles);
    return build_handler_from_tiles(tiles, stats, ratio);
}

PageHandle KnapsackStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;
    const std::size_t kTilesCnt = tile_info->get_items().size();
    auto& tiles = tile_info->get_items_mutable();
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
    std::unordered_set<std::size_t> top_tiles_indexes;

    std::size_t k = kTilesCnt;
    std::size_t s = kRAMBound;

    while (true) {
        if (dp[k][s] == 0) {
            break;
        }

        if (dp[k][s] == dp[k - 1][s]) {
            k -= 1;
        } else {
            top_tiles_indexes.insert(k);
            top_tiles.push_back(tiles[k]);
            s -= tiles[k].size;
            k -= 1;
        }
    }

    // arrange all top_tiles_indexes together
    std::size_t split = 0;
    while (!top_tiles_indexes.empty()) {
        if (top_tiles_indexes.contains(split)) {
            top_tiles_indexes.erase(split);
            split++;
            continue;
        }
        auto next = *top_tiles_indexes.begin();
        top_tiles_indexes.erase(next);
        std::swap(tiles[next], tiles[split]);
        split++;
    }

    // update_layout(tiles);

    std::ranges::sort(top_tiles, [stats_ = stats](const auto& lhs, const auto& rhs) {
        return stats_->get_visits_for(lhs.x, lhs.y, lhs.z) > stats_->get_visits_for(rhs.x, rhs.y, rhs.z);
    });

    return build_handler_from_tiles(top_tiles, stats, ratio);
}

PageHandle GreedyScaledStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    
    auto& tiles = tile_info->get_items_mutable();
    std::ranges::sort(tiles, stats::StatsGreaterScaledComparator(stats));
    update_layout(tiles);

    return build_handler_from_tiles(tiles, stats, ratio);
}

