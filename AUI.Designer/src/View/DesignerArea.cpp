//
// Created by alex2772 on 12/27/20.
//

#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Platform/AWindow.h>
#include "DesignerArea.h"

DesignerArea::DesignerArea() {
    setLayout(_new<AStackedLayout>());
    addView(mFakeWindow = _new<FakeWindow>());
}

void DesignerArea::beginDraggingView(const glm::ivec2& offset, const _<AView>& view) {
    mDraggingOffset = offset;
    mDraggingView = view;
    addViewNoLayout(mDraggingView);
    connect(AWindow::current()->mouseReleased, this, [&]() {
       endDraggingView();
       AObject::disconnect();
    });
}

void DesignerArea::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    if (mDraggingView) {
        mDraggingView->setPosition(pos - mDraggingOffset);
        redraw();
    }
}

void DesignerArea::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
    endDraggingView();
}

void DesignerArea::endDraggingView() {
    mDraggingView = nullptr;
}

