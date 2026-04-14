#include "GpuRenderer.h"

#include "glad/gl.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
#ifndef PDS_PROJECT_SOURCE_DIR
#define PDS_PROJECT_SOURCE_DIR "."
#endif

std::filesystem::path shader_path(const char* fileName) {
    return std::filesystem::path(PDS_PROJECT_SOURCE_DIR) / "src" / "renderers" / "shaders" / fileName;
}

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path.string());
    }
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::string read_shader_log(const unsigned int shaderId) {
    int length = 0;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return {};
    }
    std::vector<char> log(static_cast<std::size_t>(length));
    glGetShaderInfoLog(shaderId, length, nullptr, log.data());
    return {log.data()};
}

std::string read_program_log(const unsigned int programId) {
    int length = 0;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return {};
    }
    std::vector<char> log(static_cast<std::size_t>(length));
    glGetProgramInfoLog(programId, length, nullptr, log.data());
    return {log.data()};
}
}

GpuRenderer::~GpuRenderer() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
}

const char* GpuRenderer::get_display_name() const {
    return "GPU (OpenGL)";
}

unsigned int GpuRenderer::compile_shader(const unsigned int shaderType, const std::string& source) {
    const char* sourcePtr = source.c_str();
    const unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        const std::string log = read_shader_log(shader);
        glDeleteShader(shader);
        throw std::runtime_error("Shader compilation failed: " + log);
    }
    return shader;
}

void GpuRenderer::ensure_initialized() {
    if (program_ != 0) {
        return;
    }

    const std::string vertexSource = read_text_file(shader_path("mandelbrot.vert"));
    const std::string fragmentSource = read_text_file(shader_path("mandelbrot.frag"));
    const unsigned int vertexShader = compile_shader(GL_VERTEX_SHADER, vertexSource);
    const unsigned int fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentSource);

    program_ = glCreateProgram();
    glAttachShader(program_, vertexShader);
    glAttachShader(program_, fragmentShader);
    glLinkProgram(program_);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        const std::string log = read_program_log(program_);
        glDeleteProgram(program_);
        program_ = 0;
        throw std::runtime_error("Shader link failed: " + log);
    }

    locCenter_ = glGetUniformLocation(program_, "uCenter");
    locZoom_ = glGetUniformLocation(program_, "uZoom");
    locMaxIterations_ = glGetUniformLocation(program_, "uMaxIterations");
    locResolution_ = glGetUniformLocation(program_, "uResolution");

    glGenVertexArrays(1, &vao_);
}

void GpuRenderer::render(const MandelbrotParams& params, RenderTarget& target) {
    ensure_initialized();
    ensure_render_target(target, params.width, params.height);

    glBindFramebuffer(GL_FRAMEBUFFER, target.framebufferId);
    glViewport(0, 0, params.width, params.height);

    glUseProgram(program_);
    glUniform2f(locCenter_, static_cast<float>(params.centerX), static_cast<float>(params.centerY));
    glUniform1f(locZoom_, static_cast<float>(params.zoom));
    glUniform1i(locMaxIterations_, params.maxIterations);
    glUniform2i(locResolution_, params.width, params.height);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
