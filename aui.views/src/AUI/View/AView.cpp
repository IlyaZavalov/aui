/*
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

#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Animator/AAnimator.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/kAUI.h>
#include <AUI/ASS/AStylesheet.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/callables.h>
#include <AUI/Traits/iterators.h>
#include <stack>

#include "AUI/Platform/ADesktop.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Util/Factory.h"
#include "ALabel.h"

// windows.h
#undef max
#undef min


ABaseWindow* AView::getWindow()
{

    AView* parent = nullptr;

    for (AView* target = this; target; target = target->mParent) {
        parent = target;
    }

    return dynamic_cast<ABaseWindow*>(parent);
}

AView::AView()
{
    AUI_ASSERT_UI_THREAD_ONLY()
    aui::zero(mAss);
    setSlotsCallsOnlyOnMyThread(true);
}

void AView::redraw()
{
    if (mRedrawRequested) {
        return;
    }
    mRedrawRequested = true;
    AUI_ASSERT_UI_THREAD_ONLY()
    AUI_NULLSAFE(getWindow())->flagRedraw(); else AUI_NULLSAFE(AWindow::current())->flagRedraw();

}
void AView::requestLayoutUpdate()
{
    AUI_ASSERT_UI_THREAD_ONLY()
    AUI_NULLSAFE(getWindow())->flagUpdateLayout(); else AUI_NULLSAFE(AWindow::current())->flagUpdateLayout();
}

void AView::drawStencilMask()
{
    switch (mOverflowMask) {
        case AOverflowMask::ROUNDED_RECT:
            if (mBorderRadius > 0 && mPadding.horizontal() == 0 && mPadding.vertical() == 0) {
                Render::roundedRect(ASolidBrush{},
                                    {mPadding.left, mPadding.top},
                                    {getWidth() - mPadding.horizontal(), getHeight() - mPadding.vertical()},
                                    mBorderRadius);
            } else {
                Render::rect(ASolidBrush{},
                             {mPadding.left, mPadding.top},
                             {getWidth() - mPadding.horizontal(), getHeight() - mPadding.vertical()});
            }
            break;

        case AOverflowMask::BACKGROUND_IMAGE_ALPHA:
            if (auto s = mAss[int(ass::decl::DeclarationSlot::BACKGROUND_IMAGE)]) {
                s->renderFor(this);
            }
            break;
    }
}

void AView::postRender() {
    if (mAnimator)
        mAnimator->postRender(this);
    popStencilIfNeeded();
}

void AView::popStencilIfNeeded() {
    if (getOverflow() == AOverflow::HIDDEN)
    {
        /*
         * If the AView's Overflow set to Overflow::HIDDEN AView pushed it's mask into the stencil buffer but AView
         * cannot return stencil buffer to the previous state by itself because of C++ restrictions. We should also
         * apply mask AFTER transform updated and BEFORE rendering AView content. The only way to return the stencil
         * back is place it here, after rendering AView.
         */
        RenderHints::popMask([&]() {
            drawStencilMask();
        });
    }
}
void AView::render()
{
    if (mAnimator)
        mAnimator->animate(this);

    {
        ensureAssUpdated();

        // draw list
        for (unsigned i = 0; i < int(ass::decl::DeclarationSlot::COUNT); ++i) {
            if (i == int(ass::decl::DeclarationSlot::BACKGROUND_EFFECT)) continue;
            if (auto w = mAss[i]) {
                w->renderFor(this);
            }
        }
    }

    // stencil
    if (mOverflow == AOverflow::HIDDEN)
    {
        RenderHints::pushMask([&]() {
            drawStencilMask();
        });
    }

    if (auto w = mAss[int(ass::decl::DeclarationSlot::BACKGROUND_EFFECT)]) {
        w->renderFor(this);
    }
    mRedrawRequested = false;
}

