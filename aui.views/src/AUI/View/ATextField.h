﻿/**
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

#include "AView.h"
#include "AAbstractTextField.h"
#include <AUI/Util/ADataBinding.h>

/**
 * <p>Represents an editable field with text to receive a text input from the user.</p>
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/ATextField.gif">
 */
class API_AUI_VIEWS ATextField: public AAbstractTextField
{
public:
	ATextField();
	~ATextField() override;
    ATextField(ATextField&&) = default;

    void setText2(const AString& t) {setText(t);}

protected:
	bool isValidText(const AString& text) override;
};

template<>
struct ADataBindingDefault<ATextField, AString> {
public:
    static void setup(const _<ATextField>& view) {}
    static auto getGetter() {
        return (ASignal<AString> ATextField::*)&ATextField::textChanging;
    }
    static auto getSetter() {
        return &ATextField::setText2;
    }
};
