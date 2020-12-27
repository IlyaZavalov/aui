#pragma once


#include <AUI/View/ALabel.h>
#include <AUI/Designer/DesignerRegistrator.h>

class ViewLabel: public ALabel {
private:
    aui::detail::DesignerRegistrationBase* mRegistrationBase;
    bool mAboutToEmitView = false;

public:

    ViewLabel(aui::detail::DesignerRegistrationBase* registrationBase);

    void onMouseMove(glm::ivec2 pos) override;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
};

