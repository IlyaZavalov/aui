//
// Created by alex2 on 06.12.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include "AScrollbar.h"
#include "ASpacer.h"

class AScrollbarButton: public AView {
public:
    AScrollbarButton() {
        AVIEW_CSS;
    }
};
class AScrollbarHandle: public AView {
public:
    AScrollbarHandle() {
        AVIEW_CSS;
    }
};

AScrollbar::AScrollbar(LayoutDirection direction) : mDirection(direction) {
    AVIEW_CSS;
    auto forwardButton = _new<AScrollbarButton>();
    auto backwardButton = _new<AScrollbarButton>();

    switch (direction) {
        case LayoutDirection::HORIZONTAL:
            setLayout(_new<AHorizontalLayout>());
            forwardButton->setCss("background: url(':win/svg/sb-right.svg');"
                                  "width: 17em;"
                                  "height: 15em;");
            backwardButton->setCss("background: url(':win/svg/sb-left.svg');"
                                   "width: 17em;"
                                   "height: 15em;");
            break;
        case LayoutDirection::VERTICAL:
            setLayout(_new<AVerticalLayout>());
            forwardButton->setCss("background: url(':win/svg/sb-down.svg');"
                                  "width: 15em;"
                                  "height: 17em;");
            backwardButton->setCss("background: url(':win/svg/sb-top.svg');"
                                   "width: 15em;"
                                   "height: 17em;");
            break;
    }
    mHandle = _new<AScrollbarHandle>();

    addView(backwardButton);
    addView(mOffsetSpacer = _new<ASpacer>() let (ASpacer, {setMinimumSize({0, 0});}));
    addView(mHandle);
    addView(_new<ASpacer>() let (ASpacer, {setMinimumSize({0, 0});}));
    addView(forwardButton);
}

void AScrollbar::setOffset(size_t o) {
    switch (mDirection) {
        case LayoutDirection::HORIZONTAL:
            mOffsetSpacer->setSize(o, 0);
            break;
        case LayoutDirection::VERTICAL:
            mOffsetSpacer->setSize(0, o);
            break;
    }
}

void AScrollbar::setScrollDimensions(size_t viewportSize, size_t fullSize) {
    mViewportSize = viewportSize;
    mFullSize = fullSize;

    updateScrollHandleSize();
}

void AScrollbar::updateScrollHandleSize() {
    mHandle->setSize()
}
