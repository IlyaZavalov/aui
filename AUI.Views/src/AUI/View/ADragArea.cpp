//
// Created by alex2 on 29.11.2020.
//

#include "ADragArea.h"
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Platform/ADesktop.h>

ADragArea::ADragArea() {
    setLayout(_new<AStackedLayout>());
}

std::tuple<ADragArea*, AViewContainer*> ADragArea::ADraggableHandle::getDragAreaAndDraggingView() const {
    AViewContainer* potentionalDragArea = mParent;
    AViewContainer* potentionalDraggingView = mParent;

    while (potentionalDragArea) {
        potentionalDraggingView = potentionalDragArea;
        potentionalDragArea = potentionalDragArea->getParent();
        if (auto dragArea = dynamic_cast<ADragArea*>(potentionalDragArea)) {
            return {dragArea, potentionalDraggingView};
        }
    }


    return {nullptr, potentionalDraggingView};
}

void ADragArea::ADraggableHandle::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AView::onMousePressed(pos, button);

    auto [dragArea, container] = getDragAreaAndDraggingView();
    connect(mouseMove, dragArea, &ADragArea::handleMouseMove);
    dragArea->startDragging(container);
}

void ADragArea::ADraggableHandle::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);
    auto [dragArea,_] = getDragAreaAndDraggingView();
    mouseMove.clearAllConnectionsWith(dragArea);
    dragArea->endDragging();
}

void ADragArea::ADraggableHandle::onMouseMove(glm::ivec2 pos) {
    AView::onMouseMove(pos);
    emit mouseMove(pos);
}

void ADragArea::startDragging(AViewContainer* container) {
    for (auto& v : getViews()) {
        if (v.get() == container) {
            mDraggedView = v;
            mInitialMousePos = ADesktop::getMousePosition() - container->getPosition();
            break;
        }
    }
}

void ADragArea::handleMouseMove() {
    if (auto s = mDraggedView.lock()) {
        auto newPos = ADesktop::getMousePosition() - mInitialMousePos;
        s->setPosition(glm::clamp(newPos, glm::ivec2{0, 0}, getSize() - s->getSize()));
        redraw();
    }
}

void ADragArea::endDragging() {
    mDraggedView.reset();
}
