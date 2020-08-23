﻿#include "RenderHints.h"
#include "AUI/GL/gl.h"


extern unsigned char stencilDepth;

void RenderHints::PushMask::pushMask(const std::function<void()>& maskRenderer) {

    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glStencilMask(0xff);
    glColorMask(false, false, false, false);

    maskRenderer();

    glColorMask(true, true, true, true);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, ++stencilDepth, 0xff);

}

void RenderHints::PushMask::popMask(const std::function<void()>& maskRenderer) {

    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
    glStencilMask(0xff);
    glColorMask(false, false, false, false);

    maskRenderer();

    glColorMask(true, true, true, true);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, --stencilDepth, 0xff);
}