void AView::invalidateAllStyles()
{
    assert(("invalidateAllStyles requires mAssHelper to be initialized", mAssHelper != nullptr));
    mCursor = ACursor::DEFAULT;
    mOverflow = AOverflow::VISIBLE;
    mMargin = {};
    mMinSize = {};
    mBorderRadius = 0.f;
    //mForceStencilForBackground = false;
    mMaxSize = glm::ivec2(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    mFontStyle = {};

    auto applyStylesheet = [this](const AStylesheet& sh) {
        for (const auto& r : sh.getRules()) {
            if (r.getSelector().isPossiblyApplicable(this)) {
                mAssHelper->mPossiblyApplicableRules << &r;
                r.getSelector().setupConnections(this, mAssHelper);
            }
        }
    };

    applyStylesheet(AStylesheet::global());

    std::stack<AView*> viewTree;

    for (auto target = this; target != nullptr; target = target->getParent()) {
        if (target->mExtraStylesheet) {
            viewTree.push(target);
        }
    }

    while (!viewTree.empty()) {
        applyStylesheet(*viewTree.top()->mExtraStylesheet);
        viewTree.pop();
    }

    invalidateStateStyles();
}

void AView::invalidateStateStyles() {
    aui::zero(mAss);
    if (!mAssHelper) return;
    mAssHelper->state.backgroundCropping.size.reset();
    mAssHelper->state.backgroundCropping.offset.reset();
    mAssHelper->state.backgroundImage.reset();
    mAssHelper->state.backgroundUrl.dpiMargin.reset();
    mAssHelper->state.backgroundUrl.overlayColor.reset();
    mAssHelper->state.backgroundUrl.rep.reset();
    mAssHelper->state.backgroundUrl.scale.reset();
    mAssHelper->state.backgroundUrl.sizing.reset();
    mAssHelper->state.backgroundUrl.url.reset();


    for (auto& r : mAssHelper->mPossiblyApplicableRules) {
        if (r->getSelector().isStateApplicable(this)) {
            applyAssRule(* r);
        }
    }
    applyAssRule(mCustomStyleRule);

    redraw();
}


float AView::getTotalFieldHorizontal() const
{
    return mPadding.horizontal() + mMargin.horizontal();
}

float AView::getTotalFieldVertical() const
{
    return mPadding.vertical() + mMargin.vertical();
}

int AView::getContentMinimumWidth(ALayoutDirection layout)
{
    return 0;
}

int AView::getContentMinimumHeight(ALayoutDirection layout)
{
    return 0;
}

bool AView::hasFocus() const
{
    return mHasFocus;
}

int AView::getMinimumWidth(ALayoutDirection layout)
{
    ensureAssUpdated();
    return (mFixedSize.x == 0 ? ((glm::max)(getContentMinimumWidth(layout), mMinSize.x) + mPadding.horizontal()) : mFixedSize.x);
}

int AView::getMinimumHeight(ALayoutDirection layout)
{
    ensureAssUpdated();
    return (mFixedSize.y == 0 ? ((glm::max)(getContentMinimumHeight(layout), mMinSize.y) + mPadding.vertical()) : mFixedSize.y);
}

void AView::getTransform(glm::mat4& transform) const
{
    transform = glm::translate(transform, glm::vec3{ getPosition(), 0.f });
}

AFontStyle& AView::getFontStyle()
{
    return mFontStyle;
}


void AView::pack()
{
    setSize({getMinimumWidth(parentLayoutDirection()), getMinimumHeight(parentLayoutDirection())});
}

void AView::addAssName(const AString& assName)
{
    mAssNames << assName;
    assert(("empty ass name" && !assName.empty()));
    invalidateAssHelper();
}

void AView::invalidateAssHelper() { mAssHelper = nullptr; }

void AView::removeAssName(const AString& assName)
{
    mAssNames >> assName;
    assert(("empty ass name" && !assName.empty()));
    invalidateAssHelper();
}

void AView::ensureAssUpdated()
{
    if (mAssHelper == nullptr)
    {
        mAssHelper = _new<AAssHelper>();
        connect(customCssPropertyChanged, mAssHelper,
                &AAssHelper::onInvalidateStateAss);
        connect(mAssHelper->invalidateFullAss, this, [&]()
        {
            mAssHelper = nullptr;
        });
        connect(mAssHelper->invalidateStateAss, me::invalidateStateStyles);

        invalidateAllStyles();
    }
}

void AView::onMouseEnter()
{
    if (AWindow::shouldDisplayHoverAnimations()) {
        mHovered.set(this, true);
    }
}


void AView::onMouseMove(glm::ivec2 pos)
{
}

void AView::onMouseLeave()
{
    if (AWindow::shouldDisplayHoverAnimations()) {
        mHovered.set(this, false);
    }
}


void AView::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
    mPressed.set(this, true);

    /**
     * If button is pressed on this view, we want to know when the mouse will be released even if mouse outside
     * this view and even the mouse outside the window so we can guarantee that if we got a mouse press event, we will
     * get a mouse release event too.
     */
    if (auto w = AWindow::current())
    {
        if (w != this) {
            connect(w->mouseReleased, this, [&]()
                {
                    auto selfHolder = sharedPtr();
                    AThread::current()->enqueue([&, selfHolder = std::move(selfHolder)]()
                        {
                            // to be sure that isPressed will be false.
                            if (mPressed) {
                                onMouseReleased(pos, button);
                            }
                        });
                    disconnect();
                });
        }
    }
}

void AView::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
    mPressed.set(this, false);
    emit clickedButton(button);
    switch (button)
    {
        case AInput::LBUTTON:
            emit clicked();
            break;
        case AInput::RBUTTON:
            emit clickedRight();
            break;
    }
}

void AView::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button)
{
    emit doubleClicked(button);
}

void AView::onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) {

}

