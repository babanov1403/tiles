#include "statistics.h"

#include <algorithm>
#include <ranges>

#include <iostream>

namespace stats {

StatsLessComparator::StatsLessComparator(stats::Statistics* stats) : stats_(stats) {}

bool StatsLessComparator::operator()(const libtiles::tileindex::IndexItem& lhs, const libtiles::tileindex::IndexItem& rhs) const {
    return stats_->get_visits_for(lhs.x, lhs.y, lhs.z) < stats_->get_visits_for(rhs.x, rhs.y, rhs.z);
}

using libtiles::tileindex::IndexItem;
using libtiles::tileindex::readIndexItems;
using Tuple = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;

LogParser::LogParser(std::filesystem::path path) : stream_(path, std::ios_base::ate | std::ios_base::binary) {}

// @babanov1403 TODO: this is too slow to parse line by line file
// do smth with it
std::vector<LogParser::ParseResult> LogParser::parse() {
    size_t fileSize = stream_.tellg();
    std::vector<LogParser::ParseResult> result(fileSize / sizeof(LogParser::ParseResult));
    stream_.seekg(0);
    stream_.read(reinterpret_cast<char*>(result.data()), result.size() * sizeof(LogParser::ParseResult));
    return result;
}

// @brief
// if [x,y,z] not present in map, it will make new key with value visits
// else it will add value to existing element
void Statistics::fill_from(std::filesystem::path path) {
    LogParser parser(path);
    auto result = parser.parse();
    auto filter_pred = [zoom = kMaxZoom](const auto& item) {
        return item.z <= zoom;
    };
    for (auto [x, y, z, visits] : result | std::ranges::views::filter(filter_pred)) {
        stats_[std::make_tuple(x, y, z)] += visits; 
    }
}

std::size_t Statistics::get_visits_for(std::uint32_t x, std::uint32_t y, std::uint32_t z) const {
    if (stats_.contains(std::make_tuple(x, y, z))) {
        return stats_.at(std::make_tuple(x, y, z));
    }
    return 0;
    // throw std::runtime_error(std::format("No tile with coords x: {} y:{}, z:{}", x, y, z));
}

std::size_t Statistics::get_total_visits() const {
    // @babanov1403 TODO: is it ok?
    static std::size_t total_visits = 0;
    if (total_visits == 0) {
        for (const auto& [coords, visits] : stats_) {
            total_visits += visits;
        }
    }
    return total_visits;
}

void TileInfo::fill_from(std::filesystem::path path) {
    items_ = read_index_items(path);
}

// @babanov1403 TODO: change without extra copies lol
const std::vector<IndexItem>& TileInfo::get_sorted(StatsLessComparator cmp) {
    std::ranges::sort(items_, cmp);
    std::ranges::reverse(items_);
    return items_;
}

// @babanov1403 TODO: get rid of setters/getters
const std::vector<IndexItem>& TileInfo::get_items() const {
    return items_;
}

std::span<const IndexItem> TileInfo::get_sample() const {
    return std::span(items_.begin() + 10, items_.begin() + 100);
}

std::vector<IndexItem> TileInfo::read_index_items(const std::string& filePath) {
    std::ifstream istream(filePath, std::ios::binary | std::ios::ate);
    size_t fileSize = istream.tellg();
    std::vector<IndexItem> result(fileSize / sizeof(IndexItem));
    istream.seekg(0);
    istream.read(reinterpret_cast<char*>(result.data()), result.size() * sizeof(IndexItem));
    return filter_below_zoom(std::move(result));
}

std::vector<IndexItem> TileInfo::filter_below_zoom(std::vector<IndexItem>&& items) {
    std::vector<IndexItem> filtered;
    auto filter_pred = [zoom = kMaxZoom](const IndexItem& item) {
        return item.z <= zoom;
    };
    for (const auto& item : items | std::ranges::views::filter(filter_pred)) {
        filtered.emplace_back(item);
    }
    return filtered;
}

} // namespace stats