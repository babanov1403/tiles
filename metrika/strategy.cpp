#include "strategy.h"
#include "statistics.h"

#include <algorithm>
#include <set>
#include <cassert>
#include <random>
#include <ranges>

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
}

auto IStrategy::update_layout_smart_decompose(std::vector<IndexItem>& tiles, stats::Statistics* stats, double ratio, std::size_t min_visits) const {
    const std::size_t kInitSize = tiles.size();
    std::size_t init_offset = 0;
    for (auto item : tiles) {
        init_offset += item.size;
    }

    constexpr std::size_t kPageSize = 4 * 1024;
    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;


}

std::size_t GreedySectorStrategy::compute_min_visits(std::vector<IndexItem> tiles, stats::Statistics* stats) const {
    constexpr double kPercentile = 0.2;
    std::size_t percentile_pos = kPercentile * tiles.size();
    std::cout << "percentile_pos: " << percentile_pos << '\n';
    std::nth_element(tiles.begin(), tiles.begin() + percentile_pos, tiles.end(), [stats](const auto& lhs, const auto& rhs) {
        return stats->get_visits_for(lhs.x, lhs.y, lhs.z) < stats->get_visits_for(rhs.x, rhs.y, rhs.z);
    });
    auto [x, y, z, size, offset] = *(tiles.begin() + percentile_pos);

    return stats->get_visits_for(x, y, z);
}

