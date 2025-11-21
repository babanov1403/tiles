#pragma once

#include <filesystem>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace metrika {

std::size_t align_offset_by_page(std::size_t);
std::size_t get_size(std::size_t offset = 0);
double compute_metrics(std::vector<std::size_t> offsets,
                       std::vector<double> statistics,
                       std::vector<bool> is_cached);

}