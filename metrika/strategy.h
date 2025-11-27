#pragma once

#include "page_handle.h"
#include "statistics.h"

#include <algorithm>

class IStrategy {
public:
    virtual PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info) const = 0;

protected:
    virtual PageHandle build_handler_from_tiles(const std::vector<libtiles::tileindex::IndexItem>& tiles) const {
        PageHandle handler;
        for (const auto& item : tiles) {
            auto offset = item.offset;
            handler.include_page(offset);
        }

        return handler;
    }

    virtual ~IStrategy() = default;
};

constexpr std::size_t kMaxTilesInMemory = 1000;

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy live (no)
class GreedyStrategy : public IStrategy {
public:
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info) const override {
        auto stats_comparator = [&stats](const auto& lhs, const auto& rhs) {
            stats->get_visits_for(lhs.x, lhs.y, lhs.z) < stats->get_visits_for(rhs.x, rhs.y, rhs.z);
        };
        auto top_tiles = tile_info->get_topk_by(1000, stats_comparator);
        return build_handler_from_tiles(top_tiles);
    }
};

// @brief
// bruteforce until we find the best solution
// based on Metrika impl
class BruteForceStrategy : public IStrategy {
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info) const override {
        return PageHandle{};
    }
};