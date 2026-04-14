#include "CpuRenderer.h"

#include "glad/gl.h"
#include "imgui.h"

#include <algorithm>
#include <thread>
#include <vector>

const char* CpuRenderer::get_display_name() const {
    return "CPU";
}

void CpuRenderer::draw_imgui_options() {
    ImGui::SliderInt("CPU threads", &threadCount_, 1, 64);
}

int CpuRenderer::get_thread_count() const {
    return threadCount_;
}

void CpuRenderer::set_thread_count(const int threadCount) {
    threadCount_ = std::max(1, threadCount);
}

void CpuRenderer::render(const MandelbrotParams& params, RenderTarget& target) {
    ensure_render_target(target, params.width, params.height);

    pixels_.resize(static_cast<std::size_t>(params.width) * static_cast<std::size_t>(params.height));

    const int workerCount = std::clamp(threadCount_, 1, params.height);
    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(workerCount));

    for (int worker = 0; worker < workerCount; ++worker) {
        const int yBegin = worker * params.height / workerCount;
        const int yEnd = (worker + 1) * params.height / workerCount;
        workers.emplace_back([this, &params, yBegin, yEnd]() {
            compute_mandelbrot_rows(params, yBegin, yEnd, pixels_);
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    glBindTexture(GL_TEXTURE_2D, target.textureId);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    params.width,
                    params.height,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    pixels_.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}
