#include "Benchmark.h"

#include "glad/gl.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

TimingStats run_benchmark(IRenderer& renderer,
                          const MandelbrotParams& params,
                          RenderTarget& target,
                          const int warmupRuns,
                          const int measuredRuns) {
    if (measuredRuns <= 0) {
        throw std::runtime_error("Measured benchmark runs must be positive");
    }

    for (int i = 0; i < warmupRuns; ++i) {
        renderer.render(params, target);
        glFinish();
    }

    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(measuredRuns));
    for (int i = 0; i < measuredRuns; ++i) {
        const auto start = std::chrono::high_resolution_clock::now();
        renderer.render(params, target);
        glFinish();
        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;
        samples.push_back(elapsed.count());
    }

    TimingStats stats;
    stats.minMs = std::numeric_limits<double>::max();
    stats.maxMs = std::numeric_limits<double>::lowest();
    double sum = 0.0;
    for (const double sample : samples) {
        stats.minMs = std::min(stats.minMs, sample);
        stats.maxMs = std::max(stats.maxMs, sample);
        sum += sample;
    }
    stats.avgMs = sum / static_cast<double>(samples.size());
    return stats;
}

std::vector<BenchmarkResult> benchmark_all(const std::vector<std::unique_ptr<IRenderer>>& renderers,
                                           const MandelbrotParams& params,
                                           RenderTarget& target,
                                           const int warmupRuns,
                                           const int measuredRuns) {
    std::vector<BenchmarkResult> results;
    results.reserve(renderers.size());

    for (const auto& renderer : renderers) {
        BenchmarkResult result;
        result.rendererName = renderer->get_display_name();
        result.stats = run_benchmark(*renderer, params, target, warmupRuns, measuredRuns);
        results.push_back(result);
    }

    return results;
}