auto IStrategy::update_layout_smart(std::vector<IndexItem>&& tiles, stats::Statistics* stats, double ratio, std::size_t min_visits) const {
    std::cout << "starting..\n";
    const std::size_t kInitSize = tiles.size();

    std::size_t init_offset = 0;

    for (const auto item : tiles) {
        init_offset += item.size;
    }

    constexpr std::size_t kPageSize = 4 * 1024;
    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;
    auto [boring_tiles, interesting_tiles] = split_by(std::move(tiles), [stats, min_visits](IndexItem item){
        return stats->get_visits_for(item.x, item.y, item.z) <= min_visits; 
    });

    std::cout << "min_visits: " << min_visits << '\n';
    auto p = *boring_tiles.begin();
    std::cout << stats->get_visits_for(p.x, p.y, p.z) << "<---------\n";

    std::cout << "Parts of array after splitting by visits:\n";
    std::cout << "boring: " << boring_tiles.size() * 1. / kInitSize << " | interesting: " << interesting_tiles.size() * 1. / kInitSize << std::endl;
    std::cout << "boring: " << boring_tiles.size() << " | interesting: " << interesting_tiles.size() << std::endl;
    // we have pretty nice result here with greedy approach
    std::ranges::sort(interesting_tiles, stats::StatsGreaterComparator(stats));

    // test
    // std::ranges::sort(boring_tiles, stats::StatsGreaterComparator(stats));
    std::vector<IndexItem> ram_tiles;
    std::vector<IndexItem> external_tiles;

    std::size_t size = 0;
    for (const auto item : interesting_tiles) {
        if (size > kRAMBound) {
            external_tiles.push_back(item);
            continue;
        }
        size += item.size;
        ram_tiles.push_back(item);
    }

    std::cout << "Tiles in RAM: " << ram_tiles.size() << ' ' << "Not in RAM: " << external_tiles.size() << '\n';

    {
        std::vector<IndexItem> empty;
        std::swap(empty, interesting_tiles);
    }

    // now split external_tiles on big_tiles and small ones
    auto [small_tiles, big_tiles] = split_by(std::move(external_tiles), [](IndexItem item){
        return item.size < kPageSize;
    });

    std::ranges::sort(small_tiles, std::greater<>{}, &IndexItem::size);
    std::ranges::sort(big_tiles, stats::StatsGreaterComparator(stats));
    // std::ranges::sort(big_tiles, std::less<>{}, &IndexItem::size);

    
    std::cout << "small: " << small_tiles.size() << " | big: " << big_tiles.size() << std::endl;
    // RAM | OTHER | POHUI
    // in future try RAM | WARM | COLD | POHUI

    std::size_t offset_gl = 0;
    for (auto& tile : ram_tiles) {
        tile.offset = offset_gl;
        offset_gl += tile.size;
    }

    std::vector<IndexItem> result;
    result.reserve(small_tiles.size() + big_tiles.size());
    offset_gl += (kPageSize - offset_gl % kPageSize) % kPageSize;

    std::size_t total_tiles_used_for_padding = 0;

    for (auto& tile : big_tiles) {
        tile.offset = offset_gl;
        offset_gl += tile.size;
        result.push_back(tile);
        if (offset_gl % kPageSize == 0) {
            continue;
        }

        auto space_until_page_end = kPageSize - offset_gl % kPageSize;

        std::size_t padding_tiles = 0;
        while (!small_tiles.empty() && small_tiles.back().size <= space_until_page_end) {
            padding_tiles++;

            small_tiles.back().offset = offset_gl;
            offset_gl += small_tiles.back().size;

            space_until_page_end -= small_tiles.back().size;            
            result.push_back(small_tiles.back());
            small_tiles.pop_back();
        }
        total_tiles_used_for_padding += padding_tiles;
        offset_gl += space_until_page_end;
    }
    std::cout << "\n";

    std::cout << "Total tiles used for padding: " << total_tiles_used_for_padding << '\n';

    for (auto& tile : small_tiles) {
        tile.offset = offset_gl;
        offset_gl += tile.size;
        result.push_back(tile);
    }

    std::cout << "We have " << small_tiles.size() << " left from filling empty spaces\n";
    std::size_t sz = 0;
    for (const auto small : small_tiles) {
        sz += small.size;
    }
    std::cout << "this will cause " << sz / 4. / 1024 << " more pages\n";

    // now we have | ram_tiles | result | boring_tiles |
    if (ram_tiles.size() + result.size() + boring_tiles.size() != kInitSize) {
        std::cout << "Tiles size mismatch:\n";
        std::cout << "expected: " << kInitSize << '\n';
        std::cout << "got: " << ram_tiles.size() + result.size() + boring_tiles.size() << '\n';
        exit(1);
    }

    // and now arrange boring_tiles
    for (auto& tile : boring_tiles) {
        tile.offset = offset_gl;
        offset_gl += tile.size;
    }

    // and concat all the tiles in one array
    // now all the tiles lays in ram_tiles, but not every of them is in ram...
    std::cout << ram_tiles.size() << '\n';
    for (auto tile : result) {
        ram_tiles.push_back(tile);
    }

    for (auto tile : boring_tiles) {
        ram_tiles.push_back(tile);
    }
    std::size_t prev = 0;
    std::size_t rofl = 0;
    for (auto [x, y, z, size, offset] : ram_tiles) {
        rofl++;
        if (prev > offset) {
            std::cout << prev << " " << offset << '\n';
            std::cout << "zhp[a]";
            std::cout << " " << rofl << '\n';
            exit(1);
        } 
        prev = offset;
    }

    std::cout << "We used additional " << (offset_gl - init_offset) / 1024. / 1024. << " megabytes :)\n";
    return ram_tiles;
}

PageHandle GreedySectorStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    auto& tiles = tile_info->get_items_mutable();
    auto min_visits = compute_min_visits(tiles, stats);
    tiles = update_layout_smart(std::move(tiles), stats, ratio, min_visits);
    // validate(before, tiles);
    return build_handler_from_tiles(tiles, stats, ratio);
}

