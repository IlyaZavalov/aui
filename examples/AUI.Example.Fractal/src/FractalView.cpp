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
            "in vec4 pos;"
                 "in vec2 uv;"
                 "out vec2 pass_uv;"
                 "uniform float ratio;"

                 "void main() {"
                    "gl_Position = pos;"
                    "pass_uv = (uv - 0.5) * 2.0;"
                    "pass_uv.x *= ratio;"
                 "}",
                 "in vec2 pass_uv;\n"

                 "#define product(a, b) dvec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
                 "#define conjugate(a) dvec2(a.x,-a.y)\n"
                 "#define divide(a, b) dvec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
                 "uniform float c;"
                 "uniform int iterations;"
                 "uniform dmat4 tr;"
                 "uniform sampler2D tex;"

                 "out vec4 color;"
                 "void main() {"
                    "dvec2 tmp_pass_uv = (tr * dvec4(pass_uv, 0.0, 1.0)).xy;"
                    "dvec2 v = dvec2(0, 0);"
                    "int i = 0;"
                    "for (; i < iterations; ++i) {"
                        "v = product(v, v) + tmp_pass_uv;"
                        "dvec2 tmp = v * v;"
                        "if ((v.x + v.y) > 4) {"
                            "break;"
                        "}"
                    "}"
                    "if (i == iterations) {"
                        "color = vec4(0, 0, 0, 1);"
                    "} else {"
                        "float theta = float(i) / float(iterations);"
                        "color = texture(tex, vec2(theta, 0));"
                    "}"
                 "}", {"pos", "uv"}, "400");
    mShader.compile();
    mShader.use();
    mShader.set("tr", mTransform);
    mShader.set("sq", 1.f);

    mTexture = _new<GL::Texture2D>();
    mTexture->tex2D(AImage::fromUrl(":img/color_scheme_wikipedia.png"));
}

void FractalView::render() {
    AView::render();

    mShader.use();
    mTexture->bind();
    Render::inst().drawRect(0, 0, getWidth(), getHeight());
}

void FractalView::setSize(int width, int height) {
    AView::setSize(width, height);
    mShader.use();
    mShader.set("ratio", mAspectRatio = float(width) / float(height));
}

void FractalView::setIterations(unsigned it) {
    mShader.use();
    mShader.set("iterations", int(it));
}

void FractalView::onMouseWheel(glm::ivec2 pos, int delta) {
    AView::onMouseWheel(pos, delta);
    auto projectedPos = (glm::dvec2(pos) / glm::dvec2(getSize()) - glm::dvec2(0.5)) * 2.0;
    projectedPos.x *= mAspectRatio;
    mTransform = glm::translate(mTransform, glm::dvec3{projectedPos, 0.0});
    mTransform = glm::scale(mTransform, glm::dvec3(1.0 - delta / 1000.0));
    mTransform = glm::translate(mTransform, -glm::dvec3{projectedPos, 0.0});
    mShader.use();
    mShader.set("tr", mTransform);

    updateEmits();

    redraw();
}

void FractalView::reset() {
    mTransform = glm::dmat4(1.0);
    mShader.use();
    mShader.set("tr", mTransform);
    updateEmits();
    redraw();
}

void FractalView::updateEmits() {
    emit centerPosChanged(glm::vec2(mTransform * glm::vec4(0, 0, 0, 1)));
}
