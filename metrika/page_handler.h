#pragma once

#include <cstdint>
#include <unordered_set>

class PageHandle {
    using OffsetFromFileBegin = std::size_t;
    static constexpr std::size_t kMaxPages = 1e9;
    static constexpr std::size_t kPageSize = 4 * 1024;
public:
    PageHandle() = default;
    void warmup_pages() const {
        // @babanov1403 TODO: run through prioretized pages and touch it
        throw "not implemented";
    }

    bool is_prioritized(std::uint64_t offset) const {
        return prioritized.contains(align(offset));
    }

    void include_page(std::uint64_t offset) {
        // @babanov1403 TODO: check if size dont exceed kMaxPages
        prioritized.insert(align(offset));
    }

    void exclude_page(std::uint64_t offset) {
        prioritized.erase(align(offset));
    }

private:
    std::uint64_t align(std::uint64_t offset) const {
        constexpr std::size_t kPageMask = kPageSize - 1;
        return offset & ~kPageMask;
    }

private:
    std::unordered_set<OffsetFromFileBegin> prioritized;
};