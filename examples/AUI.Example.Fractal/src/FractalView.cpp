//
// Created by alex2772 on 12/9/20.
//

#include "FractalView.h"
#include <AUI/Render/Render.h>

FractalView::FractalView() {
    setExpanding({2, 2});
    mShader.load("attribute vec4 pos;"
                 "attribute vec2 uv;"
                 "varying vec2 pass_uv;"
                 "void main() {"
                    "gl_Position = pos;"
                    "pass_uv = (uv - 0.5) * 10.0;"
                 "}",
                 "varying vec2 pass_uv;\n"
                 "#define product(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
                 "#define conjugate(a) vec2(a.x,-a.y)\n"
                 "#define divide(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
                 "uniform float c;"
                 "uniform int iterations;"
                 "void main() {"
                    "vec2 v = pass_uv;"
                    "for (int i = 0; i < iterations; ++i) {"
                        "v = product(v, v) + c;"
                    "}"
                    "gl_FragColor = vec4(v, 0, 1);"
                 "}");
    mShader.bindAttribute(0, "pos");
    mShader.bindAttribute(1, "uv");
    mShader.compile();
}

void FractalView::render() {
    AView::render();

    mShader.use();
    Render::inst().drawRect(0, 0, getWidth(), getHeight());
}

void FractalView::setC(float c) {
    mShader.use();
    mShader.set("c", c);
}

void FractalView::setIterations(unsigned it) {
    mShader.use();
    mShader.set("iterations", int(it));
}
