#ifndef PDS_PROJEKT_GPURENDERER_H
#define PDS_PROJEKT_GPURENDERER_H

#include "IRenderer.h"
#include <string>

class GpuRenderer final : public IRenderer {
public:
    GpuRenderer() = default;
    ~GpuRenderer() override;

    [[nodiscard]] const char* get_display_name() const override;
    void render(const MandelbrotParams& params, RenderTarget& target) override;

private:
    void ensure_initialized();
    static unsigned int compile_shader(unsigned int shaderType, const std::string& source);

    unsigned int program_ = 0;
    unsigned int vao_ = 0;
    int locCenter_ = -1;
    int locZoom_ = -1;
    int locMaxIterations_ = -1;
    int locResolution_ = -1;
};

#endif //PDS_PROJEKT_GPURENDERER_H
