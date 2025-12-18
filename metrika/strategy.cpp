#include "strategy.h"
#include "statistics.h"

#include <algorithm>
#include <set>
#include <cassert>
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
            if (handler.check_memory_exceed()) {
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
    std::size_t offset_gl = 0;
    for (auto& [x, y, z, size, offset] : tiles) {
        offset = offset_gl;
        offset_gl += size;
    }
    // auto [small_tiles, big_tiles] = split_by(tiles, [](const IndexItem& item){
    //     return item.size > PageHandle{}.get_page_size();
    // });

    // tiles = 

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
        if (tiles_counter == 225) {
            // break;
        }
        auto start = handler.align(item.offset);
        // std::cout << "tiles included: " << tiles_counter << '\n';
        // std::cout << "tile on offset: " << start << " memory left: " << 16 * 1024 * 1024 * 1024. * ratio - handler.page_count() * 4096 << '\n';
        for (auto offset = start; offset < start + item.size; offset += handler.get_page_size()) {
            bool was = handler.is_prioritized(offset);
            handler.include_page(offset);
            if (handler.check_memory_exceed()) {
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
    // std::cout << "=-=-=-=-==-=-=-=-=-\n";
    // for (auto tile : tile_info->get_first(10)) {
        // std::cout << tile.x << " " << tile.y << " " << tile.z << " " << tile.size << " " << tile.offset << '\n';
    // }
    // std::cout << "=-=-=-=-==-=-=-=-=-\n";
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


    std::cout << "Using dp we packed " << dp.back().back() << " visits and ";

    std::set<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint64_t, std::uint64_t>> top_tiles;

    std::size_t k = kTilesCnt;
    std::size_t s = kRAMBound;

    while (dp[k][s] != 0) {
        if (dp[k][s] == dp[k - 1][s]) {
            k -= 1;
        } else {
            top_tiles.emplace(tiles[k].x, tiles[k].y, tiles[k].z, tiles[k].size, tiles[k].offset);
            s -= tiles[k].size;
            k -= 1;
        }
    }
    std::cout << top_tiles.size() << " tiles!\n";
    std::size_t result_v = 0;
    std::size_t result_s = 0;

    for (auto tile : top_tiles) {
        result_v += stats->get_visits_for(std::get<0>(tile), std::get<1>(tile), std::get<2>(tile));
        result_s += std::get<3>(tile);
    }

    // std::cout << dp.back().back() << " vs " << result_v << '\n';
    // assert(expr)
    // std::cout << result_s << '\n';

    auto middle = std::partition(tiles.begin(), tiles.end(), [&top_tiles](const IndexItem& item) {
        return top_tiles.contains(std::make_tuple(item.x, item.y, item.z, item.size, item.offset));
    });

    // std::cout << "Index of split is: " << split << '\n';

    std::ranges::sort(tiles.begin(),middle, [stats_ = stats](const auto& lhs, const auto& rhs) {
        return stats_->get_visits_for(lhs.x, lhs.y, lhs.z) > stats_->get_visits_for(rhs.x, rhs.y, rhs.z);
    });

    update_layout(tiles);
    std::size_t size_after_sort = 0;
    // std::cout << "=-=-=-=-==-=-=-=-=-\n";
    // for (auto tile : tile_info->get_first(224)) {
        // std::cout << tile.x << " " << tile.y << " " << tile.z << " " << tile.size << " " << tile.offset << '\n';
        // size_after_sort += stats->get_visits_for(tile.x, tile.y, tile.z);
    // }
    // std::cout << size_after_sort << ' ' << dp.back().back() << '\n';
    // std::cout << "=-=-=-=-==-=-=-=-=-\n";
    return build_handler_from_tiles(tiles, stats, ratio);
}

PageHandle GreedyScaledStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    
    auto& tiles = tile_info->get_items_mutable();
    std::ranges::sort(tiles, stats::StatsGreaterScaledComparator(stats));
    update_layout(tiles);

    return build_handler_from_tiles(tiles, stats, ratio);
}

PageHandle RofloStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    
    auto& tiles = tile_info->get_items_mutable();
    std::ranges::sort(tiles, [stats](const IndexItem& lhs, const IndexItem& rhs) {
        return lhs.size > rhs.size;
    });
    update_layout(tiles);

    return build_handler_from_tiles(tiles, stats, ratio);
}
