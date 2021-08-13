/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 01.01.2021.
//

#include "TextBorder.h"
#include <AUI/View/ALabel.h>
#include <AUI/Render/RenderHints.h>

void ass::decl::Declaration<ass::TextBorder>::renderFor(AView* view) {
    if (auto label = dynamic_cast<ALabel*>(view)) {
        auto prevColor = view->getFontStyle().color;
        view->getFontStyle().color = mInfo.borderColor;
        view->invalidateFont();
        {
            RenderHints::PushMatrix m;
            Render::inst().translate({-1, 0});
            label->doRenderText();
        }
        {
            RenderHints::PushMatrix m;
            Render::inst().translate({1, 0});
            label->doRenderText();
        }
        {
            RenderHints::PushMatrix m;
            Render::inst().translate({0, -1});
            label->doRenderText();
        }
        {
            RenderHints::PushMatrix m;
            Render::inst().translate({0, 1});
            label->doRenderText();
        }
        view->getFontStyle().color = prevColor;
        view->invalidateFont();
    }
}
ass::decl::DeclarationSlot ass::decl::Declaration<ass::TextBorder>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::TEXT_SHADOW;
}