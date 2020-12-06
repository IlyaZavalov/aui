//
// Created by alex2772 on 12/6/20.
//

#include "AScrollArea.h"
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/AMetric.h>

class AScrollAreaContainer: public AViewContainer {
public:

    int getContentMinimumWidth() override {
        return 30_dp;
    }

    int getContentMinimumHeight() override {
        return 30_dp;
    }

};

AScrollArea::AScrollArea() {
    setLayout(_new<AAdvancedGridLayout>(2, 2));
    addView(mContentContainer = _new<AScrollAreaContainer>());
    addView(mVerticalScrollbar = _new<AScrollbar>(LayoutDirection::VERTICAL));
    addView(mHorizontalScrollbar = _new<AScrollbar>(LayoutDirection::HORIZONTAL));

    mContentContainer->setCss("overflow: hidden");
    mContentContainer->setExpanding({2, 2});

    setExpanding({2, 2});
}

void AScrollArea::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    mVerticalScrollbar->setScrollDimensions(mContentContainer->getHeight(), mContentContainer->getMinimumHeight())
}
