#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

class PageHandle {
    using OffsetFromFileBegin = std::size_t;
    static constexpr std::size_t kRAMBound = 16ull * 1024 * 1024 * 1024;
    static constexpr std::size_t kPageSize = 4 * 1024;
public:
    PageHandle() = default;
    explicit PageHandle(double);

    void warmup_pages() const;
    bool is_prioritized(std::uint64_t offset) const;
    bool check_memory_exceed() const;
    void include_page(std::uint64_t offset);
    void exclude_page(std::uint64_t offset);
    std::uint64_t get_next_page(std::uint64_t offset) const;
    std::uint64_t align(std::uint64_t offset) const;
    std::size_t get_page_size() const;
    std::vector<OffsetFromFileBegin> get_cached() const;
    std::size_t page_count() const;

    void set_ratio(double);

public:
    // @babanov1403 TODO: does file aligned with page cache?
    static std::vector<OffsetFromFileBegin> make_page_grid_for(std::size_t file_size_bytes);

private:
    std::unordered_set<OffsetFromFileBegin> prioritized_;
    double ratio_ = 1.;
};