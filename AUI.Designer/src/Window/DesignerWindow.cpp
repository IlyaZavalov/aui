//
// Created by alex2 on 05.12.2020.
//

#include "DesignerWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Platform/ACustomCaptionWindow.h>
#include <AUI/Designer/DesignerRegistrator.h>
#include <View/FakeWindow.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/Image/Drawables.h>
#include <AUI/View/APageView.h>
#include <AUI/View/APageSwitch.h>
#include <View/ViewLabel.h>
#include <View/DesignerArea.h>

DesignerWindow::DesignerWindow():
    AWindow("AUI Designer")
{
    setLayout(_new<AStackedLayout>());

    auto componentsList = _new<AScrollArea>();

    componentsList->getContentContainer()->setLayout(_new<AVerticalLayout>());

    for (auto& c : aui::detail::DesignerRegistrationBase::getRegistrations()) {
        auto l = _new<ViewLabel>(c);
        if (auto icon = Drawables::get(":designer/icons/" + c->name() + ".svg")) {
            l->setIcon(icon);
        } else {
            l->setIcon(Drawables::get(":designer/icons/unknown.svg"));
        }
        componentsList->getContentContainer()->addView(l);
    }


    // вкладки представлений
    auto viewsPageView = _new<APageView>() << ".components";
    viewsPageView->setExpanding({0, 2});
    viewsPageView->addPage(componentsList);
    viewsPageView->addPage(_new<ALabel>("ты лох"));
    viewsPageView->addPage(_new<ALabel>("ты лох"));

    auto designerArea = _new<DesignerArea>();
    Autumn::put(designerArea);
    designerArea->setExpanding({2, 2});

    addView(_container<AVerticalLayout>({
        _container<AHorizontalLayout>({
            _new<AButton>("ты лох")
        }) let (AView, {
            setCss("background: $OS_THEME_COLOR");
        }),
        _container<AHorizontalLayout>({
            // левая колонка с компонентами
            _container<AVerticalLayout>({
                _new<APageSwitch>("Основные представления", 0, viewsPageView),
                _new<APageSwitch>("Представления моделей", 1, viewsPageView),
                _new<APageSwitch>("Прочее", 2, viewsPageView),
            }) << ".components_tabs_wrap",
            viewsPageView,

            // центральная рабочая область
            designerArea
        }) let (AViewContainer, {
            setExpanding({2, 2});
        })
    }) let (AViewContainer, {
        setExpanding({2, 2});
    }));
    setCss("padding: 0");
}