// if pred(x) == true, it will be in small_tiles
template <class Pred>
std::pair<std::vector<libtiles::tileindex::IndexItem>, std::vector<libtiles::tileindex::IndexItem>>
IStrategy::split_by(std::vector<IndexItem>&& tiles, Pred pred) const {
    // [begin; middle) > 4 * 1024, [middle; end) <= 4 * 1024
    auto middle = std::partition(tiles.begin(), tiles.end(), pred);
    std::cout << "middle is: " << std::distance(tiles.begin(), middle) << '\n'; 
    std::vector<IndexItem> small_tiles(tiles.begin(), middle);
    std::vector<IndexItem> big_tiles(middle, tiles.end());
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
    const std::size_t kPageSize = 4 * 1024;
    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;
    const std::size_t kTilesCnt = tile_info->get_items().size();
    auto& tiles = tile_info->get_items_mutable();
    std::cout << kRAMBound << " " << kTilesCnt << '\n';
    
    std::vector<std::vector<std::size_t>> dp(kTilesCnt + 1, std::vector<std::size_t>(kRAMBound + 1, 0));

    for (std::size_t k = 1; k <= kTilesCnt; k++) {
        for (std::size_t s = 1; s <= kRAMBound; s++) {
            if (s >= tiles[k].size) {
                dp[k][s] = std::max(dp[k - 1][s], dp[k - 1][s - tiles[k].size] + stats->get_visits_for(tiles[k].x, tiles[k].y, tiles[k].z) * (tiles[k].size + kPageSize - 1) / kPageSize);     
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

PageHandle AlignStrategy::build_handler(
    stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    auto& tiles = tile_info->get_items_mutable();
    std::ranges::sort(tiles, stats::StatsGreaterComparator(stats));
    constexpr std::size_t kPageSize = 4 * 1024;
    std::size_t offset_init = 0;
    for (auto tile : tiles) {
        offset_init += tile.size;
    }

    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;
    std::vector<IndexItem> ram_tiles;
    std::vector<IndexItem> external_tiles;

    std::size_t size = 0;
    for (const auto item : tiles) {
        if (size > kRAMBound) {
            external_tiles.push_back(item);
            continue;
        }
        size += item.size;
        ram_tiles.push_back(item);
    }


    std::size_t offset_gl = 0;
    for (auto& tile : ram_tiles) {
        tile.offset = offset_gl;
        offset_gl += tile.size;
    }

    auto new_tiles = std::move(ram_tiles);
    for (std::size_t idx = 0; idx + 1 < external_tiles.size(); idx++) {
        auto& [x, y, z, size, offset] = external_tiles[idx];
        auto& [x_p, y_p, z_p, size_p, offset_p] = external_tiles[idx + 1];
        offset = offset_gl;
        offset_gl += size;
        new_tiles.emplace_back(external_tiles[idx]);
        if (offset_gl % kPageSize == 0) {
            continue;
        }

        std::size_t remainder = kPageSize - offset_gl % kPageSize;
        if (remainder < size_p) {
            offset_gl += remainder;
        }
    }

    new_tiles.emplace_back(external_tiles.back());

    std::cout << "We used additional " << (offset_gl - offset_init) / 1024. / 1024 << "mb\n";
    // validate(tiles, new_tiles);
    tiles = std::move(new_tiles);

    return build_handler_from_tiles(tiles, stats, ratio);
}

PageHandle MetricKnapsackStrategy::build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const {
    const std::size_t kPageSize = 4 * 1024;
    const std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024 * ratio;
    const std::size_t kTilesCnt = tile_info->get_items().size();
    constexpr std::size_t kRPSBound = 50'000;
    const auto& tiles = tile_info->get_items();
    std::cout << kRAMBound << " " << kTilesCnt << '\n';

    auto get_ideal_metric_proportion = [stats, kPageSize](IndexItem item) -> std::size_t {
        std::size_t uncached_bytes = (item.size + kPageSize - 1) / kPageSize;
        return stats->get_visits_for(item.x, item.y, item.z) * uncached_bytes * kPageSize;
    };
    
    std::vector<std::size_t> curr_dp(kRAMBound + 1, std::numeric_limits<std::size_t>::max());
    std::vector<std::size_t> prev_dp(kRAMBound + 1, std::numeric_limits<std::size_t>::max());

    for (std::size_t k = 1; k <= kTilesCnt; k++) {
        for (std::size_t s = 1; s <= kRAMBound; s++) {
            if (s >= tiles[k].size) {
                curr_dp[s] = std::min(prev_dp[s] + get_ideal_metric_proportion(tiles[k]), prev_dp[s - tiles[k].size]);     
            } else {
                curr_dp[s] = prev_dp[s] + get_ideal_metric_proportion(tiles[k]);
            }
        }
        std::swap(curr_dp, prev_dp);
    }

    std::size_t total_visits = stats->get_total_visits();
    double raw_metr = prev_dp.back() * 1. / total_visits; // average bytes
    raw_metr /= 1024 * 1024; 
    raw_metr *= kRPSBound;

    std::cout << raw_metr << '\n';

    return PageHandle{};
}
