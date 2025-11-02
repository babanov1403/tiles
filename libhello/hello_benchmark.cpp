#include "libhello/hello.h"

#include "benchmark/benchmark.h"

#include <string>
#include <string_view>

void BM_Hello(benchmark::State& state, std::string_view user)
{
    for (auto _ : state) {
        std::string message = libhello::hello(user);
        benchmark::DoNotOptimize(message);
    }
}

BENCHMARK_CAPTURE(BM_Hello, hello_user, "user");
BENCHMARK_CAPTURE(BM_Hello, hello_100500, std::string(100500, 'u'));
