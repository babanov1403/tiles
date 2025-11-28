#include "statistics.h"

#include <algorithm>
#include <format>
#include <stdexcept>
#include <sstream>

#include <iostream>

namespace stats {

StatsLessComparator::StatsLessComparator(stats::Statistics* stats) : stats_(stats) {}

bool StatsLessComparator::operator()(const libtiles::tileindex::IndexItem& lhs, const libtiles::tileindex::IndexItem& rhs) const {
    return stats_->get_visits_for(lhs.x, lhs.y, lhs.z) < stats_->get_visits_for(rhs.x, rhs.y, rhs.z);
}

using libtiles::tileindex::IndexItem;
using libtiles::tileindex::readIndexItems;
using Tuple = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;

LogParser::LogParser(std::filesystem::path path) : stream_(path) {}

// @babanov1403 TODO: this is too slow to parse line by line file
// do smth with it
std::optional< LogParser::ParseResult>  LogParser::parse_next_line() {
    std::string line;
    if (std::getline(stream_, line)) {
        std::stringstream ss(line);
        ParseResult result;
        ss >> result.z; ss.get();
        ss >> result.x; ss.get();
        ss >> result.y >> result.visits;
        return result;
    }
    return std::nullopt;
}

// @brief
// if [x,y,z] not present in map, it will make new key with value visits
// else it will add value to existing element
void Statistics::fill_from(std::filesystem::path path) {
    LogParser parser(path);
    while (auto result = parser.parse_next_line()) {
        auto [x, y, z, visits] = *result;
        stats_[std::make_tuple(x, y, z)] += visits; 
    }
}

std::size_t Statistics::get_visits_for(std::uint32_t x, std::uint32_t y, std::uint32_t z) const {
    if (stats_.contains(std::make_tuple(x, y, z))) {
        return stats_.at(std::make_tuple(x, y, z));
    }
    throw std::runtime_error(std::format("No tile with coords x: {} y:{}, z:{}", x, y, z));
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
    items_ = readFirstKIndexItems(path, 100);
}

// @babanov1403 TODO: change without extra copies lol
std::vector<IndexItem> TileInfo::get_topk_by(std::size_t k, StatsLessComparator cmp) {
    // @babanov1403 TODO: maybe juggle with IndexItem* , idk
    // std::priority_queue<IndexItem, Comp> heap;
    // for (const auto& item : items_) {
    //     heap.size() < k ? heap.push(item) : heap.pop();
    // }

    // std::vector<IndexItem> output;
    // output.reserve(k);
    // while (!heap.empty()) {
    //     output.emplace_back(heap.top());
    //     heap.pop();
    // }

    std::ranges::sort(items_, cmp);
    auto result = std::vector(items_.begin(), items_.begin() + k);
    std::ranges::reverse(result);
    return result;
}

// @babanov1403 TODO: get rid of setters/getters
const std::vector<IndexItem>& TileInfo::get_items() const {
    return items_;
}

std::span<const IndexItem> TileInfo::get_sample() const {
    return std::span(items_.begin() + 10, items_.begin() + 100);
}

std::vector<IndexItem> TileInfo::readFirstKIndexItems(const std::string& filePath, std::size_t k) {
    std::ifstream istream(filePath, std::ios::binary | std::ios::ate);
    size_t fileSize = istream.tellg();
    std::vector<IndexItem> result(k);
    istream.seekg(0);
    istream.read(reinterpret_cast<char*>(result.data()), result.size() * sizeof(IndexItem));
    return result;
}

} // namespace stats