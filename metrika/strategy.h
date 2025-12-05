#pragma once

#include "libtiles/tileindex/tileindex.h"
#include "page_handle.h"
#include "statistics.h"

class IStrategy {
public:
    virtual PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const = 0;

protected:
    using IndexItem = libtiles::tileindex::IndexItem;
    virtual PageHandle build_handler_from_tiles(const std::vector<IndexItem>& tiles, double ratio) const;
    virtual ~IStrategy() = default;
};

class RandomStrategy : public IStrategy {
public:
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy live (no)
class GreedyStrategy : public IStrategy {
public:
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};

// @brief
// another naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats / size, and live happy live (no)
class GreedyScaledStrategy : public IStrategy {
public:
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};

// @brief
// max capacity kRAMBound, we want max sum of visits, each tile costs tile.size
class KnapsackStrategy : public IStrategy {
public:
    KnapsackStrategy() = default;

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};