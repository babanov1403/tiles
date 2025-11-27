#pragma once

#include <cstdint>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <format>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>

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

using Tuple = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;

class LogParser {
    struct ParseResult {
        std::uint32_t x;
        std::uint32_t y;
        std::uint32_t z;
        std::size_t visits;
    };

public:
    explicit LogParser(std::filesystem::path path) : stream_(path) {}

    std::optional<ParseResult> parse_next_line() {
        std::string line;
        if (std::getline(stream_, line)) {
            std::stringstream ss(std::move(line));
            ParseResult result;
            ss >> result.z; ss.get();
            ss >> result.x; ss.get();
            ss >> result.y >> result.visits;
            return result;
        }
        return std::nullopt;
    }
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
    void fill_from(std::filesystem::path path) {
        LogParser parser(path);

        while (auto result = parser.parse_next_line()) {
            auto [x, y, z, visits] = *result;
            stats_[std::make_tuple(x, y, z)] += visits; 
        }
    }

    std::size_t get_visits_for(std::uint32_t x, std::uint32_t y, std::uint32_t z) const {
        if (stats_.contains(std::make_tuple(x, y, z))) {
            return stats_.at(std::make_tuple(x, y, z));
        }
        throw std::runtime_error(std::format("No tile with coords x: {} y:{}, z:{}", x, y, z));
    }
private:
    std::unordered_map<Tuple, std::size_t> stats_;   
};

}