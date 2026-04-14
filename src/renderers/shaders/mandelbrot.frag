#version 330 core

uniform vec2 uCenter;
uniform float uZoom;
uniform int uMaxIterations;
uniform ivec2 uResolution;

out vec4 outColor;

vec3 palette(float t) {
    float oneMinus = 1.0 - t;
    float r = 9.0 * oneMinus * t * t * t;
    float g = 15.0 * oneMinus * oneMinus * t * t;
    float b = 8.5 * oneMinus * oneMinus * oneMinus * t;
    return vec3(r, g, b);
}

void main() {
    vec2 uv = ((gl_FragCoord.xy / vec2(uResolution)) - vec2(0.5)) * 2.0;
    float aspect = float(uResolution.x) / float(uResolution.y);
    float scale = 1.0 / uZoom;
    vec2 c = vec2(uCenter.x + uv.x * scale * aspect, uCenter.y + uv.y * scale);

    vec2 z = vec2(0.0, 0.0);
    int iter = 0;
    while (dot(z, z) <= 4.0 && iter < uMaxIterations) {
        z = vec2(z.x * z.x - z.y * z.y + c.x, 2.0 * z.x * z.y + c.y);
        iter++;
    }

    if (iter == uMaxIterations) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        float t = float(iter) / float(uMaxIterations);
        outColor = vec4(palette(t), 1.0);
    }
}