void AView::onKeyDown(AInput::Key key)
{
    if (key == AInput::TAB) {
        AWindow::current()->focusNextView();
    }
}

void AView::onKeyRepeat(AInput::Key key)
{
}

void AView::onKeyUp(AInput::Key key)
{
}

void AView::onFocusAcquired()
{
    mHasFocus.set(this, true);
}

void AView::onFocusLost()
{
    mHasFocus.set(this, false);
}

void AView::onCharEntered(wchar_t c)
{

}

bool AView::handlesNonMouseNavigation() {
    return false;
}

void AView::setEnabled(bool enabled)
{
    mDirectlyEnabled = enabled;
    updateEnableState();
}
void AView::updateEnableState()
{
    mEnabled.set(this, mDirectlyEnabled && mParentEnabled);
    emit customCssPropertyChanged();
    setSignalsEnabled(mEnabled);
    emit customCssPropertyChanged();
    redraw();
}

void AView::setAnimator(const _<AAnimator>& animator) {
    mAnimator = animator;
    if (mAnimator)
        mAnimator->setView(this);
}

glm::ivec2 AView::getPositionInWindow() const {
    glm::ivec2 p(0);
    for (const AView* i = this; i && i->getParent(); i = i->getParent()) {
        p += i->getPosition();
    }
    return p;
}


void AView::setPosition(glm::ivec2 position) {
    mPosition = position;
}
void AView::setSize(glm::ivec2 size)
{
    /*
    int minWidth = getContentMinimumWidth();
    int minHeight = getContentMinimumHeight();

    // some bias is allowed
    assert(minWidth <= width + 5);
    assert(minHeight <= height + 5);
*/
    if (mFixedSize.x != 0)
    {
        mSize.x = mFixedSize.x;
    }
    else
    {
        mSize.x = size.x;
        if (mMinSize.x != 0)
            mSize.x = glm::max(mMinSize.x, mSize.x);
    }
    if (mFixedSize.y != 0)
    {
        mSize.y = mFixedSize.y;
    }
    else
    {
        mSize.y = size.y;
        if (mMinSize.y != 0)
            mSize.y = glm::max(mMinSize.y, mSize.y);
    }
    mSize = glm::min(mSize, mMaxSize);
}

void AView::setGeometry(int x, int y, int width, int height) {
    setPosition({ x, y });
    setSize({width, height});
}

bool AView::consumesClick(const glm::ivec2& pos) {
    return true;
}

void AView::focus() {
    // holding reference here
    auto mySharedPtr = sharedPtr();
    auto window = AWindow::current();

    try {
        auto windowSharedPtr = window->sharedPtr(); // may throw bad_weak

        ui_threadX [window, mySharedPtr = std::move(mySharedPtr), windowSharedPtr = std::move(windowSharedPtr)]() {
            window->setFocusedView(mySharedPtr);
        };
    } catch (...) {
        window->setFocusedView(mySharedPtr);
    }
}

Visibility AView::getVisibilityRecursive() const {
    if (mVisibility == Visibility::GONE)
        return Visibility::GONE;

    int v = int(mVisibility);

    for (auto target = mParent; target; target = target->mParent) {
        if (v < int(target->mVisibility)) {
            v = int(target->mVisibility);
            if (v == int(Visibility::GONE)) {
                return Visibility::GONE;
            }
        }
    }

    return static_cast<Visibility>(v);
}

void AView::onDpiChanged() {
    mAssHelper = nullptr;
}

void AView::invalidateFont() {

}
void AView::notifyParentEnabledStateChanged(bool enabled) {
    mParentEnabled = enabled;
    updateEnableState();
}

bool AView::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    return transformGestureEventsToDesktop(origin, event);
}
bool AView::transformGestureEventsToDesktop(const glm::ivec2& origin, const AGestureEvent& event) {
    return std::visit(aui::lambda_overloaded {
        [&](const AFingerDragEvent& e) {
            onMouseWheel(origin, e.delta);
            return true;
        },
        [&](const ALongPressEvent& e) {
            onMousePressed(origin, AInput::RBUTTON);
            onMouseReleased(origin, AInput::RBUTTON);
            return true;
        },
        [&](const auto& e) {
            return false;
        }
    }, event);
}

void AView::applyAssRule(const RuleWithoutSelector& rule) {
    for (const auto& d : rule.getDeclarations()) {
        auto slot = d->getDeclarationSlot();
        if (slot != ass::decl::DeclarationSlot::NONE) {
            mAss[int(slot)] = d->isNone() ? nullptr : d.get();
        }
        d->applyFor(this);
    }
}

ALayoutDirection AView::parentLayoutDirection() const noexcept {
    if (mParent == nullptr) return ALayoutDirection::NONE;
    if (mParent->getLayout() == nullptr) return ALayoutDirection::NONE;
    return mParent->getLayout()->getLayoutDirection();
}

