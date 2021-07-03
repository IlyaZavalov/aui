//
// Created by alex2772 on 6/27/21.
//

#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "ARulerArea.h"
#include "ARulerView.h"
#include "AScrollbar.h"
#include <AUI/ASS/ASS.h>

using namespace ass;

class WrapperContainer: public AViewContainer {
private:
    _<AView> mWrappedView;

public:
    WrapperContainer(const _<AView>& wrappedView) : mWrappedView(wrappedView) {
        addView(mWrappedView);
    }

};

ARulerArea::ARulerArea(const _<AView>& wrappedView) : mWrappedView(wrappedView) {

    auto wrapper = _new<WrapperContainer>(mWrappedView) with_style { Expanding { 2, 2 } } << ".arulerarea-content";

    setContents(
        Horizontal {
            Vertical {
                _container<AAdvancedGridLayout>({
                    _new<ALabel>("dp") << ".arulerarea-unit",
                    mHorizontalRuler = _new<ARulerView>(LayoutDirection::HORIZONTAL),
                    mVerticalRuler = _new<ARulerView>(LayoutDirection::VERTICAL),
                    wrapper with_style { Overflow::HIDDEN },
                }, 2, 2) with_style { Expanding { 2, 2 } },
                _new<AScrollbar>(LayoutDirection::HORIZONTAL),

            } with_style { Expanding { 2, 2 } },
            _new<AScrollbar>(),
        }
    );

    setExpanding({10, 10});


    mWrappedView->setSize(mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight());
}

void ARulerArea::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    mWrappedView->setSize(mWrappedView->getMinimumWidth(), mWrappedView->getMinimumHeight());
    setWrappedViewPosition((getSize() - mWrappedView->getSize()) / 2);
}

void ARulerArea::setWrappedViewPosition(const glm::ivec2& pos) {
    mWrappedView->setPosition(pos);
    mHorizontalRuler->setOffsetPx(pos.x);
    mVerticalRuler->setOffsetPx(pos.y);
}

void ARulerArea::render() {
    AViewContainer::render();


    glDisable(GL_STENCIL_TEST);
    // cursor display
    {
        Render::inst().drawString(mMousePos.x + 2_dp, 20_dp, AString::number(int(operator""_px(mMousePos.x).getValueDp())));
        Render::inst().drawString(20_dp, mMousePos.y + 20_dp, AString::number(int(operator""_px(mMousePos.y).getValueDp())));

        Render::inst().setFill(Render::FILL_SOLID);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        Render::inst().drawRect(mMousePos.x, 0.f, 1, mMousePos.y);
        Render::inst().drawRect(0.f, mMousePos.y, mMousePos.x, 1);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glEnable(GL_STENCIL_TEST);
}

void ARulerArea::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    mMousePos = pos;
    redraw();
}
