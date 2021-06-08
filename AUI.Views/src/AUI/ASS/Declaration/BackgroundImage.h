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
// Created by alex2 on 3.12.2020.
//

#pragma once

#include <AUI/Render/Render.h>
#include "IDeclaration.h"

namespace ass {
    enum class Sizing {
        NONE,
        FIT,
        FIT_PADDING,

        SPLIT_2X2,
    };

    struct BackgroundImage {
        unset_wrap<AString> url;
        unset_wrap<AColor> overlayColor;
        unset_wrap<Repeat> rep;
        unset_wrap<Sizing> sizing;
        unset_wrap<glm::vec2> scale;

        BackgroundImage() {}

        BackgroundImage(const unset_wrap<AString>& url, const unset_wrap<AColor>& overlayColor):
            url(url),
            overlayColor(overlayColor) {}
        BackgroundImage(const AString& url):
            url(url) {}
        BackgroundImage(const AColor& overlayColor):
            overlayColor(overlayColor) {}

        BackgroundImage(const unset_wrap<AString>& url,
                        const unset_wrap<AColor>& overlayColor,
                        const unset_wrap<Repeat>& rep,
                        const unset_wrap<Sizing>& sizing = {},
                        const unset_wrap<glm::vec2>& scale = {}) : url(url),
                                                              overlayColor(overlayColor),
                                                              rep(rep),
                                                              sizing(sizing),
                                                              scale(scale) {}
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BackgroundImage>: IDeclarationBase {
        private:
            BackgroundImage mVisibility;

        public:
            Declaration(const BackgroundImage& info) : mVisibility(info) {

            }

            void applyFor(AView* view) override;
            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}