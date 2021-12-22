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

using namespace boost::unit_test;

/**
 * There's a text field and a button:
 * _____________________-[]X_
 * |                        |
 * |  [       ][Say hello]  |
 * |                        |
 * |________________________|
 *
 *
 * user types his name. When it's button is pressed, a message appears:
 * _____________________-[]X_
 * |                        |
 * |  [Steve  ][Say hello]  |
 * |      Hello, Steve!     |
 * |________________________|
 *
 * when text field is empty, hello message should be hidden.
 */
BOOST_AUTO_TEST_SUITE(Type)


class TestWindow: public AWindow {
private:
    _<ATextField> mTextField;
    _<ALabel> mHelloLabel;

public:
    TestWindow() {
        setContents(Centered {
            Vertical {
                Horizontal {
                    mTextField = _new<ATextField>() << "#username",
                    _new<AButton>("Say hello").connect(&AView::clicked, this, [&] {
                        if (mTextField->getText().empty()) {
                            mHelloLabel->setVisibility(Visibility::INVISIBLE);
                            return;
                        }
                        mHelloLabel->setText("Hello, {}!"_format(mTextField->getText()));
                        mHelloLabel->setVisibility(Visibility::VISIBLE);
                    }) let { it->setDefault(); },
                },
                mHelloLabel = _new<ALabel>() let {
                    it->setVisibility(Visibility::INVISIBLE);
                    it << "#hello";
                }
            }
        });

        pack();
    }
};



/**
 * Checks that the message is not appeared yet.
 */
UI_TEST_CASE(HelloIsNotAppeared) {
    // prepare the window
    _new<TestWindow>()->show();

    // check label is not visible
    By::text("Hello!").require(notVisible(), "label is visible");
}

/**
 * Checks that the message does not appear when button is clicked.
 */
UI_TEST_CASE(HelloNotAppearsAfterClick) {
    // prepare the window
    _new<TestWindow>()->show();

    // press the button
    By::text("Say hello").perform(click());

    // check label is NOT appeared
    By::name("#hello").require(notVisible(), "label is appeared");
}

/**
 * Checks that the message does not appear when button is clicked.
 *
 * 1. types "Steve" to the text field
 * 2. clicks the button
 */
UI_TEST_CASE(HelloAppearsAfterClick) {
    // prepare the window
    _new<TestWindow>()->show();

    // type "Steve" to the text field
    By::name("#username").perform(type("Steve"));

    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared and contains text "Hello, Steve!"
    By::name("#hello").require(visible(), "label is not appeared")
                      .require(text("Hello, Steve!"), "invalid text");
}

/**
 * Checks that the message disappears.
 *
 * 1. types "Steve" to the text field
 * 2. clicks the button
 * 3. clears the text field
 * 4. clicks the button again
 */
UI_TEST_CASE(HelloDisappearsAfterClick) {
    // prepare the window
    _new<TestWindow>()->show();

    // type "Steve" to the text field
    By::name("#username").perform(type("Steve"));

    // press the button
    By::text("Say hello").perform(click());

    // check label is appeared and contains text "Hello, Steve!"
    By::name("#hello").require(visible(), "label is not appeared")
                            .require(text("Hello, Steve!"), "invalid text");
}
BOOST_AUTO_TEST_SUITE_END()