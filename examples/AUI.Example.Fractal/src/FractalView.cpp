//
// Created by alex2772 on 12/9/20.
//

#include "FractalView.h"
#include <AUI/Render/Render.h>
#include <glm/gtc/matrix_transform.hpp>

FractalView::FractalView():
    mTransform(1.f)
{
    setExpanding({2, 2});
    mShader.load(
            "attribute vec4 pos;"
                 "attribute vec2 uv;"
                 "varying vec2 pass_uv;"
                 "uniform mat4 tr;"

                 "void main() {"
                    "gl_Position = pos;"
                    "pass_uv = (tr * vec4((vec2(uv) - 0.5) * 2.0, 0.0, 1.0)).xy;"
                 "}",
                 "varying vec2 pass_uv;\n"
                 "#define product(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
                 "#define conjugate(a) vec2(a.x,-a.y)\n"
                 "#define divide(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
                 "uniform float c;"
                 "uniform int iterations;"
                 "void main() {"
                    "vec2 v = vec2(0, 0);"
                    "for (int i = 0; i < iterations; ++i) {"
                        "v = product(v, v) + pass_uv;"
                    "}"
                    "gl_FragColor = vec4(length(v), 0, 0, 1);"
                 "}");
    mShader.bindAttribute(0, "pos");
    mShader.bindAttribute(1, "uv");
    mShader.compile();
    mShader.use();
    mShader.set("tr", mTransform);
}

void FractalView::render() {
    AView::render();

    mShader.use();
    Render::inst().drawRect(0, 0, getWidth(), getHeight());
}

void FractalView::setIterations(unsigned it) {
    mShader.use();
    mShader.set("iterations", int(it));
}

void FractalView::onMouseWheel(glm::ivec2 pos, int delta) {
    AView::onMouseWheel(pos, delta);

    mTransform = glm::translate(mTransform, glm::vec3{(glm::vec2(pos) / glm::vec2(getSize()) - glm::vec2(0.5)) * 2.0f, 0.0});
    mTransform = glm::scale(mTransform, glm::vec3(1.f + delta / 100.0));
    mTransform = glm::translate(mTransform, -glm::vec3{(glm::vec2(pos) / glm::vec2(getSize()) - glm::vec2(0.5)) * 2.0f, 0.0});
    mShader.use();
    mShader.set("tr", mTransform);

    redraw();
}

void FractalView::reset() {
    mTransform = glm::mat4(1.f);
    mShader.use();
    mShader.set("tr", mTransform);
    redraw();
}
