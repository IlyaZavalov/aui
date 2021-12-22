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
#include <ExampleWindow.h>
#include <AUI/View/ATextField.h>

using namespace boost::unit_test;

/**
 * <p>
 *   A bug appeared in this example application related to AUI layout managers. This suite checks the following issues:
 * </p>
 * <ul>
 *   <li>height of the latest element sometimes (not always!) seems to be incorrect</li>
 *   <li>latest elements sometimes (again, not always!) seems to be not fit to the parent's border</li>
 * </ul>
 *
 * <p>Details: <a href="https://github.com/aui-framework/aui/issues/18">https://github.com/aui-framework/aui/issues/18</a></p>
 * <p>Photo:</p>
 * <img src="https://user-images.githubusercontent.com/19491414/144756936-173e5d65-9433-4c00-92d6-142ca217164d.png" />
 *
 */
BOOST_AUTO_TEST_SUITE(LayoutManager)


/**
 * Checks height of all buttons - they should be the same (see "Show all views..." button above).
 */
UI_TEST_CASE(Height) {
    // prepare the window
    _new<ExampleWindow>()->show();

    // check height
    (By::text("Common button") | By::text("Show all views...")).require(sameHeight(), "height mismatch");
}


/**
 * Checks alignment (see "Alex2772, 2021, alex2772.ru" - it is not perfectly aligned)
 */
UI_TEST_CASE(LastElementAlignment) {
    // prepare the window
    _new<ExampleWindow>()->show();

    // check alignment
    (By::name("#copyright") | By::type<ATextField>()).require(rightAligned(), "bad alignment");

    // copyright width can be also not minimal
    By::name("#copyright").require(widthIsMinimal(), "copyright width should be minimal");
}

/**
 * Checks alignment (looks like buttons and list views are not perfectly aligned)
 */
UI_TEST_CASE(ButtonsAlignment) {
    // prepare the window
    _new<ExampleWindow>()->show();

    // buttons column should be perfectly aligned
    By::name("Common button")
        .parent()
        .allChildren()
        .require(leftRightAligned(), "elements should be perfectly aligned");
}

BOOST_AUTO_TEST_SUITE_END()