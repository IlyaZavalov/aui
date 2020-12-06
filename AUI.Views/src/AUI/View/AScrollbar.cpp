//
// Created by alex2 on 06.12.2020.
//

#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include "AScrollbar.h"

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
            forwardButton->setCss("background: url(':win/svg/sb-right.svg')");
            backwardButton->setCss("background: url(':win/svg/sb-left.svg')");
            break;
        case LayoutDirection::VERTICAL:
            setLayout(_new<AVerticalLayout>());
            forwardButton->setCss("background: url(':win/svg/sb-down.svg')");
            backwardButton->setCss("background: url(':win/svg/sb-top.svg')");
            break;
    }
    auto handle = _new<AScrollbarHandle>();

    addView(backwardButton);
    addView(handle);
    addView(forwardButton);
}
