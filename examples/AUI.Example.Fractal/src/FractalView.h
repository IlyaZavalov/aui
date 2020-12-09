#pragma once


#include <AUI/View/AView.h>
#include <AUI/GL/Shader.h>

class FractalView: public AView {
private:
    GL::Shader mShader;

    glm::mat4 mTransform;

public:
    FractalView();

    void render() override;
    void reset();
    void setIterations(unsigned it);

    void onMouseWheel(glm::ivec2 pos, int delta) override;
};

