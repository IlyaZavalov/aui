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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 15.04.2021.
//


#pragma once


#include "AViewContainer.h"
#include "ATextField.h"

class API_AUI_VIEWS APathChooserView: public AViewContainer {
private:
    _<ATextField> mPathField;

public:
    explicit APathChooserView(const APath& defaultPath = "");
    APathChooserView(APathChooserView&&) = default;

    void setPath(const APath& path);
    APath getPath() const;

signals:
    emits<APath> changed;
};



template<>
struct ADataBindingDefault<APathChooserView, APath> {
public:
    static auto setup(const _<APathChooserView>& v) {}

    static auto getGetter() {
        return &APathChooserView::changed;
    }
    static auto getSetter() {
        return &APathChooserView::setPath;
    }
};