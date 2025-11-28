#pragma once

#include "libtiles/tileindex/tileindex.h"
 
#include <cstdint>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>

template<>
struct std::hash<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> {
    using Tuple = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;
    static constexpr std::size_t kMod = 1e9 + 33; 
    static constexpr std::size_t kMult = 11587;

    std::size_t operator()(const Tuple& tuple) const {
        const auto& [x, y, z] = tuple;
        return ((z * kMult + y) * kMult + x) % kMod; 
    }
};

namespace stats {

using libtiles::tileindex::IndexItem;
using libtiles::tileindex::readIndexItems;
using Tuple = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;

class Statistics;

struct StatsLessComparator {
    StatsLessComparator() = delete;
    explicit StatsLessComparator(stats::Statistics* stats);

    bool operator()(const libtiles::tileindex::IndexItem& lhs, const libtiles::tileindex::IndexItem& rhs) const;

private:
    stats::Statistics* stats_;
};

class LogParser {
    struct ParseResult {
        std::uint32_t x;
        std::uint32_t y;
        std::uint32_t z;
        std::size_t visits;
    };

public:
    explicit LogParser(std::filesystem::path path);

    // @babanov1403 TODO: this is too slow to parse line by line file
    // do smth with it
    std::optional<ParseResult> parse_next_line();

private:
    std::ifstream stream_;
}; 

class Statistics {
public:
    Statistics() = default;
    Statistics(const Statistics&) = delete;
    Statistics(Statistics&&) = delete;

    // @brief
    // if [x,y,z] not present in map, it will make new key with value visits
    // else it will add value to existing element
    void fill_from(std::filesystem::path path);
    std::size_t get_visits_for(std::uint32_t x, std::uint32_t y, std::uint32_t z) const;
    std::size_t get_total_visits() const;

private:
    std::unordered_map<Tuple, std::size_t> stats_;   
};

class TileInfo {
public:
    void fill_from(std::filesystem::path path);

    // @babanov1403 TODO: change without extra copies lol
    std::vector<IndexItem> get_topk_by(std::size_t k, StatsLessComparator cmp);

    // @babanov1403 TODO: get rid of setters/getters
    const std::vector<IndexItem>& get_items() const;
    std::span<const IndexItem> get_sample() const;

private:
    std::vector<IndexItem> readFirstKIndexItems(const std::string& filePath, std::size_t k);

private:
    std::vector<IndexItem> items_;
};

} // namespace stats