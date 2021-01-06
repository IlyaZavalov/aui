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

/**
 * It's not an ASS, it's Aui Style Sheets!!
 */

#pragma once

// Declarations
#include "Declaration/BackgroundImage.h"
#include "Declaration/BackgroundSolid.h"
#include "Declaration/BackgroundGradient.h"
#include "Declaration/Border.h"
#include "Declaration/BorderRadius.h"
#include "Declaration/BoxShadow.h"
#include "Declaration/Cursor.h"
#include "Declaration/FixedSize.h"
#include "Declaration/FontFamily.h"
#include "Declaration/FontRendering.h"
#include "Declaration/FontSize.h"
#include "Declaration/LayoutSpacing.h"
#include "Declaration/Margin.h"
#include "Declaration/MinSize.h"
#include "Declaration/Overflow.h"
#include "Declaration/Padding.h"
#include "Declaration/TextAlign.h"
#include "Declaration/TextColor.h"

// Selectors
#include "AUI/ASS/Selector/AAssSelector.h"
#include "Selector/ParentSelector.h"
#include "Selector/DirectParentSelector.h"
#include "Selector/any.h"
#include "Selector/attribute.h"
#include "Selector/active.h"
#include "Selector/hovered.h"
#include "Selector/class_of.h"

// Other
#include "unset.h"
#include "AStylesheet.h"