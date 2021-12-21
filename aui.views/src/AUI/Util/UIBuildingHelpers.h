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

#pragma once
#include "AUI/Common/AVector.h"
#include "AUI/View/AView.h"
#include "AUI/Layout/AGridLayout.h"
#include <variant>
#include "AUI/View/AViewContainer.h"
#include "AUI/View/ALabel.h"
#include <AUI/Common/SharedPtr.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/AMetric.h>
#include <AUI/View/ASpacer.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/Layout/AWordWrappingLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Layout/AGridLayout.h>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Traits/strings.h>


template<typename Layout, typename... Args>
inline auto _container(const AVector<_<AView>>& views, Args&&... args)
{
	auto c = _new<AViewContainer>();
	c->setLayout(_new<Layout>(args...));

	for (const auto& v : views)
		c->addView(v);

	return c;
}

inline auto _form(const AVector<std::pair<std::variant<AString, _<AView>>, _<AView>>>& views)
{
	auto c = _new<AViewContainer>();
	c->setLayout(_new<AAdvancedGridLayout>(2, views.size()));
	c->setExpanding({2, 0});
	for (const auto& v : views) {
		try {
			c->addView(_new<ALabel>(std::get<AString>(v.first)));
		}
		catch (const std::bad_variant_access&) {
			c->addView(std::get<_<AView>>(v.first));
		}
		v.second->setExpanding({2, 0});
		c->addView(v.second);
	}

	return c;
}

namespace aui::detail {
    template<typename Layout, bool expanding>
    struct container_helper_base {
    private:
        AVector<_<AView>> mViews;

        void processList() {

        }
        template<typename Arg, typename... Args>
        void processList(Arg&& arg, Args&&... args) {
            if constexpr (std::is_base_of_v<AView, Arg>) {
                mViews.push_back(std::forward<Arg>(arg) >> shared);
            } else {
                mViews.push_back(std::forward<Arg>(arg));
            }
            processList(std::forward<Args>(args)...);
        }

        _<AViewContainer> makeContainer() const {
            auto container = _container<Layout>(mViews);
            if constexpr(expanding) container->setExpanding();
            return container;
        }
    public:

        template<typename... Args>
        container_helper_base(Args&&... views) {
            mViews.reserve(sizeof...(views));
            processList(std::forward<Args>(views)...);
        }
        operator _<AView>() const {
            return makeContainer();
        }
        operator _<AViewContainer>() const {
            return makeContainer();
        }
        _<AViewContainer> operator<<(const AString& assEntry) const {
            return makeContainer() << assEntry;
        }
        template<typename T>
        _<AViewContainer> operator^(const T& t) const {
            return makeContainer() ^ t;
        }
        template<typename T>
        _<AViewContainer> operator+(const T& t) const {
            return makeContainer() + t;
        }

        _<AViewContainer> operator->() const {
            return makeContainer();
        }
    };

    template<typename Layout>
    struct container_helper: container_helper_base<Layout, false> {
        using container_helper_base<Layout, false>::container_helper_base;
        using Expanding = container_helper_base<Layout, true>;
    };
}

/**
 * Places views in a column.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/vertical.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> <a href="#AViewContainer">AViewContainer</a></dt>
 *    <dt><b>Layout manager:</b> <a href="#AVerticalLayout">AVerticalLayout</a></dt>
 *  </dl>
 * </p>
 */
using Vertical = aui::detail::container_helper<AVerticalLayout>;

/**
 * Places views in a row.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/horizontal.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> <a href="#AViewContainer">AViewContainer</a></dt>
 *    <dt><b>Layout manager:</b> <a href="#AHorizontalLayout">AHorizontalLayout</a></dt>
 *  </dl>
 * </p>
 */
using Horizontal = aui::detail::container_helper<AHorizontalLayout>;

/**
 * Places views in a stack, centering them.
 * <p>
 *  <img width="960" src="https://github.com/aui-framework/aui/raw/master/docs/imgs/stacked2.jpg">
 *
 *  <dl>
 *    <dt><b>View:</b> <a href="#AViewContainer">AViewContainer</a></dt>
 *    <dt><b>Layout manager:</b> <a href="#AStackedLayout">AStackedLayout</a></dt>
 *  </dl>
 * </p>
 */
using Stacked = aui::detail::container_helper<AStackedLayout>;

/**
 * <p>
 * <code>Center</code> is an alias to <a href="#Stacked">Stacked</a>. When <a href="#Stacked">Stacked</a> is used only for centering views, you can use
 * this alias in order to improve understanding of your code.
 * </p>
 */
using Centered = Stacked;