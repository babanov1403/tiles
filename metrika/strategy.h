#pragma once

#include "libtiles/tileindex/tileindex.h"
#include "page_handle.h"
#include "statistics.h"

#include <algorithm>
#include <ranges>

class IStrategy {
public:
    virtual PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const = 0;

protected:
    using IndexItem = libtiles::tileindex::IndexItem;
    virtual PageHandle build_handler_from_tiles(const std::vector<IndexItem>& tiles, double ratio) const;
    virtual PageHandle build_handler_from_tiles(const std::vector<IndexItem>& tiles, stats::Statistics* stats, double ratio) const;
    
    // @brief updates layout \sum stats[page] -> max, or at least tries to
    void update_layout(std::vector<IndexItem>& tiles) const;

    auto update_layout_smart(std::vector<IndexItem>&& tiles, stats::Statistics*, double, std::size_t) const;
    auto update_layout_smart_decompose(std::vector<IndexItem>&, stats::Statistics*, double, std::size_t) const;
    
    // @brief util function, splits array on two by pred
    template <class Pred>
    std::pair<std::vector<libtiles::tileindex::IndexItem>, std::vector<libtiles::tileindex::IndexItem>>
    split_by(std::vector<IndexItem>&&, Pred) const;


    static bool validate(std::vector<IndexItem> before, std::vector<IndexItem> after) {
        std::cout << "===============STARTING VALIDATION...===============\n";
        std::ranges::sort(before, [](const auto& lhs, const auto& rhs) {
            return std::tuple(lhs.x, lhs.y, lhs.z, lhs.size) < std::tuple(rhs.x, rhs.y, rhs.z, rhs.size);
        });

        std::ranges::sort(after, [](const auto& lhs, const auto& rhs) {
            return std::tuple(lhs.x, lhs.y, lhs.z, lhs.size) < std::tuple(rhs.x, rhs.y, rhs.z, rhs.size);
        });

        for (auto [lhs, rhs] : std::ranges::views::zip(before, after)) {
            if (std::tuple(lhs.x, lhs.y, lhs.z, lhs.size) != std::tuple(rhs.x, rhs.y, rhs.z, rhs.size)) {
                std::cout << lhs.x << " " << lhs.y << " " << lhs.z << " " << lhs.size << '\n';
                std::cout << rhs.x << " " << rhs.y << " " << rhs.z << " " << rhs.size << '\n';
                std::cout << "=-=-=-=-=-=-=-=-= ALL BAD CAPTAIN! =-=-=-=-=-=-=-=-=\n";
                exit(1);
                return false;
            }
        }
        std::cout << "=================VALIDATION PASSED!=================\n";

        return true;
    }

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
// max capacity kRAMBound, we want max sum of visits * pages, each tile costs tile.size
class KnapsackStrategy : public IStrategy {
public:
    KnapsackStrategy() = default;

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};

class GreedySectorStrategy : public IStrategy {
public:
    GreedySectorStrategy() = default;

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
    
    std::size_t compute_min_visits(std::vector<IndexItem>, stats::Statistics*) const;
};

class AlignStrategy : public IStrategy {
public:
    AlignStrategy() = default;

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};

class MetricKnapsackStrategy : public IStrategy {
public:
    MetricKnapsackStrategy() = default;

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};

class RofloStrategy : public IStrategy {
public:
    RofloStrategy() = default;

    PageHandle build_handler(
        stats::Statistics* stats, stats::TileHandle* tile_info, double ratio) const override;
};