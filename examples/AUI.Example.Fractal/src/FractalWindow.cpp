//
// Created by alex2772 on 12/9/20.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ANumberPicker.h>
#include "FractalWindow.h"
#include "FractalView.h"


FractalWindow::FractalWindow():
    AWindow("Fractal")
{
    setLayout(_new<AHorizontalLayout>());
    auto fractal = _new<FractalView>();
    addView(fractal);
    addView(_container<AVerticalLayout>({
        _new<ALabel>("C / 1000:"),
        _new<ANumberPicker>().connect(&ANumberPicker::valueChanged, this, [fractal](int v) {
            fractal->setC(v / 1000.f);
        }) let (ANumberPicker, {
            setMax(100000);
            setValue(2);
        }),
        _new<ALabel>("Iterations:"),
        _new<ANumberPicker>().connect(&ANumberPicker::valueChanged, this, [fractal](int v) {
            fractal->setIterations(v);
        }) let (ANumberPicker, {
            setMax(100000);
            setValue(100);
        })
    }));
}
