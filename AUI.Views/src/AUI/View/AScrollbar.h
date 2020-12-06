//
// Created by alex2 on 06.12.2020.
//

#pragma once

#include <AUI/Util/LayoutDirection.h>
#include "AViewContainer.h"

class API_AUI_VIEWS AScrollbar: public AViewContainer {
private:
    LayoutDirection mDirection;

public:
    explicit AScrollbar(LayoutDirection direction);
};


