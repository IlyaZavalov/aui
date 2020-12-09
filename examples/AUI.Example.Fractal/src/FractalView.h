#pragma once


#include <AUI/View/AView.h>
#include <AUI/GL/Shader.h>

class FractalView: public AView {
private:
    GL::Shader mShader;

public:
    FractalView();

    void render() override;
    void setC(float c);
    void setIterations(unsigned it);
};

