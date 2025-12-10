// #include "absl/flags/flag.h"

#include "metrika/statistics.h"
#include "metrika/strategy.h"
#include "metrika/metrika.h"

#include <string>
#include <format>
#include <iostream>

// ABSL_FLAG(std::string, PATH_INDEX, "", "");

// 101779385472 total size of tiles, 51047581 for zoom <= 5
// ratio is 5e-4

int main(int argc, char* argv[]) {
    // std::string path_to_index = absl::GetFlag(FLAGS_PATH_INDEX);
    std::string path_to_index = "/home/yc-user/tiles/tilesets/data/2025-08-02-planet.index";

    stats::Statistics stats;
    for (std::size_t idx = 1; idx <= 1 /* 30 */; idx++) {
        auto day = std::to_string(idx);
        if (idx < 10) {
            day = "0" + day;
        }

        stats.fill_from(std::format("/home/yc-user/tiles/tilesets/log/tiles-2025-09-{}.bin", day));
    }

    std::cout << "total visits is: " << stats.get_total_visits() << '\n';
    stats::TileHandle tiles;
    tiles.fill_from(path_to_index);

    std::size_t tiles_size = 0;
    for (auto tile : tiles.get_items()) {
        tiles_size += tile.size;
    }
    std::cout << "total size of tiles in bytes: " << tiles_size << '\n';
    constexpr std::size_t kOriginSize = 101779385472;

    std::cout << tiles_size * 1. / kOriginSize * 16 * 1024ull * 1024 * 1024 << '\n';

    std::cout << "\n====================================================\n";

    {
        RandomStrategy strategy;
        PageHandle handler = strategy.build_handler(&stats, &tiles, tiles_size * 1. / kOriginSize);
        Metrika metrika(&stats, &tiles, &handler);
        std::cout << "Metric for random approach is: " << metrika.compute() << " Mb/s" << '\n';
        std::cout << "Average stats on page is: " << metrika.compute_sum_among_pages() << '\n'; 
    }

    std::cout << "\n====================================================\n";

    {
        KnapsackStrategy strategy;
        PageHandle handler = strategy.build_handler(&stats, &tiles, tiles_size * 1. / kOriginSize);
        Metrika metrika(&stats, &tiles, &handler);
        std::cout << "Metric for knapsack 0-1 approach is: " << metrika.compute() << " Mb/s" << '\n';
        std::cout << "Average stats on page is: " << metrika.compute_sum_among_pages() << '\n'; 
    }

    std::cout << "\n====================================================\n";

    {
        GreedyStrategy strategy;
        PageHandle handler = strategy.build_handler(&stats, &tiles, tiles_size * 1. / kOriginSize);
        Metrika metrika(&stats, &tiles, &handler);
        std::cout << "Metric for greedy approach is: " << metrika.compute() << " Mb/s" << '\n';
        std::cout << "Average stats on page is: " << metrika.compute_sum_among_pages() << '\n'; 
    }

    std::cout << "\n====================================================\n";

    {
        GreedyScaledStrategy strategy;
        PageHandle handler = strategy.build_handler(&stats, &tiles, tiles_size * 1. / kOriginSize);
        Metrika metrika(&stats, &tiles, &handler);
        std::cout << "Metric for scaled greedy approach is: " << metrika.compute() << " Mb/s" << '\n';
        std::cout << "Average stats on page is: " << metrika.compute_sum_among_pages() << '\n'; 
    }
}