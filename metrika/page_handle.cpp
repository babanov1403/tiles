#include "page_handle.h"

#include <iostream>

PageHandle::PageHandle(double ratio) : ratio_(ratio) {}

void PageHandle::warmup_pages() const {
    // @babanov1403 TODO: run through prioretized pages and touch it
    throw "not implemented";
}

bool PageHandle::is_prioritized(std::uint64_t offset) const {
    return prioritized_.contains(align(offset));
}

void PageHandle::include_page(std::uint64_t offset) {
    prioritized_.insert(align(offset));
}

void PageHandle::exclude_page(std::uint64_t offset) {
    prioritized_.erase(align(offset));
}

std::uint64_t PageHandle::get_next_page(std::uint64_t offset) const {
    return align(offset) + kPageSize;
}

std::uint64_t PageHandle::align(std::uint64_t offset) const {
    constexpr std::size_t kPageMask = kPageSize - 1;
    return offset & ~kPageMask;
}

std::size_t PageHandle::get_page_size() const {
    return kPageSize;
}

std::vector<PageHandle::OffsetFromFileBegin> PageHandle::get_cached() const {
    return std::vector(prioritized_.begin(), prioritized_.end());
}

std::size_t PageHandle::page_count() const {
    return prioritized_.size();
} 

void PageHandle::set_ratio(double ratio) {
    ratio_ = ratio;
}

bool PageHandle::check_memory_exceed() const {
    return prioritized_.size() * kPageSize > kRAMBound * ratio_;
}

// @babanov1403 TODO: does file aligned with page cache?
std::vector<PageHandle::OffsetFromFileBegin> PageHandle::make_page_grid_for(std::size_t file_size_bytes) {
    std::size_t start = 0;
    std::vector<OffsetFromFileBegin> pages((file_size_bytes + kPageSize - 1) / kPageSize);
    for (auto& page : pages) {
        page = start;
        start += kPageSize;
    }
    return pages;
}