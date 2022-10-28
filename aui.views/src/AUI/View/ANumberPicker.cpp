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

#include "ANumberPicker.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AButton.h"
#include "AUI/Platform/Platform.h"
#include <AUI/Util/kAUI.h>

bool ANumberPicker::ANumberPickerField::isValidText(const AString& text)
{
	if (text.length() > 10)
		return false;
	for (auto& c : text)
	{
		if ((c < '0' || c > '9') && c != '-')
			return false;
	}

	return true;
}

void ANumberPicker::ANumberPickerField::onKeyRepeat(AInput::Key key) {

    switch (key) {
        case AInput::DOWN:
            mPicker.decrease();
            break;
        case AInput::UP:
            mPicker.increase();
            break;
        default:
            AAbstractTextField::onKeyRepeat(key);
    }
}


ANumberPicker::ANumberPicker()
{

	
	setLayout(_new<AHorizontalLayout>());
	addView(mTextField = _new<ANumberPickerField>(*this));
    addAssName(".input-field");
    addAssName(".number-picker");

	mTextField->setExpanding({ 1, 1 });
	connect(mTextField->focusState, this, [&](bool c)
	{
		if (c)
			onFocusAcquired();
		else
			onFocusLost();
	});
	
	auto c = _new<AViewContainer>();
    c->addAssName(".up-down-wrapper");
	auto up = _new<AButton>() let { it->setDefault(); it  << ".up"; };
	auto down = _new<AButton>() let { it->setDefault(); it  << ".down"; };
	c->setLayout(_new<AVerticalLayout>());
	c->setExpanding({ 0, 0 });
	c->addView(up);
	c->addView(down);

	connect(up->clicked, me::increase);
	connect(down->clicked, me::decrease);

	connect(mTextField->textChanged, this, [&]()
	{
		int v = getValue();
		if (v < mMin)
		{
			v = mMin;
			Platform::playSystemSound(Platform::S_ASTERISK);
		} else if (v > mMax)
		{
			v = mMax;
			Platform::playSystemSound(Platform::S_ASTERISK);
		} else
		{
			emit valueChanged(v);
			return;
		}
		mTextField->setSignalsEnabled(false);
		setValue(v);
		mTextField->setSignalsEnabled(true);
		emit valueChanged(v);
	});

	connect(mTextField->textChanging, this, [&]()
	{
		emit valueChanging();
	});
	
	addView(c);
}

int ANumberPicker::getContentMinimumHeight(ALayoutDirection layout)
{
	return AViewContainer::getContentMinimumHeight(ALayoutDirection::NONE);
}

void ANumberPicker::setValue(int v)
{
	mTextField->setText(AString::number(v));
    redraw();
}

int ANumberPicker::getValue() const
{
	return mTextField->text().toInt().valueOr(0);
}

void ANumberPicker::setMin(int min)
{
	mMin = min;
	if (getValue() < min)
		setValue(min);
}

void ANumberPicker::setMax(int max)
{
	mMax = max;
	if (getValue() > max)
		setValue(max);
}


void ANumberPicker::increase() {
    changeBy(AInput::isKeyDown(AInput::LCONTROL) ? 10 : 1);
}

void ANumberPicker::decrease() {
    changeBy(AInput::isKeyDown(AInput::LCONTROL) ? -10 : -1);
}

void ANumberPicker::changeBy(int v) {
    setValue(getValue() + v);
    emit valueChanging();
}
