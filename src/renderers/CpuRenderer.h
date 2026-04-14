#ifndef PDS_PROJEKT_CPURENDERER_H
#define PDS_PROJEKT_CPURENDERER_H

#include "IRenderer.h"

#include <cstdint>
#include <vector>
#include <thread>

class CpuRenderer final : public IRenderer {
public:
    [[nodiscard]] const char* get_display_name() const override;
    void render(const MandelbrotParams& params, RenderTarget& target) override;
    void draw_imgui_options() override;

    [[nodiscard]] int get_thread_count() const;
    void set_thread_count(int threadCount);

private:
    int threadCount_ = static_cast<int>(std::thread::hardware_concurrency());
    std::vector<std::uint32_t> pixels_;
};

#endif //PDS_PROJEKT_CPURENDERER_H
