﻿/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Enum/VerticalAlign.h>
#include <AUI/Enum/TextTransform.h>
#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Enum/WordBreak.h"
#include <AUI/Util/ADataBinding.h>
#include <AUI/Common/IStringable.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Represents an abstract text display view.
 */
class API_AUI_VIEWS AAbstractLabel: public AView, public IStringable
{
private:
    AString mText;
    _<IDrawable> mIcon;
    _<AFont> mFontOverride;
    uint8_t mFontSizeOverride = 0;
    VerticalAlign mVerticalAlign = VerticalAlign::DEFAULT;
    TextTransform mTextTransform = TextTransform::NONE;
    AColor mIconColor = {1, 1, 1, 1};

    glm::ivec2 getIconSize() const;
    AString getTransformedText();
protected:
    Render::PrerenderedString mPrerendered;

    AFontStyle getFontStyleLabel();

    const Render::PrerenderedString& getPrerendered() {
        return mPrerendered;
    }

    //void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;


    // for correct selection positioning (used in ASelectableLabel)
    int mTextLeftOffset = 0;

public:
    AAbstractLabel();
    explicit AAbstractLabel(AString text) noexcept: mText(std::move(text)) {}

    void render() override;
    void doRenderText();

    int getContentMinimumWidth(ALayoutDirection layout) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;

    void invalidateFont() override;

    const _<IDrawable>& getIcon() const
    {
        return mIcon;
    }

    AString toString() const override;

    void setIcon(const _<IDrawable>& drawable) {
        mIcon = drawable;
        redraw();
    }

    void setIconColor(const AColor& iconColor) {
        mIconColor = iconColor;
    }

    void doPrerender();

    bool consumesClick(const glm::ivec2& pos) override;

    void onDpiChanged() override;

    void setText(AString newText) {
        if (mText == newText) {
            return;
        }
        mText = std::move(newText);
        mPrerendered = nullptr;

        requestLayoutUpdate();
        redraw();
    }

    [[nodiscard]]
    const AString& text() const
    {
        return mText;
    }

    void setFont(_<AFont> font) {
        mFontOverride = std::move(font);
        invalidateFont();
    }
    void setFontSize(uint8_t size) {
        mFontSizeOverride = size;
        invalidateFont();
    }

    void setVerticalAlign(VerticalAlign verticalAlign) {
        mVerticalAlign = verticalAlign;
        invalidateFont();
    }
    void setTextTransform(TextTransform textTransform) {
        mTextTransform = textTransform;
        invalidateFont();
    }

    void setSize(glm::ivec2 size) override;
};


template<>
struct ADataBindingDefault<AAbstractLabel, AString> {
public:
    static void setup(const _<AAbstractLabel>& view) {}
    static auto getGetter() {
        return (ASignal<AString> AAbstractLabel::*)nullptr;
    }
    static auto getSetter() {
        return &AAbstractLabel::setText;
    }
};

