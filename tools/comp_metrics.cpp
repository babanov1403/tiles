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

template <class S>
void output_metrics(stats::Statistics* stats, stats::TileHandle* tiles_original, double ratio, std::string strategy_name) {
    auto tiles = *tiles_original;
    std::cout << "\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
    std::cout << "Running strategy for " << strategy_name << '\n';
    S strategy;
    PageHandle handler = strategy.build_handler(stats, &tiles, ratio);
    Metrika metrika(stats, &tiles, &handler);
    std::cout << "Paged metric is: " << metrika.compute() << " Mb/s" << '\n';
    std::cout << "Unpaged metric is: " << metrika.compute_unpaged() << " Mb/s" << '\n';
    // std::cout << "Average stats on page is: " << metrika.compute_sum_among_pages() << '\n';
    std::cout << "Ideal metric is: " << metrika.compute_ideal() << " Mb/s" << '\n';
    std::cout << "\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
}

int main(int argc, char* argv[]) {
    // std::string path_to_index = absl::GetFlag(FLAGS_PATH_INDEX);
    std::string path_to_index = "/home/yc-user/tiles/tilesets/data/2025-08-02-planet.index";

    std::cout << "There are three places with RAM parameter to change...\n";

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

    std::cout << tiles.get_items().size() << '\n';

    std::size_t tiles_size = 0;
    std::cout << "total tiles: " << tiles.get_items().size() << '\n';
    for (auto tile : tiles.get_items()) {
        tiles_size += tile.size;
    }
    std::cout << "total size of tiles in megabytes: " << tiles_size / 1024. / 1024. << '\n';
    constexpr std::size_t kOriginSize = 101779385472;

    std::cout << tiles_size * 1. / kOriginSize * 16 * 1024ull * 1024 * 1024 << '\n';
    double ratio = tiles_size * 1. / kOriginSize;

    output_metrics<MetricKnapsackStrategy>(&stats, &tiles, ratio, "MetricKnapsackStrategy");
    // output_metrics<MetricKnapsackSplittingStrategy>(&stats, &tiles, ratio, "MetricKnapsackSplittingStrategy");
    // output_metrics<GreedySectorStrategy>(&stats, &tiles, ratio, "SectorStrategy");
    // output_metrics<AlignStrategy>(&stats, &tiles, ratio, "AlignStategy");
    // output_metrics<KnapsackStrategy>(&stats, &tiles, ratio, "KnapsackStrategy");
    output_metrics<GreedyStrategy>(&stats, &tiles, ratio, "GreedyStrategy");
    // output_metrics<GreedyScaledStrategy>(&stats, &tiles, ratio, "GreedyScaledStrategy");
    // output_metrics<RandomStrategy>(&stats, &tiles, ratio, "RandomStrategy");
    output_metrics<GreedyNoReaarrangeStrategy>(&stats, &tiles, ratio, "GreedyNoRearrangeStrategy");
}