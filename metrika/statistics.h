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

static std::size_t kMaxZoom = 4;

using libtiles::tileindex::IndexItem;
using Tuple = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;

class Statistics;

struct StatsGreaterComparator {
    StatsGreaterComparator() = delete;
    explicit StatsGreaterComparator(stats::Statistics* stats);

    bool operator()(const libtiles::tileindex::IndexItem& lhs, const libtiles::tileindex::IndexItem& rhs) const;

private:
    stats::Statistics* stats_;
};

struct StatsGreaterScaledComparator {
    StatsGreaterScaledComparator() = delete;
    explicit StatsGreaterScaledComparator(stats::Statistics* stats);

    bool operator()(const libtiles::tileindex::IndexItem& lhs, const libtiles::tileindex::IndexItem& rhs) const;

private:
    stats::Statistics* stats_;
};

class LogParser {
    struct __attribute__((packed)) ParseResult {
        std::uint64_t x;
        std::uint64_t y;
        std::uint64_t z;
        std::uint64_t visits;
    };

public:
    explicit LogParser(std::filesystem::path path);

    // @babanov1403 TODO: this is too slow to parse line by line file
    // do smth with it
    std::vector<ParseResult> parse();

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

class TileHandle {
public:
    void fill_from(std::filesystem::path path);

    // @babanov1403 TODO: get rid of setters/getters
    const std::vector<IndexItem>& get_items() const;
    std::vector<IndexItem>& get_items_mutable();
    std::span<const IndexItem> get_first(std::size_t k = 10) const;

private:
    std::vector<IndexItem> read_index_items(const std::string&);
    std::vector<IndexItem> filter_below_zoom(std::vector<IndexItem>&& items);

private:
    std::vector<IndexItem> items_;
};

} // namespace stats