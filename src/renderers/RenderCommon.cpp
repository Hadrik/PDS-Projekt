#include "RenderCommon.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
std::uint32_t pack_rgba8(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a = 255) {
    return static_cast<std::uint32_t>(r) |
           (static_cast<std::uint32_t>(g) << 8u) |
           (static_cast<std::uint32_t>(b) << 16u) |
           (static_cast<std::uint32_t>(a) << 24u);
}
}

void ensure_render_target(RenderTarget& target, const int width, const int height) {
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Render target size must be positive");
    }

    if (target.textureId != 0 && target.framebufferId != 0 && target.width == width && target.height == height) {
        return;
    }

    if (target.textureId == 0) {
        glGenTextures(1, &target.textureId);
    }
    if (target.framebufferId == 0) {
        glGenFramebuffers(1, &target.framebufferId);
    }

    glBindTexture(GL_TEXTURE_2D, target.textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, target.framebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.textureId, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw std::runtime_error("Failed to create framebuffer for render target");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    target.width = width;
    target.height = height;
}

void destroy_render_target(RenderTarget& target) {
    if (target.framebufferId != 0) {
        glDeleteFramebuffers(1, &target.framebufferId);
        target.framebufferId = 0;
    }
    if (target.textureId != 0) {
        glDeleteTextures(1, &target.textureId);
        target.textureId = 0;
    }
    target.width = 0;
    target.height = 0;
}

std::uint32_t mandelbrot_color(const int iteration, const int maxIterations) {
    if (iteration >= maxIterations) {
        return pack_rgba8(0, 0, 0);
    }

    const double t = static_cast<double>(iteration) / static_cast<double>(maxIterations);
    const double oneMinus = 1.0 - t;
    const auto r = static_cast<std::uint8_t>(std::clamp(9.0 * oneMinus * t * t * t * 255.0, 0.0, 255.0));
    const auto g = static_cast<std::uint8_t>(std::clamp(15.0 * oneMinus * oneMinus * t * t * 255.0, 0.0, 255.0));
    const auto b = static_cast<std::uint8_t>(std::clamp(8.5 * oneMinus * oneMinus * oneMinus * t * 255.0, 0.0, 255.0));
    return pack_rgba8(r, g, b);
}

void compute_mandelbrot_rows(const MandelbrotParams& params,
                             const int yBegin,
                             const int yEnd,
                             std::vector<std::uint32_t>& pixels) {
    const int width = params.width;
    const int height = params.height;
    const int maxIterations = params.maxIterations;
    const double scale = 1.0 / params.zoom;
    const double aspect = static_cast<double>(width) / static_cast<double>(height);

    for (int y = yBegin; y < yEnd; ++y) {
        const double yNorm = ((static_cast<double>(y) / static_cast<double>(height)) - 0.5) * 2.0;
        const double cY = params.centerY + yNorm * scale;
        const int rowOffset = y * width;

        for (int x = 0; x < width; ++x) {
            const double xNorm = ((static_cast<double>(x) / static_cast<double>(width)) - 0.5) * 2.0;
            const double cX = params.centerX + xNorm * scale * aspect;

            double zX = 0.0;
            double zY = 0.0;
            int iteration = 0;
            while ((zX * zX + zY * zY) <= 4.0 && iteration < maxIterations) {
                const double nextZX = zX * zX - zY * zY + cX;
                zY = 2.0 * zX * zY + cY;
                zX = nextZX;
                ++iteration;
            }

            pixels[static_cast<std::size_t>(rowOffset + x)] = mandelbrot_color(iteration, maxIterations);
        }
    }
}
