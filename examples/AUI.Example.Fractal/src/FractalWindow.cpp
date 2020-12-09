//
// Created by alex2772 on 12/9/20.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/AButton.h>
#include "FractalWindow.h"
#include "FractalView.h"


FractalWindow::FractalWindow():
    AWindow("Mandelbrot set")
{
    setLayout(_new<AHorizontalLayout>());
    auto fractal = _new<FractalView>();
    addView(fractal);
    addView(_container<AVerticalLayout>({
        _new<AButton>("Identity").connect(&AButton::clicked, slot(fractal)::reset),
        _new<ALabel>("Iterations:"),
        _new<ANumberPicker>().connect(&ANumberPicker::valueChanged, this, [fractal](int v) {
            fractal->setIterations(v);
        }) let (ANumberPicker, {
            setMax(100000);
            setValue(1000);
        })
    }));
}
