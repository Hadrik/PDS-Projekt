#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION

#include "benchmark/Benchmark.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "renderers/CpuRenderer.h"
#include "renderers/GpuRenderer.h"

int main(int, char**)
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }

    glfwWindowHint(GLFW_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello World", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "Failed to initialize OpenGL.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glViewport(0, 0, 1280, 720);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, const int w, const int h) {
        glViewport(0, 0, w, h);
    });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigDebugIsDebuggerPresent = true;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    MandelbrotParams params;
    RenderTarget renderTarget;
    CpuRenderer cpuRenderer;
    GpuRenderer gpuRenderer;

    int currentRendererIndex = 1;
    int benchmarkWarmups = 2;
    int benchmarkRuns = 10;
    std::vector<BenchmarkResult> benchmarkResults;
    bool isPanningImage = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Options");
        const auto selectedRendererName = (currentRendererIndex == 0) ? cpuRenderer.get_display_name() : gpuRenderer.get_display_name();
        if (ImGui::BeginCombo("Renderer", selectedRendererName)) {
            const bool cpuSelected = currentRendererIndex == 0;
            if (ImGui::Selectable(cpuRenderer.get_display_name(), cpuSelected)) {
                currentRendererIndex = 0;
            }
            if (cpuSelected) {
                ImGui::SetItemDefaultFocus();
            }

            const bool gpuSelected = currentRendererIndex == 1;
            if (ImGui::Selectable(gpuRenderer.get_display_name(), gpuSelected)) {
                currentRendererIndex = 1;
            }
            if (gpuSelected) {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::InputInt("Image width", &params.width);
        ImGui::InputInt("Image height", &params.height);
        params.width = std::max(params.width, 16);
        params.height = std::max(params.height, 16);

        ImGui::SliderInt("Max iterations", &params.maxIterations, 32, 5000);
        params.maxIterations = std::max(params.maxIterations, 32);

        auto centerX = static_cast<float>(params.centerX);
        auto centerY = static_cast<float>(params.centerY);
        auto zoom = static_cast<float>(params.zoom);
        if (ImGui::SliderFloat("Center X", &centerX, -2.5f, 2.5f, "%.5f")) {
            params.centerX = centerX;
        }
        if (ImGui::SliderFloat("Center Y", &centerY, -2.0f, 2.0f, "%.5f")) {
            params.centerY = centerY;
        }
        if (ImGui::SliderFloat("Zoom", &zoom, 1.0f, 500000.0f, "%.5f", ImGuiSliderFlags_Logarithmic)) {
            params.zoom = zoom;
        }

        if (currentRendererIndex == 0) {
            cpuRenderer.draw_imgui_options();
        }

        ImGui::Separator();
        ImGui::InputInt("Benchmark warmups", &benchmarkWarmups);
        ImGui::InputInt("Benchmark runs", &benchmarkRuns);
        benchmarkWarmups = std::max(benchmarkWarmups, 0);
        benchmarkRuns = std::max(benchmarkRuns, 1);

        if (ImGui::Button("Run benchmark")) {
            benchmarkResults.clear();

            const int originalThreadCount = cpuRenderer.get_thread_count();
            cpuRenderer.set_thread_count(1);
            benchmarkResults.push_back({"CPU (1 thread)", run_benchmark(cpuRenderer, params, renderTarget, benchmarkWarmups, benchmarkRuns)});

            const int multiThreadCount = std::max(2, originalThreadCount);
            cpuRenderer.set_thread_count(multiThreadCount);
            benchmarkResults.push_back({"CPU (" + std::to_string(multiThreadCount) + " threads)",
                                        run_benchmark(cpuRenderer, params, renderTarget, benchmarkWarmups, benchmarkRuns)});

            benchmarkResults.push_back({"GPU (OpenGL)", run_benchmark(gpuRenderer, params, renderTarget, benchmarkWarmups, benchmarkRuns)});
            cpuRenderer.set_thread_count(originalThreadCount);
        }

        if (ImGui::BeginTable("benchmark-table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Renderer");
            ImGui::TableSetupColumn("Min (ms)");
            ImGui::TableSetupColumn("Avg (ms)");
            ImGui::TableSetupColumn("Max (ms)");
            ImGui::TableHeadersRow();
            for (const auto&[rendererName, stats] : benchmarkResults) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(rendererName.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.3f", stats.minMs);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.3f", stats.avgMs);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.3f", stats.maxMs);
            }
            ImGui::EndTable();
        }
        ImGui::End();

        if (currentRendererIndex == 0) {
            cpuRenderer.render(params, renderTarget);
        } else {
            gpuRenderer.render(params, renderTarget);
        }

        ImGui::Begin("Mandelbrot");
        const auto available = ImGui::GetContentRegionAvail();
        if (available.x > 1.0f && available.y > 1.0f && renderTarget.textureId != 0) {
            auto textureSize = available;
            const auto textureAspectRatio = static_cast<float>(renderTarget.width) / static_cast<float>(renderTarget.height);
            const auto availAspectRatio = available.x / available.y;
            if (textureAspectRatio > availAspectRatio) {
                textureSize.y = available.x / textureAspectRatio;
            } else {
                textureSize.x = available.y * textureAspectRatio;
            }

            const auto textureId = static_cast<ImTextureID>(static_cast<intptr_t>(renderTarget.textureId));
            ImGui::Image(textureId, textureSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

            const auto imageMin = ImGui::GetItemRectMin();
            const auto imageMax = ImGui::GetItemRectMax();
            const ImVec2 imageSize = {imageMax.x - imageMin.x, imageMax.y - imageMin.y};
            const bool imageHovered = ImGui::IsItemHovered();

            if (imageSize.x > 1.0f && imageSize.y > 1.0f) {
                if (imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    isPanningImage = true;
                }
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    isPanningImage = false;
                }

                if (isPanningImage && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
                    const auto mouseDelta = io.MouseDelta;
                    const auto scale = 1.0 / params.zoom;
                    const auto aspect = static_cast<double>(params.width) / static_cast<double>(params.height);
                    params.centerX -= static_cast<double>(mouseDelta.x) / static_cast<double>(imageSize.x) * 2.0 * scale * aspect;
                    params.centerY += static_cast<double>(mouseDelta.y) / static_cast<double>(imageSize.y) * 2.0 * scale;
                    params.centerX = std::clamp(params.centerX, -2.5, 2.5);
                    params.centerY = std::clamp(params.centerY, -2.0, 2.0);
                }

                if (imageHovered && io.MouseWheel != 0.0f) {
                    const auto mouseX = std::clamp(io.MousePos.x, imageMin.x, imageMax.x);
                    const auto mouseY = std::clamp(io.MousePos.y, imageMin.y, imageMax.y);
                    const auto u = static_cast<double>(mouseX - imageMin.x) / static_cast<double>(imageSize.x);
                    const auto v = static_cast<double>(mouseY - imageMin.y) / static_cast<double>(imageSize.y);
                    const auto xNorm = (u - 0.5) * 2.0;
                    const auto yNorm = (0.5 - v) * 2.0;

                    const auto aspect = static_cast<double>(params.width) / static_cast<double>(params.height);
                    const auto oldScale = 1.0 / params.zoom;
                    const auto worldX = params.centerX + xNorm * oldScale * aspect;
                    const auto worldY = params.centerY + yNorm * oldScale;

                    const auto zoomFactor = std::pow(1.1, static_cast<double>(io.MouseWheel));
                    params.zoom = std::clamp(params.zoom * zoomFactor, 1.0, 500000.0);

                    const auto newScale = 1.0 / params.zoom;
                    params.centerX = std::clamp(worldX - xNorm * newScale * aspect, -2.5, 2.5);
                    params.centerY = std::clamp(worldY - yNorm * newScale, -2.0, 2.0);
                }
            }
        }
        ImGui::End();

        int d_w, d_h;
        glfwGetFramebufferSize(window, &d_w, &d_h);
        glViewport(0, 0, d_w, d_h);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLFWwindow* backup_window = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_window);
        glfwSwapBuffers(window);
    }

    destroy_render_target(renderTarget);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
