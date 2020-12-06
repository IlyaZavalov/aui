#pragma once


#include "AViewContainer.h"
#include "AScrollbar.h"

class API_AUI_VIEWS AScrollArea: public AViewContainer {
private:
    _<AViewContainer> mContentContainer;
    _<AScrollbar> mVerticalScrollbar;
    _<AScrollbar> mHorizontalScrollbar;

public:
    AScrollArea();
    virtual ~AScrollArea() = default;

    const _<AViewContainer>& getContentContainer() const {
        return mContentContainer;
    }

    void setSize(int width, int height) override;
};

