﻿#include "AAbstractTextField.h"


#include "AUI/Platform/Platform.h"
#include "AUI/Render/Render.h"
#include "AUI/Render/RenderHints.h"

ATimer& AAbstractTextField::blinkTimer()
{
	static ATimer t(500);
	if (!t.isStarted())
	{
		t.start();
	}
	return t;
}

void AAbstractTextField::updateCursorBlinking()
{
	blinkTimer().restart();
	mCursorBlinkVisible = true;
	mCursorBlinkCount = 0;
}

void AAbstractTextField::updateCursorPos()
{
	auto absoluteCursorPos = -mHorizontalScroll + int(getFontStyle().getWidth(getContentsPasswordWrap().mid(0, mCursorIndex)));

	const int SCROLL_ADVANCEMENT = getContentWidth() * 4 / 10;

	if (absoluteCursorPos < 0)
	{
		mHorizontalScroll += absoluteCursorPos - SCROLL_ADVANCEMENT;
	}
	else if (absoluteCursorPos >= getContentWidth())
	{
		mHorizontalScroll += absoluteCursorPos - getContentWidth() + SCROLL_ADVANCEMENT;
	}
	mHorizontalScroll = glm::clamp(mHorizontalScroll, 0, glm::max(int(getFontStyle().getWidth(getContentsPasswordWrap())) - getContentWidth() + 1, 0));
}

unsigned AAbstractTextField::getCursorIndexByPos(int posX)
{
	posX = posX - (mPadding.left - mHorizontalScroll);
	if (posX <= 0)
		return 0;
	return getFontStyle().font->trimStringToWidth(getContentsPasswordWrap(), posX, getFontStyle().size, getFontStyle().fontRendering).length();
}

int AAbstractTextField::getPosByIndex(int index)
{
    return -mHorizontalScroll + int(getFontStyle().getWidth(getContentsPasswordWrap().mid(0, index)));
}

bool AAbstractTextField::hasSelection()
{
	return mCursorIndex != mCursorSelection && mCursorSelection != -1;
}


AAbstractTextField::AAbstractTextField()
{
	connect(blinkTimer().fired, this, [&]()
	{
		if (hasFocus() && mCursorBlinkCount < 60) {
			mCursorBlinkVisible = !mCursorBlinkVisible;
			mCursorBlinkCount += 1;
			redraw();
		}
	});
	AVIEW_CSS;
}


AAbstractTextField::~AAbstractTextField()
{
}

AAbstractTextField::Selection AAbstractTextField::getSelection()
{
	return { glm::min(mCursorIndex, mCursorSelection), glm::max(mCursorIndex, mCursorSelection) };
}


int AAbstractTextField::getContentMinimumHeight()
{
	return getFontStyle().size;
}

void AAbstractTextField::onKeyDown(AInput::Key key)
{
	onKeyRepeat(key);
}

void AAbstractTextField::onKeyRepeat(AInput::Key key)
{
	if (AInput::isKeyDown(AInput::LButton))
		return;


	if (!AInput::isKeyDown(AInput::LShift) && !AInput::isKeyDown(AInput::RShift))
	{
		mCursorSelection = -1;
	}
	else if (mCursorSelection == -1)
	{
		mCursorSelection = mCursorIndex;
	}
	
	mTextChangedFlag = true;
	switch (key)
	{
	case AInput::Delete:
		if (hasSelection()) {
			auto sel = getSelection();
			mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
			mCursorSelection = -1;
			mCursorIndex = sel.begin;
		} else
		{
			if (mCursorIndex < mContents.length())
			{
				mContents.removeAt(mCursorIndex);
			}
		}
		break;

	case AInput::Left:
		if (mCursorIndex)
			mCursorIndex -= 1;
		break;

	case AInput::Right:
		if (mCursorIndex < mContents.length())
			mCursorIndex += 1;
		break;

	case AInput::Home:
		mCursorIndex = 0;
		break;
	case AInput::End:
		mCursorIndex = mContents.length();
		break;
	default:
		return;
	}

	emit textChanging();
	
	updateCursorPos();
	
	redraw();
}

