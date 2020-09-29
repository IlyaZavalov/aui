﻿#include "AViewContainer.h"
#include "AView.h"
#include "AUI/Render/Render.h"
#include <glm/gtc/matrix_transform.hpp>

#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AMetric.h"


unsigned char stencilDepth = 0;

void AViewContainer::drawView(const _<AView>& view)
{
	if (view->getVisibility() == V_VISIBLE) {
		RenderHints::PushState s;
		glm::mat4 t(1.f);
		view->getTransform(t);
		Render::instance().setTransform(t);

		try {
			view->render();
		}
		catch (...) {}

		if (view->getOverflow() == OF_HIDDEN)
		{
		    /*
		     * Если у AView есть ограничение по Overflow, то он запушил свою маску в буфер трафарета, но он не может
		     * вернуть буфер трафарета к прежнему состоянию из-за ограничений C++, поэтому заносить маску нужно
		     * после обновлений преобразований (позиция, поворот), но перед непосредственным рендером содержимого AView
		     * (то есть, в <code>AView::render</code>), а возвращать трафарет к предыдущему состоянию можно только
		     * здесь, после того, как AView будет полностью отрендерен.
		     */
		    RenderHints::PushMask::popMask([&]() {
		       view->drawStencilMask();
		    });
		}
	}
}


void AViewContainer::userProcessStyleSheet(
	const std::function<void(css, const std::function<void(property)>&)>& processor)
{
	processor(css::T_AUI_SPACING, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mLayout->setSpacing(AMetric(p->getArgs()[0]).getValuePx());
		}
	});
}

AViewContainer::AViewContainer()
{
	AVIEW_CSS;
}

AViewContainer::~AViewContainer()
{
	//Stylesheet::instance().invalidateCache();
}

void AViewContainer::addView(_<AView> view)
{
	mViews << view;
	view->mParent = this;
	if (mLayout)
		mLayout->addView(view);
}

void AViewContainer::removeView(_<AView> view)
{
	mViews.remove(view);
	if (mLayout)
		mLayout->removeView(view);
}

void AViewContainer::render()
{
	AView::render();
	drawViews(mViews.begin(), mViews.end());
}

void AViewContainer::onMouseEnter()
{
	AView::onMouseEnter();
}

void AViewContainer::onMouseMove(glm::ivec2 pos)
{
	AView::onMouseMove(pos);

	for (auto& view : mViews)
	{
		if (!view->isEnabled())
			continue;
		
		auto mousePos = pos - view->getPosition();
		bool hovered = mousePos.x >= 0 && mousePos.y >= 0 && mousePos.x < view->getSize().x && mousePos.y < view->getSize().y;
		if (view->isMouseHover() != hovered)
		{
			if (hovered)
			{
				view->onMouseEnter();
			} else
			{

				view->onMouseLeave();
			}
		}
		if (hovered)
		{
			view->onMouseMove(mousePos);
		}
	}
}

void AViewContainer::onMouseLeave()
{
	AView::onMouseLeave();
	for (auto& view : mViews)
	{
		if (view->isMouseHover() && view->isEnabled())
			view->onMouseLeave();
	}
}

int AViewContainer::getContentMinimumWidth()
{
	if (mLayout)
	{
		return mLayout->getMinimumWidth();
	}
	return AView::getContentMinimumWidth();
}

int AViewContainer::getContentMinimumHeight()
{
	if (mLayout)
	{
		return mLayout->getMinimumHeight();
	}
	return AView::getContentMinimumHeight();
}


void AViewContainer::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	AView::onMousePressed(pos, button);

	auto p = getViewAt(pos);
	if (p && p->isEnabled()) {
		AWindow::current()->setFocusedView(p);
		p->onMousePressed(pos - p->getPosition(), button);
	}
}

void AViewContainer::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
	AView::onMouseReleased(pos, button);
	auto p = getViewAt(pos);
	if (p && p->isEnabled() && p->isMousePressed())
		p->onMouseReleased(pos - p->getPosition(), button);
}
void AViewContainer::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button)
{
	AView::onMouseDoubleClicked(pos, button);

	auto p = getViewAt(pos);
	if (p && p->isEnabled())
		p->onMouseDoubleClicked(pos - p->getPosition(), button);
}

bool AViewContainer::consumesClick(const glm::ivec2& pos) {
    auto p = getViewAt(pos);
    if (p)
        return p->consumesClick(pos - p->getPosition());
    return false;
}

void AViewContainer::setSize(int width, int height)
{
	AView::setSize(width, height);
	updateLayout();
}


void AViewContainer::setLayout(_<ALayout> layout)
{
	mViews.clear();
	mLayout = layout;
}

_<ALayout> AViewContainer::getLayout() const
{
	return mLayout;
}

_<AView> AViewContainer::getViewAt(glm::ivec2 pos, bool ignoreGone)
{
    _<AView> possibleOutput = nullptr;
	for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
	{
		auto view = *it;
		auto mousePos = pos - view->getPosition();

		if (mousePos.x >= 0 && mousePos.y >= 0 && mousePos.x < view->getSize().x && mousePos.y < view->getSize().y)
		{
			if (!ignoreGone || view->getVisibility() != V_GONE) {
			    possibleOutput = view;
                if (view->consumesClick(mousePos))
			        return view;
            }
		}
	}
	return possibleOutput;
}

_<AView> AViewContainer::getViewAtRecusrive(glm::ivec2 pos)
{
	_<AView> target = getViewAt(pos);
	if (!target)
		return nullptr;
	while (auto parent = _cast<AViewContainer>(target))
	{
		pos -= parent->getPosition();
		target = parent->getViewAt(pos);
		if (!target)
			return parent;
	}
	return target;
}

void AViewContainer::updateLayout()
{
    if (mLayout)
        mLayout->onResize(mPadding.left, mPadding.top,
                          getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
}

void AViewContainer::recompileCSS() {
    AView::recompileCSS();
    updateLayout();
}

void AViewContainer::setGeometry(int x, int y, int width, int height) {
    AView::setGeometry(x, y, width, height);
    updateLayout();
}

void AViewContainer::removeAllViews() {
    if (mLayout) {
        for (auto& x : getViews()) {
            mLayout->removeView(x);
        }
    }
    mViews.clear();
}

