#pragma once


#include <AUI/View/ADragArea.h>
#include "FakeWindow.h"

class DesignerArea: public AViewContainer {
private:
    _<FakeWindow> mFakeWindow;

    _<AView> mDraggingView;
    glm::ivec2 mDraggingOffset;

public:
    DesignerArea();

    void beginDraggingView(const glm::ivec2& offset, const _<AView>& view);
    void endDraggingView();

    void onMouseMove(glm::ivec2 pos) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
};

