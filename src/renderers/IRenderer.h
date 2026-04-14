#ifndef PDS_PROJEKT_IRENDERER_H
#define PDS_PROJEKT_IRENDERER_H

#include "RenderCommon.h"

class IRenderer {
public:
    virtual ~IRenderer() = default;

    [[nodiscard]] virtual const char* get_display_name() const = 0;

    virtual void render(const MandelbrotParams& params, RenderTarget& target) = 0;

    virtual void draw_imgui_options() {}
};

#endif //PDS_PROJEKT_IRENDERER_H
