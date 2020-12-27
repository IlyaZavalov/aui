//
// Created by alex2772 on 12/27/20.
//

#include <Window/DesignerWindow.h>
#include "ViewLabel.h"
#include "DesignerArea.h"

ViewLabel::ViewLabel(aui::detail::DesignerRegistrationBase* registrationBase):
    ALabel(registrationBase->name()),
    mRegistrationBase(registrationBase) {

}

void ViewLabel::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AView::onMousePressed(pos, button);
    mAboutToEmitView = true;
}

void ViewLabel::onMouseMove(glm::ivec2 pos) {
    AView::onMouseMove(pos);
    if (mAboutToEmitView) {
        mAboutToEmitView = false;
        auto view = mRegistrationBase->instanciate();
        view->setSize(view->getMinimumWidth(), view->getMinimumHeight());
        Autumn::get<DesignerArea>()->beginDraggingView(view->getSize() / 2, view);
        AWindow::current()->setFocusedView(Autumn::get<DesignerArea>());
    }
}

void ViewLabel::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AView::onMouseReleased(pos, button);
    Autumn::get<DesignerArea>()->endDraggingView();
    mAboutToEmitView = false;
}
