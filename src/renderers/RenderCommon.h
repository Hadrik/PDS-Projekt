#ifndef PDS_PROJEKT_RENDERCOMMON_H
#define PDS_PROJEKT_RENDERCOMMON_H

#include "glad/gl.h"

#include <cstdint>
#include <vector>

struct MandelbrotParams {
    int width = 2560;
    int height = 1440;
    int maxIterations = 500;
    double centerX = -0.75;
    double centerY = 0.0;
    double zoom = 1.0;
};

struct RenderTarget {
    GLuint textureId = 0;
    GLuint framebufferId = 0;
    int width = 0;
    int height = 0;
};

void ensure_render_target(RenderTarget& target, int width, int height);
void destroy_render_target(RenderTarget& target);

std::uint32_t mandelbrot_color(int iteration, int maxIterations);
void compute_mandelbrot_rows(const MandelbrotParams& params, int yBegin, int yEnd, std::vector<std::uint32_t>& pixels);

#endif //PDS_PROJEKT_RENDERCOMMON_H
