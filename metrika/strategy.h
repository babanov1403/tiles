#pragma once

#include "libtiles/tileindex/tileindex.h"
#include "metrika/metrika.h"

#include <bitset>
#include <cassert>

class IStrategy {
public:
    virtual PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const = 0;

protected:
    using IndexItem = libtiles::tileindex::IndexItem;
    virtual PageHandle build_handler_from_tiles(const std::vector<IndexItem>& tiles, double ratio) const;
    virtual ~IStrategy() = default;
};

class RandomStrategy : public IStrategy {
public:
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const override;
};

// @brief
// basic naive approach - we want to put in cache all top-k tiles (by data)
// so we will sort tiles by stats, and live happy live (no)
class GreedyStrategy : public IStrategy {
public:
    PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const override;
};

// @brief
// bruteforce until we find the best solution
// based on Metrika impl
class BruteForceStrategy : public IStrategy {
private:
    template <std::size_t MaxPages>
    class PageCombinator {
    public:
        PageCombinator() = delete;
        explicit PageCombinator(std::vector<std::size_t> pages) {}

        std::optional<std::vector<std::size_t>> get_next_pages() {

        }
    private:
        std::bitset<MaxPages> variants_;
    };

public:
    explicit BruteForceStrategy(Metrika* metrika);

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileInfo* tile_info, double ratio) const override;

private:
    Metrika* metrika_;
};