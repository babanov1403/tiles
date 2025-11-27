// #include "absl/flags/flag.h"

#include "metrika/statistics.h"

#include <string>
#include <format>

// ABSL_FLAG(std::string, PATH_INDEX, "", "");



int main(int argc, char* argv[]) {
    // std::string path_to_index = absl::GetFlag(FLAGS_PATH_INDEX);
    std::string path_to_index = "~/tiles/tilesets/2025-08-02-planet.index";

    stats::Statistics stats;
    for (std::size_t idx = 1; idx <= 2 /* 30 */; idx++) {
        auto day = std::to_string(idx);
        if (idx < 10) {
            day = "0" + day;
        }

        stats.fill_from(std::format("~/tiles/tilesets/log/tiles-2025-09-{}.txt", day));
    }
}