// #include "absl/flags/flag.h"

#include "metrika/statistics.h"
#include "metrika/strategy.h"
#include "metrika/metrika.h"

#include <string>
#include <format>
#include <iostream>

// ABSL_FLAG(std::string, PATH_INDEX, "", "");

int main(int argc, char* argv[]) {
    // std::string path_to_index = absl::GetFlag(FLAGS_PATH_INDEX);
    std::string path_to_index = "/home/yc-user/tiles/tilesets/data/2025-08-02-planet.index";

    stats::Statistics stats;
    for (std::size_t idx = 1; idx <= 1 /* 30 */; idx++) {
        std::cout << "reading " << idx << "...\n";
        auto day = std::to_string(idx);
        if (idx < 10) {
            day = "0" + day;
        }

        stats.fill_from(std::format("/home/yc-user/tiles/tilesets/log/tiles-2025-09-{}.txt", day));
    }

    std::cout << "total visits is: " << stats.get_total_visits() << '\n';

    stats::TileInfo tiles;
    tiles.fill_from(path_to_index);
    {
        GreedyStrategy strategy;
        PageHandle handler = strategy.build_handler(&stats, &tiles);
        Metrika metrika(&stats, &tiles, &handler);
        std::cout << "Metric for greedy approach is: " << metrika.compute() << '\n';
    }

    {
        RandomStrategy strategy;
        PageHandle handler = strategy.build_handler(&stats, &tiles);
        Metrika metrika(&stats, &tiles, &handler);
        std::cout << "Metric for random approach is: " << metrika.compute() << '\n';
    }
}