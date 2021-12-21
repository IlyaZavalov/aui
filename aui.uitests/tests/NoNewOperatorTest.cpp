/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ALayoutInflater.h>

using namespace boost::unit_test;


/**
 * Implementation of NoNewOperator feature is checked in this suite. It should replace
 * <code>Vertical { _new<ALabel>("Hello") }</code> to <code>Vertical { ALabel{"Hello"} }</code> which is cleaner.
 * More info: <a href="https://github.com/aui-framework/aui/issues/17">https://github.com/aui-framework/aui/issues/17</a>
 */
BOOST_AUTO_TEST_SUITE(NoNewOperator)


/**
 * Checks that views using NoNewOperator are appeared.
 */
UI_TEST_CASE(BasicAppearance) {
    // prepare the window

    auto window = _new<AWindow>();

    ALayoutInflater::inflate(window, Centered {
        AButton { "Hello!" }
    });

    window->pack();
    window->show();

    // check label is not visible
    By::text("Hello!").require(exists(), "label is not appeared");
}

/**
 * Checks that views using NoNewOperator are normally positioned.
 */
UI_TEST_CASE(ComplexLayout) {
    // prepare the window

    auto window = _new<AWindow>();

    ALayoutInflater::inflate(window, Vertical {
        Horizontal {
            ALabel { "Username:" },
            ATextField {},
        },

        Horizontal {
            ASpacer {},
            AButton { "Login" },
            AButton { "Cancel" },
        }
    });

    window->pack();
    window->show();

    // check label is not visible
    By::text("Username:").require(above(By::type<AButton>()), "Username label is incorrectly positioned");
}

/**
 * Checks that text field works.
 */
UI_TEST_CASE(Type) {
    // prepare the window

    auto window = _new<AWindow>();

    //
    ALayoutInflater::inflate(window, Centered {
        ATextField {},
    });

    window->pack();
    window->show();

    // check ATextField
    By::type<ATextField>().perform(type("hello"))
                          .require(text("hello"), "text is not typed")
                          .perform(click())
                          .require([](const _<AView>& v) {
                              if (auto f = _cast<ATextField>(v)) {
                                  return f->getSelection().end == 5;
                              }
                              return false;
                          }, "ATextField selection is broken");
}

/**
 * Checks that views using NoNewOperator are normally reacting to the input.
 */
UI_TEST_CASE(SignalSlot) {
    // prepare the window

    auto window = _new<AWindow>();

    //
    ALayoutInflater::inflate(window, Vertical {
        Horizontal {
            ALabel { "Username:" },
            ATextField {},
        },

        Horizontal {
            ASpacer {},
            AButton { "Login" },
            AButton { "Cancel" },
        }
    });

    window->pack();
    window->show();

}

BOOST_AUTO_TEST_SUITE_END()