#ifndef PDS_PROJEKT_BENCHMARK_H
#define PDS_PROJEKT_BENCHMARK_H

#include "../renderers/IRenderer.h"

#include <memory>
#include <string>
#include <vector>

struct TimingStats {
    double minMs = 0.0;
    double avgMs = 0.0;
    double maxMs = 0.0;
};

struct BenchmarkResult {
    std::string rendererName;
    TimingStats stats;
};

TimingStats run_benchmark(IRenderer& renderer,
                          const MandelbrotParams& params,
                          RenderTarget& target,
                          int warmupRuns,
                          int measuredRuns);

std::vector<BenchmarkResult> benchmark_all(const std::vector<std::unique_ptr<IRenderer>>& renderers,
                                           const MandelbrotParams& params,
                                           RenderTarget& target,
                                           int warmupRuns,
                                           int measuredRuns);

#endif //PDS_PROJEKT_BENCHMARK_H