void AAbstractTextField::onCharEntered(wchar_t c)
{
	if (AInput::isKeyDown(AInput::LButton))
		return;
	if (c == '\n' || c == '\r')
        return;

	mTextChangedFlag = true;
	AString contentsCopy = mContents;
	auto cursorIndexCopy = mCursorIndex;

	if (hasSelection()) {
		auto sel = getSelection();
		mContents.erase(mContents.begin() + sel.begin, mContents.begin() + sel.end);
		
		switch (c)
		{
		case '\b':
			mCursorIndex = sel.begin;
			break;
		default:
			mContents.insert(sel.begin, c);
			mCursorIndex = sel.begin + 1;
		}
		mCursorSelection = -1;
	} else {
		switch (c)
		{
		case '\b':
			if (mCursorIndex != 0) {
				mContents.removeAt(--mCursorIndex);
			}
			break;
		default:
			mContents.insert(mCursorIndex++, c);
		}
		if (!isValidText(mContents))
		{
			mContents = std::move(contentsCopy);
			mCursorIndex = cursorIndexCopy;
			Platform::playSystemSound(Platform::S_ASTERISK);
		}
	}
	emit textChanging();
	updateCursorBlinking();
	updateCursorPos();

	if (!AInput::isKeyDown(AInput::LShift) && !AInput::isKeyDown(AInput::RShift))
	{
		mCursorSelection = -1;
	}
	
	redraw();
}

void AAbstractTextField::render()
{
	AView::render();

	int absoluteBegin, absoluteEnd;
	auto absoluteCursorPos = getPosByIndex(mCursorIndex);

	// выделение
	if (hasSelection() && hasFocus())
	{
		auto absoluteSelectionPos = getPosByIndex(mCursorSelection);

		absoluteBegin = glm::min(absoluteCursorPos, absoluteSelectionPos);
		absoluteEnd = glm::max(absoluteCursorPos, absoluteSelectionPos);

		Render::instance().setFill(Render::FILL_SOLID);
		RenderHints::PushColor c;
		Render::instance().setColor(AColor(1.f) - AColor(0x0078d700u));
		Render::instance().drawRect(mPadding.left + absoluteBegin, mPadding.top, absoluteEnd - absoluteBegin + 1, getFontStyle().size + 3);
	}
	Render::instance().drawString(mPadding.left - mHorizontalScroll, mPadding.top, getContentsPasswordWrap(), getFontStyle());
	Render::instance().setFill(Render::FILL_SOLID);
	Render::instance().setColor({ 1, 1, 1, 1 });
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	
	if (hasSelection() && hasFocus())
	{
		Render::instance().drawRect(mPadding.left + absoluteBegin, mPadding.top, absoluteEnd - absoluteBegin + 1, getFontStyle().size + 3);
	}
	
	// курсор
	if (hasFocus() && mCursorBlinkVisible) {

		Render::instance().drawRect(mPadding.left + absoluteCursorPos,
			mPadding.top, 1, getFontStyle().size + 3);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void AAbstractTextField::onFocusLost()
{
	AView::onFocusLost();
	mCursorSelection = -1;
	if (mTextChangedFlag)
	{
		mTextChangedFlag = false;
		emit textChanged(mContents);
	}
	
}

void AAbstractTextField::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	if (button == AInput::LButton) {
		mCursorSelection = mCursorIndex = getCursorIndexByPos(pos.x);
	}
	updateCursorBlinking();
}

void AAbstractTextField::onMouseMove(glm::ivec2 pos)
{
	if (AInput::isKeyDown(AInput::LButton)) {
		mCursorIndex = getCursorIndexByPos(pos.x);
		redraw();
	}
}

void AAbstractTextField::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
	if (button == AInput::LButton)
	{
		if (mCursorSelection == mCursorIndex)
		{
			mCursorSelection = -1;
		}
	}
}

void AAbstractTextField::setText(const AString& t)
{
	mContents = t;
	mCursorIndex = t.length();
	mCursorSelection = 0;
	updateCursorBlinking();

	emit textChanged;
}

AString AAbstractTextField::getContentsPasswordWrap() {
    if (mIsPasswordTextField) {
        AString s;
        for (auto c : mContents)
            s += "•";
        return s;
    }
    return mContents;
}
